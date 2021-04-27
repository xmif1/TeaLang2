//
// Created by Xandru Mifsud on 13/04/2021.
//

#ifndef CPS2000_SYMBOL_H
#define CPS2000_SYMBOL_H

#include <utility>
#include <variant>
#include <vector>
#include <string>
#include "../visitor_ast/astNode.h"
#include "../lexer/grammarDFA.h"

using namespace std;

typedef variant<bool, int, float, char, string> literal_t;

enum IdentifierType{
    VAR, FUNC
};

class symbol{
public:
    string identifier;
    grammarDFA::Symbol type;
    IdentifierType id_type;
    literal_t literal;
    
    symbol(string* identifier, grammarDFA::Symbol type){
        if(identifier != nullptr){
            this->identifier = *identifier;
        }
        this->type = type;
    }

    void set_literal(literal_t value){
        this->literal = std::move(value);
    }
};

class varSymbol: public symbol{
public:

    varSymbol(string* identifier, grammarDFA::Symbol type) : symbol(identifier, type){
        id_type = VAR;
    };
};

class funcSymbol: public symbol{
public:
    vector<varSymbol*>* fparams;
    astBLOCK* func_ref;

    funcSymbol(string* identifier, grammarDFA::Symbol type, vector<varSymbol*>* fparams) : symbol(identifier, type){
        id_type = FUNC;
        this->fparams = fparams;
    };

    void set_func_ref(astBLOCK* func_node_ptr){
        this->func_ref = func_node_ptr;
    }
};

#endif //CPS2000_SYMBOL_H
