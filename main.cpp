#include <fstream>
#include <sstream>
#include <string>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic_analysis/semantic_analysis.h"
#include "semantic_analysis/graphviz_example/graphviz_ast_visitor.h"
// #include "interpreter/interpreter.h"

int main(){
    std::ifstream source_file("/Users/xandrumifsud/Documents/Dev/TeaLang2/example_scripts/array_test.txt");
    std::stringstream source_buffer;
    source_buffer << source_file.rdbuf();

    auto* lex = new lexer(source_buffer.str());
    auto* par = new parser(lex);
    // auto* gav = new graphviz_ast_visitor();
    auto* sa = new semantic_analysis();
    // auto* itpr = new interpreter();

    // par->root->accept(gav);
    par->root->accept(sa);
    // par->root->accept(itpr);

    return 0;
}

// interpreter/interpreter.cpp interpreter/interpreter.h