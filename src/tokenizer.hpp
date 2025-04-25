//tokenizer.hpp file
#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <cstdlib>
#include <string>
#include <optional>
#include <vector>
#include <regex>
#include <iostream>

// set up token types:
enum class TokenType{
	EXIT, KEYWORD, IDENTIFIER, EQUALS, INT_LITERAL, SEMI, WHITESPACE, RPAREN, LPAREN, PLUS, STAR, STRING, PRINT, PRINTLN, LCURL, RCURL, IF, BEQ, WHILE, LESSTHAN, 
};
struct Token{
    TokenType type;
    size_t line;
std::optional<std::string> value {};
};

//perhaps will be useful for other things, but for now good for testing if my tokenizer works as expected
inline std::string token_string(const TokenType type){
    switch(type){
        case TokenType::KEYWORD:
            return "KEYWORD";
        case TokenType::EQUALS:
            return "EQUALS";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::INT_LITERAL:
            return "INT_LITERAL";
        case TokenType::SEMI:
            return "SEMI";
        case TokenType::WHITESPACE:
            return "WHITESPACE";
        case TokenType::EXIT:
            return "EXIT";
        case TokenType::RPAREN:
            return "RPAREN";
        case TokenType::LPAREN:
            return "LPAREN";
        case TokenType::PLUS:
            return "PLUS";
        case TokenType::STAR:
            return "STAR";
        case TokenType::STRING:
            return "STRING";
        case TokenType::PRINT:
            return "PRINT";
        case TokenType::PRINTLN:
            return "PRINTLN";
        case TokenType::RCURL:
            return "RCURL";
        case TokenType::LCURL:
            return "LCURL";
        case TokenType::IF:
            return "IF";
        case TokenType::BEQ:
            return "BEQ"; // boolean equals
        case TokenType::WHILE:
            return "WHILE"; 
        case TokenType::LESSTHAN:
            return "LESSTHAN"; 
        default:
            return "UNKNOWN"; //should never happen, avoid compilier being mad
    }
}

inline int tok_prec(const TokenType type){
    switch(type){
        case TokenType::PLUS:
            return 1;
        case TokenType::STAR:
            return 2;
        case TokenType::BEQ:
        case TokenType::LESSTHAN:
            return 0;
        default:
            std::cerr << "Bad token passed to tok_prec (precedance mapping) function: " << token_string(type) << std::endl;
            return -1; //should never happen, avoid compilier being mad
    }
}
inline bool is_bin_exp_op(const TokenType type){
    switch(type){
        case TokenType::PLUS:
        case TokenType::STAR:
        case TokenType::BEQ:
        case TokenType::LESSTHAN:
            return true;
        default:
            return false;
    }
}
// idea is to use regular expressions to determine token types

class Tokenizer{
public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src)){}
    std::vector<Token> tokenize(){
        std::vector<Token> tokens;
        std::regex pattern
            (R"(\b(exit|int|print|println|string|if|while)\b|"[^"]*"|[a-zA-Z_][a-zA-Z0-9_]*|(==)|[=,;,<,\(,\),+,*, \{,\}]|[\-]?\d+|\s+|.+)");
        std::sregex_iterator itr(m_src.begin(), m_src.end(), pattern);
        std::sregex_iterator end_marker;

        while(itr != end_marker){
            std::smatch match = *itr;
            std::string match_str = match.str();
            itr++; //iterate
            //create tokens:
            if(match_str == "exit"){ // direct string comp fast, use when i can
                tokens.push_back({TokenType::EXIT, m_index});
            }
            else if(match_str == "print"){ // direct string comp fast, use when i can
                tokens.push_back({TokenType::PRINT, m_index});
            }
            else if(match_str == "println"){ // direct string comp fast, use when i can
                tokens.push_back({TokenType::PRINTLN, m_index});
            }
            else if(match_str == "if"){ // direct string comp fast, use when i can
                tokens.push_back({TokenType::IF, m_index});
            }
            else if(match_str == "while"){ // direct string comp fast, use when i can
                tokens.push_back({TokenType::WHILE, m_index});
            }
            else if(match_str == "int" ||  match_str == "string"){ //str
                tokens.push_back({TokenType::KEYWORD, m_index, match_str});
            }
            else if(std::regex_match(match_str, std::regex(R"("[^"]*")"))){ // direct string comp fast, use when i can
                std::string str_no_quotes = match_str.substr(1, match_str.size()-2); // strip quotes
                tokens.push_back({TokenType::STRING, m_index, str_no_quotes});
            }

            else if(std::regex_match(match_str, std::regex(R"([\-]?\d+)"))){
                tokens.push_back({TokenType::INT_LITERAL, m_index, match_str});
            }
            else if(std::regex_match(match_str, std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)"))){
                tokens.push_back({TokenType::IDENTIFIER, m_index, match_str});
            }
            else if(std::regex_match(match_str, std::regex(R"(\s+)"))){
                tokens.push_back({TokenType::WHITESPACE, m_index, std::to_string(match_str.size())});
            }
            else if(match_str == "=="){ // direct string comp fast, use when i can
                tokens.push_back({TokenType::BEQ, m_index});
            }
            else if(match_str[0] == '=' && match_str.size() == 1){
                tokens.push_back({TokenType::EQUALS, m_index}); 
            }
            else if(match_str[0] == ')' && match_str.size() == 1){
                tokens.push_back({TokenType::RPAREN, m_index}); 
            }
            else if(match_str[0] == '(' && match_str.size() == 1){
                tokens.push_back({TokenType::LPAREN, m_index}); 
            }
            else if(match_str[0] == ';' && match_str.size() == 1){
                tokens.push_back({TokenType::SEMI, m_index}); 
            }
            else if(match_str[0] == '<' && match_str.size() == 1){
                tokens.push_back({TokenType::LESSTHAN, m_index}); 
            }
            else if(match_str[0] == '+' && match_str.size() == 1){
                tokens.push_back({TokenType::PLUS, m_index}); 
            }
            else if(match_str[0] == '*' && match_str.size() == 1){
                tokens.push_back({TokenType::STAR, m_index}); 
            }
            else if(match_str[0] == '{' && match_str.size() == 1){
                tokens.push_back({TokenType::LCURL, m_index}); 
            }
            else if(match_str[0] == '}' && match_str.size() == 1){
                tokens.push_back({TokenType::RCURL, m_index}); 
            }
            else{
                //string not tokenizable, and not white space, so this is bad.
            std::cerr << "Token at " << m_index << " is unknown: " << match_str << "." << std::endl;
                exit(EXIT_FAILURE);
            }

            m_index += match_str.size(); // iterate by size of this string
            // works because white spaces are grabbed individually (kind of inefficient..but simple)
        }



        return tokens;

    }


private:
	const std::string m_src; //object member source string
    size_t m_index = 0;
};


#endif // ends the ifndef (gaurds)
