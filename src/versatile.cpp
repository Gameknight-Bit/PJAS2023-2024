#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

#include "./tokenization.hpp"
#include "./parser.hpp"
#include "./generator.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. \nUse veratile <input.vst>" << std::endl;
        return EXIT_FAILURE;
    }

    //Getting File Contents
    std::string content;
    {
        std::stringstream contents_s;
        std::ifstream input(argv[1]);
        contents_s << input.rdbuf();
        content = contents_s.str();
    }
    //

    //Tokenize the File :)
    Tokenizer tokenizer(std::move(content));
    std::vector<Token> tokens = tokenizer.tokenize();

    Parser parser(std::move(tokens));
    std::optional<NodeProg> tree = parser.parse_prog();

    if (!bool(tree)) {
        std::cerr <<"Invalid program!" << std::endl;
        exit(EXIT_FAILURE);
    }

    Generator generator(tree.value());

    //Output to ASM File
    {
        std::ofstream file("../out.asm");
        file << generator.generate_prog();
    }

    system("nasm -felf64 ../out.asm");
    system("ld -o ../out ../out.o");

    return EXIT_SUCCESS;
}