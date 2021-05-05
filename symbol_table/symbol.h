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
typedef vector<literal_t>* literal_arr_t;
typedef variant<literal_t, literal_arr_t> obj_t;

class symbol{
public:
    string identifier;
    grammarDFA::Symbol type;
    grammarDFA::Symbol object_class;
    obj_t object;
    
    symbol(string* identifier, grammarDFA::Symbol type){
        if(identifier != nullptr){
            this->identifier = *identifier;
        }
        this->type = type;
    }

    void set_object(obj_t value){
        this->object = std::move(value);
    }
};

class varSymbol: public symbol{
public:

    varSymbol(string* identifier, grammarDFA::Symbol type) : symbol(identifier, type){
        object_class = grammarDFA::SINGLETON;
    };
};

class arrSymbol: public symbol{
public:
    int size;

    arrSymbol(string* identifier, grammarDFA::Symbol type, int size) : symbol(identifier, type){
        object_class = grammarDFA::ARRAY;
        this->size = size;
    };
};

class funcSymbol: public symbol{
public:
    grammarDFA::Symbol ret_obj_class;
    vector<symbol*>* fparams;
    astBLOCK* func_ref;

    funcSymbol(string* identifier, grammarDFA::Symbol type, grammarDFA::Symbol ret_obj_class,
               vector<symbol*>* fparams) : symbol(identifier, type){

        object_class = grammarDFA::FUNCTION;
        this->ret_obj_class = ret_obj_class;
        this->fparams = fparams;
    };

    void set_func_ref(astBLOCK* func_node_ptr){
        this->func_ref = func_node_ptr;
    }
};

#endif //CPS2000_SYMBOL_H
