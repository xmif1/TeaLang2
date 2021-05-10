//
// Created by Xandru Mifsud on 13/04/2021.
//

#ifndef CPS2000_SYMBOL_H
#define CPS2000_SYMBOL_H

#include <utility>
#include <variant>
#include <vector>
#include <string>
#include "symbol_table.h"
#include "../visitor_ast/astNode.h"
#include "../lexer/grammarDFA.h"

class symbol_table;

using namespace std;

typedef pair<grammarDFA::Symbol, string> type_t;
typedef variant<bool, int, float, char, string, symbol_table*> literal_t;
typedef vector<literal_t>* literal_arr_t;
typedef variant<literal_t, literal_arr_t> obj_t;

class symbol{
public:
    string identifier;
    type_t type;
    grammarDFA::Symbol object_class;
    obj_t object;
    
    symbol(string* identifier, type_t type){
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

    varSymbol(string* identifier, type_t type) : symbol(identifier, type){
        object_class = grammarDFA::SINGLETON;
    };
};

class arrSymbol: public symbol{
public:
    int size;

    arrSymbol(string* identifier, type_t type, int size) : symbol(identifier, type){
        object_class = grammarDFA::ARRAY;
        this->size = size;
    };
};

class tlsSymbol: public symbol{
public:
    astBLOCK* tls_ref;

    tlsSymbol(string* identifier, string* struct_name) : symbol(identifier, type_t(grammarDFA::T_TLSTRUCT, *struct_name)){
        object_class = grammarDFA::SINGLETON;
    };

    void set_tls_ref(astBLOCK* tls_node_ptr){
        this->tls_ref = tls_node_ptr;
    }
};

class funcSymbol: public symbol{
public:
    grammarDFA::Symbol ret_obj_class;
    vector<symbol*>* fparams;
    astBLOCK* func_ref;

    funcSymbol(string* identifier, type_t type, grammarDFA::Symbol ret_obj_class,
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
