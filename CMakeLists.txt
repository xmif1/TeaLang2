cmake_minimum_required(VERSION 3.17)
project(TeaLang2)

set(CMAKE_CXX_STANDARD 17)

add_executable(TeaLang2 main.cpp lexer/lexer.cpp lexer/lexer.h lexer/grammarDFA.cpp lexer/grammarDFA.h parser/parser.cpp parser/parser.h visitor_ast/astNode.cpp visitor_ast/astNode.h visitor_ast/visitor.h semantic_analysis/graphviz_example/graphviz_ast_visitor.cpp semantic_analysis/graphviz_example/graphviz_ast_visitor.h)