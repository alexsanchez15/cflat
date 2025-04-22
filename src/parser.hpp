#ifndef PARSER_HPP
#define PARSER_HPP
#include "tokenizer.hpp"
#include "ASTnode.hpp"
#include <cstdlib>
// The parser will take the tokens generated and produce the 
// AST, seperating into appropriate nodes for the assembly generator
// (or perhaps the llvm) to process

// My standard: whitespace will always be cleared going into an expression
class Parser{
public:
    explicit Parser(std::vector<Token> t) : m_tokens(t), m_tindex(0){}


    std::optional<std::unique_ptr<NodeExpr>> parse_term(){
        // remember that a term is also an expression. In fact maybe i should say that a term should be before an expression in hierarchy. 
        // prehaps that is more accurate? this does work though.
        //term: an expression inclosed in parens, or int literal
        // (for now) just handle term
        auto token = peek();
        if(!token.has_value()){
            return {}; // then just no term
        }
        if(token.value().type == TokenType::INT_LITERAL){
            consume(); //consume the int literal
            try_consume_whitespace(); // if theres whitespace, kill it.
            //.value.value() lol but value is optional so is correct
            return std::make_unique<NodeTermIntLit>(std::stoi(token.value().value.value()));

        }
        else if(token.value().type == TokenType::IDENTIFIER){
            consume(); //consume the int literal
            try_consume_whitespace(); // if theres whitespace, kill it.
            //.value.value() lol but value is optional so is correct
            return std::make_unique<NodeTermVar>(std::move(std::make_unique<std::string>(token.value().value.value())));

        }
        else if(token.value().type == TokenType::STRING){
            consume(); //consume the int literal
            try_consume_whitespace(); // if theres whitespace, kill it.
            return std::make_unique<NodeTermStrLit>(std::move(std::make_unique<std::string>(token.value().value.value())));

        }
        return {};
    }
    // expression parser will need to map binary expression -> precedance
    // we can manually draft the table for this as follows.


    std::optional<std::unique_ptr<NodeExpr>> parse_expr(int min_prec=0){
        //idea: every expression have a LHS term, 
        //but not necessarily a operand & another term.
        

        auto lhs = parse_term(); // note to self this also consumes it :p
        if(!lhs){
            error_out("Could not resolve expression");
        }
        try_consume_whitespace(); //incase whitespace

        auto operand = peek();
        if(!operand){
            error_out("Expected something following term"); //; at very least
        }
        if(lhs.value()->str_op()){
            //handle string concat here eventually. for now assuming all string are just that... nothing else
        }
        else{
        // THIS IS FOR NUMERICS ONLY:
            while(true){
                auto operand = peek();
                // lets make some bool func "is_bin_exp_op"
                if(!operand || !is_bin_exp_op(operand.value().type)){
                    break;
                } 
                // at this point, are using the operator, and can recurse and all that :)
                //operator already stored in operator, so can consume.
                int prec = tok_prec(operand.value().type);
                if(prec < min_prec){
                    break;
                }
                consume();
                try_consume_whitespace();
                auto rhs = parse_expr(prec + 1); // from pratt's algorithm. i dont really understand why +1 so much but thats ok smile
                
                if(!rhs.has_value()){
                    error_out("RHS has no value, expected term.");
                }
                
                if(operand.value().type == TokenType::PLUS){
                    // consume the next token and prepare the next one for a recursive call.
                    lhs = std::make_unique<NodeExprBinAdd>(std::move(lhs.value()), std::move(rhs.value()));
                }
                else if (operand.value().type == TokenType::STAR){
                    lhs = std::make_unique<NodeExprBinMult>(std::move(lhs.value()), std::move(rhs.value()));

                }
                else if (operand.value().type == TokenType::BEQ){
                    lhs = std::make_unique<NodeExprBoolEq>(std::move(lhs.value()), std::move(rhs.value()));

                }

            }

        }
        return lhs;

    }

    std::optional<std::unique_ptr<NodeStmt>> parse_stmt(){
        try_consume_whitespace(); //if theres whitespace, just remove it.

        auto token = peek();
        if(!token.has_value()){
            return {}; // error out with "expected statement"
        }
        // exit statement
        if(token.value().type == TokenType::EXIT){
            consume(); //consume the exit
            //expect parenthases immediately, if not throw error:
            consume_or_error(TokenType::LPAREN);
            try_consume_whitespace(); //perhaps some whitespace
            if (auto node_expression = parse_expr(); node_expression.has_value()){
                if(node_expression.value()->str_op() == true){
                    std::cerr << "passed a string expression to exit statement - invalid" << std::endl;
                    exit(EXIT_FAILURE);
                }
                consume_or_error(TokenType::RPAREN);
                try_consume_whitespace();
                consume_or_error(TokenType::SEMI);
                //statement is over now
                if(node_expression.value()->str_op()){

                }
                return std::make_unique<NodeStmtExit>(std::move(node_expression.value()));
            }
            else{
                // TODO please better errors :sob:
                std::cerr << "no expression found in exit statement sadface" << std::endl;
            }
        }
        else if(token.value().type == TokenType::KEYWORD){
            // first token is a keyword, making this a var init statement.
            // syntax : <keyword> <identifier> = <expressiion> so are strings an expression ?? TBD assume ints for this
            std::unique_ptr type = std::make_unique<std::string>(token.value().value.value()); // lol whatever
            consume(); //consume the keyword
            consume_whitespace(); 


            std::unique_ptr identifier = std::make_unique<std::string>(peek().value().value.value()); // lol whatever
            consume();

            try_consume_whitespace(); // space between eq sign is optional
            consume_or_error(TokenType::EQUALS);
            try_consume_whitespace();
            auto expr = parse_expr();
            if(!expr.has_value()){  
                error_out("expression failed to parse");
            }
            try_consume_whitespace();
            consume_or_error(TokenType::SEMI); // end of statement always with a semi
            return std::make_unique<NodeStmtInitVar>(std::move(type), std::move(identifier), std::move(expr.value()));


        }
        else if(token.value().type == TokenType::PRINT){
            consume(); //consume the exit
            //expect parenthases immediately, if not throw error:
            consume_or_error(TokenType::LPAREN);
            try_consume_whitespace(); //perhaps some whitespace
            if (auto node_expression = parse_expr(); node_expression.has_value()){
                // ensure that expression is a string expression:
                consume_or_error(TokenType::RPAREN);
                try_consume_whitespace();
                consume_or_error(TokenType::SEMI);
                //statement is over now
                return std::make_unique<NodeStmtPrint>(std::move(node_expression.value()));
            }
            else{
                // TODO please better errors :sob:
                std::cerr << "no expression found in print statement sadface" << std::endl;
            }
        }
        else if(token.value().type == TokenType::LCURL){
            consume(); //consume the {
            try_consume_whitespace();
            std::unique_ptr<NodeStmtScope> sc = std::make_unique<NodeStmtScope>();
            while(peek().has_value()){
                if(auto stmt = parse_stmt(); stmt.has_value()){
                    sc->addStatement(std::move(stmt.value()));
                }
                else{
                    std::cout << "ERROR AT" << token_string(peek().value().type) << std::endl;
                    std::cerr << "Parse Statment Error in SCOPE" << std::endl; // TODO make better errors
                    exit(EXIT_FAILURE);
                }
                try_consume_whitespace();
                if(auto t = peek(); t.has_value() && t.value().type == TokenType::RCURL){
                    consume();// consume the RCURL
                    break;
                }
            }
            return sc;

        }
        else if(token.value().type == TokenType::IDENTIFIER){

            std::unique_ptr identifier = std::make_unique<std::string>(peek().value().value.value()); // lol whatever
            consume();

            try_consume_whitespace(); // space between eq sign is optional
            if(peek().has_value() && peek().value().type == TokenType::EQUALS){
                consume_or_error(TokenType::EQUALS);
                try_consume_whitespace();
                auto expr = parse_expr();
                if(!expr.has_value()){  
                    error_out("expression failed to parse");
                }
                try_consume_whitespace();
                consume_or_error(TokenType::SEMI); // end of statement always with a semi
                return std::make_unique<NodeStmtReVar>(std::move(identifier), std::move(expr.value()));

            }
            else{} // TODO: if <identifier> <identifier> = ...
            // is the case if i implement objects(?) perhaps...

        }
        else if(token.value().type == TokenType::IF){
            consume(); //consume the exit
            //expect parenthases immediately, if not throw error:
            consume_or_error(TokenType::LPAREN);
            try_consume_whitespace(); //perhaps some whitespace
            auto node_expression = parse_expr();
            if(!node_expression.has_value()){
                std::cerr << "no expression found in if statement sadface" << std::endl;
                exit(EXIT_FAILURE);
            }
            consume_or_error(TokenType::RPAREN);
            try_consume_whitespace();
            
            //now we want another statement. either a actual statement or a {} (also a valid statement)
            auto statement = parse_stmt();
            if(!statement.has_value()){
                std::cerr << "no statement found following if statement" << std::endl;
                exit(EXIT_FAILURE);
            }
            //no need to consume ; or {} or anything because statements do this themselves.
            //statement is over now
            return std::make_unique<NodeStmtIf>(std::move(node_expression.value()), std::move(statement.value()));
        }
        else if(token.value().type == TokenType::WHILE){
            consume(); //consume the exit
            //expect parenthases immediately, if not throw error:
            consume_or_error(TokenType::LPAREN);
            try_consume_whitespace(); //perhaps some whitespace
            auto node_expression = parse_expr();
            if(!node_expression.has_value()){
                std::cerr << "no expression found in while statement sadface" << std::endl;
                exit(EXIT_FAILURE);
            }
            consume_or_error(TokenType::RPAREN);
            try_consume_whitespace();
            
            //now we want another statement. either a actual statement or a {} (also a valid statement)
            auto statement = parse_stmt();
            if(!statement.has_value()){
                std::cerr << "no statement found following while statement" << std::endl;
                exit(EXIT_FAILURE);
            }
            //no need to consume ; or {} or anything because statements do this themselves.
            //statement is over now
            return std::make_unique<NodeStmtWhile>(std::move(node_expression.value()), std::move(statement.value()));
        }
        
        return {}; // TODO placeholder
    }


    std::optional<NodeProg> parse_prog(){
        NodeProg prog;
        while(peek().has_value()){
            if(auto stmt = parse_stmt(); stmt.has_value()){
                prog.addStatement(std::move(stmt.value()));
            }
            else{
                std::cerr << "Parse Error Expected Statement" << std::endl; // TODO make better errors
                exit(EXIT_FAILURE);
            }
        }
        return prog; //TODO not sure if this will work (but maybe its good?)
    }

private:
    std::vector<Token> m_tokens; 
    size_t m_tindex; // member token index

    std::optional<Token> peek(const int i=0){
        if (m_tindex >= 0 && m_tindex< m_tokens.size() - 1){ //the trailing newline at the end of file - can be ignored
            return m_tokens.at(m_tindex + i);
        }
        else return {}; // how cool are optionals though
    }
    std::optional<Token> peek_next_or_after_white(const int j= 0){
        if(auto t = peek(j); t.has_value() && t.value().type == TokenType::WHITESPACE){
            consume();
            return peek(1 + j); //if next is whitespace
        }
        return peek(j);
    }
    std::optional<Token> consume(){
        return m_tokens.at(m_tindex++); 
    }
    //consumes whitespace token, or exits with an error
    void consume_whitespace(){
        if(!(m_tokens.at(m_tindex).type == TokenType::WHITESPACE)){
            std::cerr << "expected whitespace at " << m_tokens.at(m_tindex).line << std::endl;
        }
        else consume();
    }
    // concumes whitespace if that is the next token, or does nothing
    void try_consume_whitespace(){
        if(m_tokens.at(m_tindex).type == TokenType::WHITESPACE){
            consume();
        }
    }
    void consume_or_error(TokenType ttype){
        if(m_tokens.at(m_tindex).type == ttype){
            consume();
        }
        else{
            std::cerr << "Bad token " << token_string(peek().value().type) << " at line " << peek().value().line << ", Expected token: " << token_string(ttype) <<"." << std::endl;
            exit(EXIT_FAILURE);
        }

    }
    void error_out(std::string err_str){
        std::cerr << "Error at line " << peek().value().line << "\nError: " << err_str << std::endl;
        exit(EXIT_FAILURE);
    }

};


#endif
