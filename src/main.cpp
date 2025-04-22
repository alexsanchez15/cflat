#include <iostream>
#include <sstream>
#include <fstream>
#include "tokenizer.hpp"
#include "parser.hpp"
#include "generator.hpp"

// i dont want this func at the top it looks bad
const std::string show_tokens(std::vector<Token> tokens);
void load_to_file(std::string filename, std::string contents);

int main(int argc, char* argv[]){
	if (argc != 2){
		std::cerr << "Incorrect usage. Need file to run" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string contents;
	{ // own scope, assuming that strstream has a destructor lol
		std::stringstream contents_stream;
		std::fstream input(argv[1], std::ios::in);
		contents_stream << input.rdbuf();
		contents = contents_stream.str(); // turn into stream, set to contents
	}

    // make tokenizer from tokenizer.hpp
    Tokenizer tokenizer(contents);
    std::vector<Token> tokens = tokenizer.tokenize();
    std::cout << show_tokens(tokens); // COMMENT THIS LINE 
    Parser parser(tokens);
    auto prog = parser.parse_prog();
    if (prog.has_value()) {
        NodeProg p = std::move(prog).value();
        p.display_self();
        Generator g(&p);
        g.generate(); // generates NASM
        std::cout <<"\n\n\nASSEMBLY GEN:\n"<< g.get_assembly() << std::endl;

        load_to_file("output.asm", g.get_assembly());
    }

	std::cout << "\nfile contents:\n"<< contents << std::endl;
	return 0;



}
const std::string show_tokens(std::vector<Token> tokens){
    std::stringstream sstream;
    for(const Token& t: tokens){
        std::string value = t.value.value_or("NONE");
        sstream << "Token Type: " << token_string(t.type) << " line: " << t.line << " value: " << value << std::endl;
    }
    return sstream.str();

}

void load_to_file(std::string filename, std::string contents){
    std::fstream output(filename, std::ios::out);
    if(!output){
        std::cerr << "error opening file: " << filename << std::endl; 
        return;
    }
    output << contents;
}

