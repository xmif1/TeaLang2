#include <fstream>
#include <sstream>
#include <string>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic_analysis/semantic_analysis.h"
#include "semantic_analysis/graphviz_example/graphviz_ast_visitor.h"
#include "interpreter/interpreter.h"

/* Main class running the entire compilation pipeline. Execute as:
 * ./main <source> [-v=1]
 * where <source> is an absolute file path to a .txt file with TeaLang source and -v=1 is an optional flag which if set,
 * a .dot file is outputted with graphviz code representing the abstract syntax tree.
 */
int main(int argc, char *argv[]){
    bool graphviz_on = false;

    // option checking...
    if(argc == 1){
        throw std::runtime_error("Source file not specified...exiting...");
    }
    else if(argc > 3){
        throw std::runtime_error("Invalid options specified (please only specify source and optionally -v=1)...exiting...");
    }
    else if(argc == 3 && strcmp(argv[2], "-v=1") == 0){
        graphviz_on = true;
    }

    // open file at path specified and validate
    std::ifstream source_file(argv[1]);
    std::stringstream source_buffer;
    source_buffer << source_file.rdbuf();

    // hold reference to the file name and extension
    string filename = argv[1];
    string extension;

    // extract filename with extentsion from the path
    int last_slash_idx = filename.find_last_of("\\/");
    if(std::string::npos != last_slash_idx){
        filename.erase(0, last_slash_idx + 1);
    }

    // extract extension from the path
    int period_idx = filename.rfind('.');
    if(std::string::npos != period_idx){
        extension = filename.substr(period_idx + 1);
        filename.erase(period_idx);
    }

    // check if .tlg file
    if(extension != "tlg"){
        throw std::runtime_error("Source file is not a TeaLang program (does not have a .tlg extension)...exiting...");
    }

    // create new lexer and parser instance
    auto* lex = new lexer(source_buffer.str());
    auto* par = new parser(lex); // carries out syntax analysis

    // if using graphviz, draw AST in dot format; output <filename>.dot at the source file path
    if(graphviz_on){
        auto *gav = new graphviz_ast_visitor(filename);
        par->root->accept(gav);
    }

    // create new semantic_analysis instance and traverse AST returned by parser via visitor design pattern
    auto* sa = new semantic_analysis();
    par->root->accept(sa);

    // if no syntax or semantic error occured
    if(par->err_count == 0 && sa->err_count == 0){
        // then interpret by creating a new interpreter instance and traversing AST via visitor design pattern
        auto* itpr = new interpreter();
        par->root->accept(itpr);
    }
    else{
        return 1;
    }

    return 0;
}