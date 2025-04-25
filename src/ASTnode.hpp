// Class to 
#ifndef AST_H
#define AST_H
#include <memory>
#include <sstream>
#include <vector>
#include <iostream>
#include <sstream>

// forward declarations for all concrete node types:
class NodeProg;
class NodeExprBinAdd;
class NodeExprBinMult;
class NodeExprBoolEq;
class NodeExprBoolL;
class NodeExprFunc;
class NodeTermVar;
class NodeTermIntLit;
class NodeStmtExit;
class NodeStmtIf;
class NodeStmtWhile;
class NodeStmtPrint;
class NodeStmtInitVar;
class NodeStmtReVar;
class NodeTermStrLit;
class NodeStmtScope;

// visitor design pattern, move through the nodes easily 
class ASTVisitor{
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(NodeProg* p) = 0;
    virtual void visit(NodeExprBinAdd* p) = 0;
    virtual void visit(NodeExprBinMult* p) = 0;
    virtual void visit(NodeTermVar* p) = 0;
    virtual void visit(NodeTermIntLit* p) = 0;
    virtual void visit(NodeExprBoolEq* p) = 0;
    virtual void visit(NodeExprBoolL* p) = 0;
    virtual void visit(NodeExprFunc* p) = 0;
    virtual void visit(NodeStmtExit* p) = 0;
    virtual void visit(NodeStmtPrint* p) = 0;
    virtual void visit(NodeStmtInitVar* p) = 0;
    virtual void visit(NodeStmtReVar* p) = 0;
    virtual void visit(NodeStmtScope* p) = 0;
    virtual void visit(NodeStmtIf* p) = 0;
    virtual void visit(NodeStmtWhile* p) = 0;
    virtual void visit(NodeTermStrLit* p) = 0;
    
};



class ASTNode{
public:
    virtual ~ASTNode() = default;
    virtual void display_self(int depth) = 0;
    virtual void accept(ASTVisitor& visitor) = 0; 


    //just for debugging/printing parse tree (AST)
    std::string makespace(int spaces){
        std::stringstream str;
        for(int i =0; i<spaces; i++){
            str << "  ";
        }
        return str.str();
    }
};


class NodeExpr : public ASTNode {
public:
    virtual bool str_op() = 0; // only numeric / string expr (this is somewhat ugly i think)
};

class NodeStmt : public ASTNode {};

class NodeTerm : public NodeExpr {};


// Program node is the highest level node, contains list of statements. 
class NodeProg : public ASTNode {
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    std::vector<std::unique_ptr<NodeStmt>> statements;

    void addStatement(std::unique_ptr<NodeStmt> statement){
        statements.push_back(std::move(statement));
    }

    // function for debugging/just seeing the AST
    void display_self(int depth=0) override {
        std::cout << "Program Node With #" << statements.size() << " statements" << std::endl;
        depth += 1; // all statements need += 1
        for(const std::unique_ptr<NodeStmt>& statement: statements){
            std::cout << "what" << std::endl;
            statement->display_self(depth);
        }
        std::cout << "displayself over" << std::endl;
    }
};

//We can implement different statements of NodeStmt:
class NodeStmtExit: public NodeStmt{
public:

    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }

    std::unique_ptr<NodeExpr> exit_code;

    NodeStmtExit(std::unique_ptr<NodeExpr> ec) : exit_code(std::move(ec)){}
    

    void display_self(int depth) override {
        std::cout << makespace(depth) <<"Node Statement: Exit" << std::endl;
        depth += 1;
        exit_code->display_self(depth);
    }

};

class NodeStmtIf: public NodeStmt{
public:

    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }

    std::unique_ptr<NodeExpr> bool_res; //boolean result

    std::unique_ptr<NodeStmt> following_logic; // bad naming bro

    NodeStmtIf(std::unique_ptr<NodeExpr> ec, std::unique_ptr<NodeStmt> fl) : bool_res(std::move(ec)), following_logic(std::move(fl)){}
    

    void display_self(int depth) override {
        std::cout << makespace(depth) <<"Node Statement: If" << std::endl;
        std::cout << makespace(depth) <<"Condition Expression:" << std:: endl;
        bool_res->display_self(depth + 1);
        std::cout << makespace(depth) <<"Following Statement:" << std:: endl;
        following_logic->display_self(depth + 1);

    }

};

class NodeStmtWhile: public NodeStmt{
public:

    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }

    std::unique_ptr<NodeExpr> expression; //boolean result

    std::unique_ptr<NodeStmt> inner_logic; // bad naming bro

    NodeStmtWhile(std::unique_ptr<NodeExpr> ec, std::unique_ptr<NodeStmt> fl) : expression(std::move(ec)), inner_logic(std::move(fl)){}
    

    void display_self(int depth) override {
        std::cout << makespace(depth) <<"Node Statement: If" << std::endl;
        std::cout << makespace(depth) <<"Condition Expression:" << std:: endl;
        expression->display_self(depth + 1);
        std::cout << makespace(depth) <<"Following Statement:" << std:: endl;
        inner_logic->display_self(depth + 1);

    }

};

class NodeStmtPrint: public NodeStmt{
public:

    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }

    std::unique_ptr<NodeExpr> str_expr;
    bool newline = false;

    NodeStmtPrint(std::unique_ptr<NodeExpr> s, bool nl) : str_expr(std::move(s)), newline(nl){}
    

    void display_self(int depth) override {
        std::cout << makespace(depth) <<"Node Statement: Print" << std::endl;
        depth += 1;
        str_expr->display_self(depth);
    }

};

class NodeStmtInitVar : public NodeStmt{
public:
    // or maybe use that string for something else later?
    // idk but for now will just do:
    // <string(type)> <name> = <expression> 
    // where a term is a type of expression, so other vars, lits, usable
    std::unique_ptr<std::string> type;
    std::unique_ptr<std::string> name;
    std::unique_ptr<NodeExpr> expr;

    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }

    NodeStmtInitVar(std::unique_ptr<std::string> t, std::unique_ptr<std::string> n, std::unique_ptr<NodeExpr> e) : type(std::move(t)), name(std::move(n)), expr(std::move(e)){}
    void display_self(int depth) override{
        std::cout << makespace(depth) << "Init Var Statement:" << std::endl;
        std::cout << makespace(depth) << "type: " << *type << "\tname: " << *name << std::endl;
        std::cout << makespace(depth) << "Expr:" << std::endl;
        expr->display_self(depth + 1);
    }
};

class NodeStmtReVar: public NodeStmt{
public:
    // variable reassignment
    //<name> = <expression> 
    std::unique_ptr<std::string> name;
    std::unique_ptr<NodeExpr> expr;

    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }

    NodeStmtReVar(std::unique_ptr<std::string> n, std::unique_ptr<NodeExpr> e) : name(std::move(n)), expr(std::move(e)){}
    void display_self(int depth) override{
        std::cout << makespace(depth) << "Reassign Var Statement:" << std::endl;
        std::cout << makespace(depth) << "Expr:" << std::endl;
        expr->display_self(depth + 1);
    }
};

class NodeStmtScope: public NodeStmt{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    std::vector<std::unique_ptr<NodeStmt>> statements;

    void addStatement(std::unique_ptr<NodeStmt> statement){
        statements.push_back(std::move(statement));
    }

    // function for debugging/just seeing the AST
    void display_self(int depth) override {
        std::cout << makespace(depth) << "Scope Node With #" << statements.size() << " statements" << std::endl;
        depth += 1; // all statements need += 1
        for(const std::unique_ptr<NodeStmt>& statement: statements){
            statement->display_self(depth);
        }
    }
};



//Different expressions for NodeExpr -> Term, BinaryExpression[expr +*-/ expr]
class NodeTermIntLit : public NodeTerm{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    bool str_op() override{
        return false;
    }
    int int_lit;

    NodeTermIntLit(int term) : int_lit(term){}

    void display_self(int depth) override{
        std::cout << makespace(depth) <<"Term: Int Lit: " << int_lit << std::endl;
}};
class NodeTermVar : public NodeTerm{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    bool str_op() override{
        return is_str;
    }
    bool is_str = false;
    std::unique_ptr<std::string> varname;

    NodeTermVar(std::unique_ptr<std::string> iden) : varname(std::move(iden)){} //varname... identifier... same thing

    void display_self(int depth) override{
        std::cout << makespace(depth) <<"Term: Var " << *varname << std::endl;
    }
};
class NodeTermStrLit : public NodeTerm{
public:
    bool str_op() override{
        return true;
    }
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    std::unique_ptr<std::string> str_contents;

    NodeTermStrLit(std::unique_ptr<std::string> c) : str_contents(std::move(c)){} //varname... identifier... same thing

    void display_self(int depth) override{
        std::cout << makespace(depth) <<"Term: StrLit: " << *str_contents << std::endl;
    }
};

class NodeExprFunc : public NodeExpr{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    bool str_op() override{
        return is_str;
    }
    bool is_str = false;
    std::unique_ptr<std::string> fname;
    std::unique_ptr<NodeExpr> param; // single param funcs is all for now

    NodeExprFunc(std::unique_ptr<std::string> iden, std::unique_ptr<NodeExpr> ex) : fname(std::move(iden)), param(std::move(ex)){} //varname... identifier... same thing

    void display_self(int depth) override{
        std::cout << makespace(depth) <<"Function: " << *fname << std::endl;
        std::cout << makespace(depth) << "Taking Parameter(s) Expression:" << std::endl;
        param->display_self(depth + 1);
    }
};

class NodeExprBin :public NodeExpr{
public:
    bool str_op() override{
        return false;
    }

    std::unique_ptr<NodeExpr> lhs;
    std::unique_ptr<NodeExpr> rhs;

    NodeExprBin(std::unique_ptr<NodeExpr> left_term,
                   std::unique_ptr<NodeExpr> right_term, std::string nname) :
    lhs(std::move(left_term)), rhs(std::move(right_term)), nodename(nname){}

    void display_self(int depth) override{
        std::cout << makespace(depth)<< "Binary " << nodename << " Expression:" << std::endl;
        std::cout << makespace(depth) << "LHS:" << std::endl;
        lhs->display_self(depth + 1);
        std::cout << makespace(depth) << "RHS:" << std::endl;
        rhs->display_self(depth + 1);
    }
private:
    std::string nodename; 

};

class NodeExprBinAdd: public NodeExprBin{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    NodeExprBinAdd(std::unique_ptr<NodeExpr> left_term,
                   std::unique_ptr<NodeExpr> right_term) :
    NodeExprBin(std::move(left_term), std::move(right_term), "Add Node") {}

};
class NodeExprBinMult: public NodeExprBin{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    NodeExprBinMult(std::unique_ptr<NodeExpr> left_term,
                   std::unique_ptr<NodeExpr> right_term) :
    NodeExprBin(std::move(left_term), std::move(right_term), "Mult Node") {}
};

class NodeExprBoolEq: public NodeExprBin{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    NodeExprBoolEq(std::unique_ptr<NodeExpr> left_term,
                   std::unique_ptr<NodeExpr> right_term) :
    NodeExprBin(std::move(left_term), std::move(right_term), "Boolean Equals Node") {}
};

class NodeExprBoolL: public NodeExprBin{
public:
    void accept(ASTVisitor& visitor) override{
        visitor.visit(this);
    }
    NodeExprBoolL(std::unique_ptr<NodeExpr> left_term,
                   std::unique_ptr<NodeExpr> right_term) :
    NodeExprBin(std::move(left_term), std::move(right_term), "Boolean Equals Node") {}
};
#endif
