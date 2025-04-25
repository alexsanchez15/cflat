#ifndef GENERATOR_HPP
#define GENERATOR_HPP
#include <cstdlib>
#include <sstream>
#include <stack>
#include <optional>
#include <string>
#include <unordered_map>
#include "ASTnode.hpp"

// NASM code generation file.
// Each node in the AST will have some sort of code gen, or point to
// its children node's code gen.
// objected oriented programming! Generator object -> takes Program node
// Program node points to statements, and magic ensues (:

// convention: expressions evaluate just simply to rax register.
// also for simplicity (i know it ignores the speed of assembly) ill jst use full registers

struct offset_t{
    int o1;
    std::optional<int> o2;
    // offset 1, offset 2, optional and only used for string types
    // only string types and int types exist so this is fine,
    // but expanding this i would probably include some metadata here
};
struct func_data{
    std::string input_t; // as i am doing only single parem, single return type
    std::string output_t;
    std::optional<bool> is_std_lib;
};
class Generator : ASTVisitor{
public:
    //constructor
    explicit Generator(NodeProg* p) : program(p){} // only thing we need is p
    
    void generate(){
        fillfuncs();
        visit(program);
    }
    

    void visit(NodeProg* p){
        // will go through the program, and make the code
        // default template NASM to start with:
        std::cout << "Visiting NodeProg Node. # of statements in prog: " << p->statements.size() << std::endl;
        stream_p << "section .text\nglobal _start\n_start:\n\t; code starts here" << std::endl;
        stream_p << "\tmov rbp, rsp" << std::endl; // store location of start of stack pointer, for variable offset.
        stream_p <<"\tsub rsp, RESERVED_VARS" << std::endl;
        for(const std::unique_ptr<NodeStmt>& statement: program->statements){
            // do some stuff
            statement->accept(*this);
        }
    }

    void visit(NodeTermVar* p) {
        // using the offset, load the term into rax.
        // rbp - (offset +1) * 8 is where varables are stored.
        // put the var is rax.
        if(var_offset.count(*p->varname) == 0){
            std::cerr << "Var name: " << *p->varname << " not found." << std::endl;
            exit(EXIT_FAILURE);
        }
        stream_p <<"\tmov rax, [rbp - " << (var_offset[*p->varname].o1 + 1) * 8 <<"]" << std::endl; 
        if(var_offset[*p->varname].o2){
            p->is_str = true;
            stream_p <<"\tmov rsi, [rbp - " << (var_offset[*p->varname].o1 + 2) * 8 <<"]" << std::endl; 
        }
        

    }

    void visit(NodeTermIntLit* p) {
        // simplest one, just put that into into rax.
        stream_p << "\tmov rax, " << p->int_lit << std::endl;
        std::cout << p->int_lit << std::endl;
    };
    void visit(NodeStmtExit* p) {
        // nasm gen for exit statement
        p->exit_code->accept(*this); // puts exit code into rax.
        if(p->exit_code->str_op()){
            std::cerr << "string type passed as exit code, error" << std::endl;
            exit(EXIT_FAILURE);
        }
        // push rax to stack, load 60 into rax (exit code), pop into rdi (the exit code) syscall
        stream_p << "\tpush rax" << std::endl;
        stream_p << "\tmov rax, 60" << std::endl;
        stream_p << "\tpop rdi" << std::endl;
        stream_p << "\tsyscall" << std::endl;
    };
    void visit(NodeTermStrLit* p){
        // make sure data exists in rodata (read only) for str lits
        // will just be a pointer to the string in this case.
        int label_offset = add_rodata(*p->str_contents); //(str_contents is uniq ptr)
        std::string strlabel = "Lstr" + std::to_string(label_offset);
        std::string lenlabel = "len" + std::to_string(label_offset);
        //my convention: pointer to string contents in rax, length in rsi.
        // rax -> rsi -> rdx is my thinking of convention for passing refrences
        stream_p << "\tmov rax, " << strlabel << std::endl;
        stream_p << "\tmov rsi, " << lenlabel << std::endl;
    };
    void visit(NodeExprBinAdd* p) {
        // smallest atomic size is term, so these always are either term or recurse down to term
        // add two vals: add rax, rbx =: rax = rax + rbx (sol held in rax)
        p->lhs->accept(*this); // now value of lhs should be in rax
        stream_p << "\tpush rax" << std::endl; // put onto stack
        p->rhs->accept(*this); // now val of rhs should be in rax
        stream_p << "\tmov rbx, rax" << std::endl; // move into rbs from rax (rbx = rax)
        stream_p << "\tpop rax" << std::endl;
        stream_p << "\tadd rax, rbx" << std::endl; //still ends with sum in rax, which is ideal
    };
    void visit(NodeExprBinMult* p) {
        // signed mult is imul rax, rbx =: rax = rax * rbx
        p->lhs->accept(*this); // now value of lhs should be in rax
        stream_p << "\tpush rax" << std::endl; // put onto stack
        p->rhs->accept(*this); // now val of rhs should be in rax
        stream_p << "\tmov rbx, rax" << std::endl; // move into rbs from rax (rbx = rax)
        stream_p << "\tpop rax" << std::endl;
        stream_p << "\timul rax, rbx" << std::endl; 

    };
    void visit(NodeStmtPrint* p) {

        //visit it
        p->str_expr->accept(*this);

        if(!p->str_expr->str_op()){
            std::cerr << "numeric type passed to print, error" << std::endl;
            exit(EXIT_FAILURE);
        }
        // AS PER MY CONVENTION, now str pointer in rax, length in rsi.
        // printing to stdout: rax 1 rdi 1 rsi strptr rdx lenptr
        stream_p <<"\tmov rdx, rsi\t; mov len" << std::endl; // this first as rsi wants to be used
        stream_p <<"\tmov rsi, rax\t; mov string pointer" << std::endl; // mov from rax -> rsi, str pointer
        //if not, visit the expression.
        stream_p <<"\tmov rax, 1" << std::endl;
        stream_p <<"\tmov rdi, 1" << std::endl;
        stream_p <<"\tsyscall" << std::endl;
        if(p->newline) print_newline();
    };
    void visit(NodeStmtInitVar* p) {
        // ok so what happens when we init a var?
        // we have 2 primitive types: string and int, dealt with seperately.
        // if int, gen expression code -> result into rax
        // push onto stack, store the offest in var_offset map for each integer.
        std::string type = *p->type; //easier access
        p->expr->accept(*this);

        if(type == "int"){
            if(p->expr->str_op()){
                std::cerr << "type \"int\" cannot be initialized as a string." << std::endl;
                exit(EXIT_FAILURE);
            }
            stream_p <<"\tmov qword [rbp - " << (var_stack_index + 1) *8 << "], rax" << std::endl; // put onto stack.
            var_offset[*p->name].o1 = var_stack_index; // how many places down on the stack it is
            var_stack_index++;
            var_stack.push(*p->name);
            total_vars +=1;

        }
        if(type == "string"){
            if(!p->expr->str_op()){
                std::cerr << "type \"string\" cannot be initialized as a numeric." << std::endl;
                exit(EXIT_FAILURE);
            }
            stream_p <<"\tmov qword [rbp - " << (var_stack_index + 1) *8 << "], rax" << std::endl; // put onto stack.
            var_offset[*p->name].o1 = var_stack_index; // how many places down on the stack it is
            var_stack_index++;

            //for the strlen which exists in rsi
            stream_p <<"\tmov qword [rbp - " << (var_stack_index + 1) *8 << "], rsi" << std::endl; 
            var_offset[*p->name].o2 = var_stack_index; // how many places down on the stack it is
            var_stack_index++;
            var_stack.push(*p->name);
            // need to store this

            // my string types are like a struct: two things stored in memory under the hood
            total_vars +=2; // ptr to string, ptr to string len
        }
    };
    void visit(NodeStmtReVar* p) {
        // now we can safely (...) visit the expression, putting its result in rax.
        p->expr->accept(*this);
        // result in rax (& rsi if string), which is either a string or an int.
        stream_p <<"\tmov qword [rbp - " << (var_offset[*p->name].o1 + 1) *8 << "], rax" << std::endl; // put onto stack.
        if(var_offset[*p->name].o2){
            // I AM NOT REALLY SURE HOW TO IMPLEMENT THIS FOR STRINGS BESIDES JUST ADDING ANOTHER THING TO rodata
            // IF I WANT TO KEEP STRING TYPE IMMUTABLE! so whatever wont be ideal but its ok
            // means that this var is a string container. (only strings contain a val in the o2)
            // juuuust now realizing I could have made a much better structure if I saved more metadata
            // in the offset_t struct, like some enum for type instead of just using "if offset2 exists" lol
            // TODO: implement the above? readaibility will improve..

            stream_p <<"\tmov qword [rbp - " << (var_offset[*p->name].o2.value() + 1) *8 << "], rsi" << std::endl; // put onto stack.
        }
    };
    void visit(NodeExprBoolL* p){
        // movzx rax, al -> moves result into rax by my convention expressions -> rax. zero out beginning
        p->lhs->accept(*this);
        stream_p << "\tpush rax" << std::endl; //throw on stack, avoid colisions with other registers (and thinking)
        p->rhs->accept(*this);
        stream_p << "\tpop rsi" << std::endl;
        stream_p << "\tcmp rsi, rax" << std::endl; // 
        stream_p << "\tsetl al" << std::endl; // is rsi < rax?
        stream_p << "\tmovzx rax, al" << std::endl; 
    }
    void visit(NodeExprBoolEq* p) {
        // cmp register 1, register 2
        // result flags get set.
        // set<?> al , al is a 1 byte register, set<?> sets to 1 byte registers.
        // movzx rax, al -> moves result into rax by my convention expressions -> rax. zero out beginning
        p->lhs->accept(*this);
        stream_p << "\tpush rax" << std::endl; //throw on stack, avoid colisions with other registers (and thinking)
        p->rhs->accept(*this);
        stream_p << "\tpop rsi" << std::endl;
        stream_p << "\tcmp rax, rsi" << std::endl; //set result flags
        //mayhaps make this more extended? for now, just eq case.
        stream_p << "\tsete al" << std::endl;
        stream_p << "\tmovzx rax, al" << std::endl; // smile


    }
    void visit(NodeStmtIf* p){
        // result is placed in rax from expression. Make the label, go to it
        // jz .end<counter>
        // <logic>
        // .end:
        // ..continued code
        p->bool_res->accept(*this); //put res in rax
        stream_p << "\ttest rax, rax" << std::endl; // call test, will fill jz (jump if zero)

        std::string endlabel = ".end" + std::to_string(endlabel_count);
        endlabel_count++;

        stream_p << "jz " << endlabel << std::endl;
        p->following_logic->accept(*this); // now fill will logic inside of if

        stream_p << endlabel << ":"<<  std::endl; //.. and then continue along
    }
    void visit(NodeStmtWhile* p){
        // whileblock<count>:
        // eval conditon expression.
        // result is placed in rax from expression. Make the label, go to it
        // jz .end<counter>
        // <logic>
        // jmp whileblock<count>
        // .end:
        // ..continued code
        std::string whilelabel = "whileblock" + std::to_string(whilelabel_count);
        std::string endwhile = "endwhile" + std::to_string(whilelabel_count);
        whilelabel_count++;

        stream_p << whilelabel << ":" << std::endl;
        p->expression->accept(*this); //put res in rax
        stream_p << "\ttest rax, rax" << std::endl; // call test, will fill jz (jump if zero)

        stream_p << "\tjz " << endwhile << std::endl;
        p->inner_logic->accept(*this); // now fill will logic inside of if
        stream_p << "\tjmp " << whilelabel << std::endl;

        stream_p << endwhile << ":"<<  std::endl; //.. and then continue along
    }

    void visit(NodeStmtScope* p){
        // var_stack is already populated with vars in a stack,
        // when reach end of statements, pop all from stack up to that {
        var_stack.push("{");
        for(const std::unique_ptr<NodeStmt>& statement: p->statements){
            // do some stuff
            statement->accept(*this);
        }
        while(true){
            if(var_stack.empty()){
                std::cerr << "FATAL ERROR reached end of stack without scope pointer!" << std::endl;
            }
            else if(var_stack.top() == "{"){
                var_stack.pop();
                break;
            }
            else{
                std::cout << "popping var" << var_stack.top() << std::endl;
                var_offset.erase(var_stack.top()); //get rid of it from scope
                var_stack.pop(); // take it off the stack.
            }
        }
    }
    void visit(NodeExprFunc* p){
        std::string fname = *p->fname;
        if(functions_map.count(fname) == 0){
            std::cerr << "Use of undefined function: " << fname << std::endl;
            exit(EXIT_FAILURE);
        }

        p->param->accept(*this); // param in rax

        if(functions_map[fname].is_std_lib){
            // as this doesn't support other functions at this time, this is the only logic I will do.
            if((functions_map[fname].input_t == "int" && !p->param->str_op())|| (
            functions_map[fname].input_t == "string" && p->param->str_op())){ //this is really ugly. 
                //input matches
                stream_p << "\tcall " << fname << std::endl;
                externs << "extern " << fname << std::endl;
                std::cout << "externs\n" << externs.str() << std::endl;
                if(functions_map[fname].output_t == "string"){
                    p->is_str = true;
                }

            }
            else{
                std::cerr << "Improper use of function " << fname << std::endl;
                std::cerr << "Accepts: " << functions_map[fname].input_t << std::endl;
                exit(EXIT_FAILURE);

            }
        }
    }

    //get the final nasm generated
    std::string get_assembly(){
        std::stringstream final;
        std::string exit_clean = "\t;exit cleanly\n\tmov rax, 60\n\tmov rdi, 0\n\tsyscall";
        if(!externs.str().empty()){
            final << externs.str() << std::endl;
        }
        if(!stream_data.str().empty()){
            final << stream_data.str() << std::endl;
        }
        if(total_vars !=0){
            stream_rodata << "RESERVED_VARS equ " << (8 * total_vars) << std::endl;
        }
        else stream_rodata << "RESERVED_VARS equ 0" << std::endl; 
        if(!stream_rodata.str().empty()){
            final << stream_rodata.str() << std::endl;
        }
        
        final << stream_p.str() << std::endl; 
        final << exit_clean << std::endl;
        return final.str();
    }

private:
    NodeProg* program; // raw pointer to the program node
    std::stringstream stream_p;
    std::stringstream stream_data;
    std::stringstream stream_rodata;
    std::stringstream externs;
    int count_rodata = 0;
    int count_data = 0;

    // stack of existing vars
    std::unordered_map<std::string, offset_t> var_offset;
    std::stack<std::string> var_stack;
    int var_stack_index = 0;
    int total_vars = 0;

    //for if labels:
    int endlabel_count = 0;

    //for while labels:
    int whilelabel_count = 0;

    //for functions
    std::unordered_map<std::string, func_data> functions_map;

    int add_rodata(std::string s){
        if(stream_rodata.str().empty()){
            stream_rodata <<"section .rodata" << std::endl;
            stream_rodata <<"newline: db 10" << std::endl;
        }
        // format .Lstr0: db "xyz", 0
        // len0: equ $ - Lstr0
        std::string label = "Lstr" + std::to_string(count_rodata);
        stream_rodata << label << ": db \"" << s << "\", 0" << std::endl;
        stream_rodata << "len" << std::to_string(count_rodata) << " equ $ - " << label << std::endl;
        count_rodata++;
        return count_rodata-1; //return the non incremented version (but obv we still want it inc for later)

    }
    void print_newline(){
        // for at the end of prints. i dont want newlines attached to memory. a second syscall is not that expensive
        // plus now i can very easily write different print() and println() functions if i wanna
        // AND concat wouldnt be weird as hell
        stream_p << "\tmov rax , 1" <<std::endl;
        stream_p << "\tmov rdi , 1" <<std::endl;
        stream_p << "\tmov rdx, 1" << std::endl;
        stream_p << "\tmov rsi, newline" << std::endl;
        stream_p << "\tsyscall" << std::endl;
    }
    void fillfuncs(){
        // fills in standard functions
        functions_map["to_str"] = {"int", "string", true};

    }

};
#endif
