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

// Since Tea2Lang supports named types, we maintain the type name as well now. So for a tlstruct definition named 'foo1',
// the associated type would be the <T_TLSTRUCT, 'foo1'> (i.e. the symbol represents a foo1 tlstruct instance).
typedef pair<grammarDFA::Symbol, string> type_t;

/* We also extend the means by which we maintain right-values in Tea2Lang, since we now must also maintain tlstruct instances
 * as well as arrays. For a tlstruct instance, we maintain a symbol table representing the internal state of all member
 * symbols of the tlstruct. Hence literal_t has been extended to support pointers to symbol_table instances.
 *
 * To support arrays, literal_arr_t was defined, which is simply syntactic sugar for defining a vector of literal_t pointers.
 *
 * Lastly, since a right-value can either be a singular value (SINGLETON) or a collection of values (ARRAY), we define a
 * new variant, obj_t, which handles literal_t and literal_arr_t. This is the new type used to maintain right-values in
 * symbol instances for Tea2Lang.
 */
typedef variant<bool, int, float, char, string, symbol_table*> literal_t;
typedef vector<literal_t>* literal_arr_t;
typedef variant<literal_t, literal_arr_t> obj_t;

/* Defines an instance of a symbol table entry, outlining the minimum amount of meta-data to be held. Derivatives of this
 * class may add further meta-data requirements. At a minimum, on instantiation we must maintain:
 * (i)   An identifier which (uniquely) identifies the symbol for lookup etc
 * (ii)  An associated type (int, string, int array, etc)
 *
 * A setter is also provided for associating a value (be it a literal, tlstruct or array) with the symbol. This is the
 * right-value associated with the identifier (also known as the left-value).
 */
class symbol{
public:
    string identifier;
    type_t type;
    grammarDFA::Symbol object_class; // replaces the id_type variable, an instance of the IdentifierType enum in TeaLang
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
        object_class = grammarDFA::SINGLETON; // SINGLETON = singular value
    };
};

class arrSymbol: public symbol{
public:
    int size;

    arrSymbol(string* identifier, type_t type, int size) : symbol(identifier, type){
        object_class = grammarDFA::ARRAY; // ARRAY = collection of values (in contrast to SINGLETON)
        this->size = size; // size of the collection (which is fixed)
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
