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

    if(argc == 1){
        throw std::runtime_error("Source file not specified...exiting...");
    }
    else if(argc > 3){
        throw std::runtime_error("Invalid options specified (please only specify source and optionally -v=1)...exiting...");
    }
    else if(argc == 3 && strcmp(argv[2], "-v=1") == 0){
        graphviz_on = true;
    }

    std::ifstream source_file(argv[1]);
    std::stringstream source_buffer;
    source_buffer << source_file.rdbuf();

    auto* lex = new lexer(source_buffer.str());
    auto* par = new parser(lex);

    if(graphviz_on){
        auto *gav = new graphviz_ast_visitor();
        par->root->accept(gav);
    }

    auto* sa = new semantic_analysis();
    par->root->accept(sa);

    if(par->err_count == 0 && sa->err_count == 0){
        auto* itpr = new interpreter();
        par->root->accept(itpr);
    }
    else{
        return 1;
    }

    return 0;
}