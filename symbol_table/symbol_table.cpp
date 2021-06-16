//
// Created by Xandru Mifsud on 14/04/2021.
//

#include "symbol_table.h"

/* Lookup function for funcSymbol instances in a (linked) symbol_table instance, based on a given identifier and vector
 * (possibly empty) of pointers to varSymbol instances whose types (in order) represent the type-signature of the
 * funcSymbol being looked up. In this manner, we support function overloading.
 *
 * Returns a valid pointer to funcSymbol instance if a match is found; otherwise nullptr is returned.
 */
funcSymbol* symbol_table::lookup(const string& identifier, vector<symbol*>* fparams){
    // begin by iterating through the scopes, recalling that scopes can be considered as nested subsets: S_i < S_{i+1}.
    for(auto curr_scope = scopeTable->rbegin(); curr_scope != scopeTable->rend(); ++curr_scope){
        // fetch symbols from unordered hashmap matching passed identifier and if returned iterator does not have its
        // start and end equal (i.e. at least one symbol instance with matching identifier found)
        if(auto ret_symbols = curr_scope->equal_range(identifier); ret_symbols.first != ret_symbols.second){
            // iterate across all returned symbol instances, searching for one with a matching function type-signature
            for(auto symb_iter = ret_symbols.first; symb_iter != ret_symbols.second; symb_iter++){
                // if symbol instance is a funcSymbol instance
                if((symb_iter->second)->object_class == grammarDFA::FUNCTION){
                    auto* curr_symbol = (funcSymbol*) symb_iter->second; // extract pointer to funcSymbol instance
                    bool same_signature = true; // maintain flag if signature is the same

                    // if no parameters in both, then we have a match
                    if(fparams->size() == 0 && curr_symbol->fparams->size() == 0){
                        same_signature = true;
                    }
                    // else if they have the same number (non--zero) of parameters, check if the sequence of types is the
                    // same by iterating through the respective fparams vectors; if mismatch is found in type or object
                    // class, same_signature = false and we break.
                    else if(fparams->size() == curr_symbol->fparams->size()){
                        for(int i = 0; i < fparams->size(); i++){
                            if(fparams->at(i)->type != curr_symbol->fparams->at(i)->type ||
                               fparams->at(i)->object_class != curr_symbol->fparams->at(i)->object_class){
                                same_signature = false;
                                break;
                            }
                        }
                    }
                    else{ // else if they do not have the same number of parameters, same_signature = false
                        same_signature = false;
                    }

                    // if same_signature = true, match found hence we break and stop searching in upper scopes
                    if(same_signature){
                        return (funcSymbol*) symb_iter->second;
                    }
                }
                else{ // else identifier is bound to a symbol instance which is not a funcSymbol instance i.e assuming
                      // correctness of symbol_table, a funcSymbol with the same identifier cannot exist
                    return nullptr;
                }
            }
        }
    }

    // if no match found in current symbol table, lookup in parent symbol table (used for nested structs etc)
    if(parent_symbolTable != nullptr){
        return parent_symbolTable->lookup(identifier, fparams);
    }

    return nullptr;
}

/* Lookup function for symbol instances in a (linked) symbol_table instance, based on a given identifier.
 *
 * Returns a valid pointer to symbol instance if a match is found; otherwise nullptr is returned. In the case that the
 * identifier is bound to a funcSymbol instance, a nullptr is still returned, since we support function overloading and
 * hence we require the type-signature as well. This enforces the use of the specialised lookup function for functions.
 */
symbol* symbol_table::lookup(const string& identifier){
    // begin by iterating through the scopes, recalling that scopes can be considered as nested subsets: S_i < S_{i+1}.
    for(auto curr_scope = scopeTable->rbegin(); curr_scope != scopeTable->rend(); ++curr_scope){
        /* Fetch symbols from unordered hashmap matching passed identifier and if returned iterator does not have its
         * start and end equal (i.e. at least one symbol instance with matching identifier found).
         *
         * Indeed, assuming correctness of the symbol_table instance, for non-funcSymbol symbol instance, there should
         * be at most one matching symbol instance with the same identifier.
         */
        if(auto ret_symbols = curr_scope->equal_range(identifier); ret_symbols.first != ret_symbols.second){
            // enforce use of special func lookup function with overloading support
            if(((ret_symbols.first)->second)->object_class != grammarDFA::FUNCTION){
                return (ret_symbols.first)->second; // return matching symbol instance
            }
            else{
                return nullptr;
            }
        }
    }

    // if no match found in current symbol table, lookup in parent symbol table (used for nested structs etc)
    if(parent_symbolTable != nullptr){
        return parent_symbolTable->lookup(identifier);
    }

    return nullptr;
}

// Insertion function with support for function overloading, ensuring uniqueness of identifier across as symbol types.
// Returns true whenever insertion is successful, false otherwise.
bool symbol_table::insert(symbol* s){
    // Fetch symbols from unordered hashmap matching passed identifier
    pair<scope::iterator, scope::iterator> ret_symbols = (scopeTable->back()).equal_range(s->identifier);

    /* If returned iterator does not have its start and end equal (i.e. at least one symbol instance with matching
     * identifier found), we check if we are dealing with funcSymbol instances. If yes, we check if the type-signature
     * of the symbol being inserted is unique. If we are not dealing with funcSymbol instances, then there can only be
     * one symbol with the identifier and hence we return false. Whenever false is returned, an appropriate semantical
     * error should be reported in the semantic analysis phase.
     */
    if(ret_symbols.first != ret_symbols.second){
        auto* func_s = (funcSymbol*) s;

        if(s->object_class == grammarDFA::FUNCTION){ // check if funcSymbol instance
            bool atleast_one_same_signature = false;

            // begin iterating through symbols returned; if at least one is not a funcSymbol instance, then return false
            // as the same identifier cannot be used for distinct symbol concrete implementations
            for(auto symb_iter = ret_symbols.first; symb_iter != ret_symbols.second; symb_iter++){
                // if symbol instance is a funcSymbol instance
                if((symb_iter->second)->object_class == grammarDFA::FUNCTION){
                    auto* curr_symbol = (funcSymbol*) symb_iter->second; // extract pointer to funcSymbol instance
                    bool same_signature = true; // maintain flag if signature is the same

                    // check if the sequence of types is the same by iterating through the respective fparams vectors;
                    // if mismatch is found in type or object class, same_signature = false and we break i.e. insert
                    if(func_s->fparams->size() == curr_symbol->fparams->size()){
                        for(int i = 0; i < func_s->fparams->size(); i++){
                            if(func_s->fparams->at(i)->type != curr_symbol->fparams->at(i)->type ||
                               func_s->fparams->at(i)->object_class != curr_symbol->fparams->at(i)->object_class){
                                same_signature = false;
                                break;
                            }
                        }
                    }
                    else{ // else if different number of parameters, then trivially distinct
                        same_signature = false;
                    }

                    atleast_one_same_signature = atleast_one_same_signature || same_signature;
                }
                else{ // same identifier cannot be used for distinct symbol concrete implementations
                    return false;
                }
            }

            if(!atleast_one_same_signature){ // if type signatures are all distinct, insert symbol
                (scopeTable->back()).insert(make_pair(s->identifier, s));
                return true;
            }

            return false; // match found, i.e. same_signature = true at some point and we did not return earlier
        }
        else{
            // in the case input is a non-funcSymbol instance and entries in the symbol table correspond to a funcSymbol
            return false; // same identifier cannot be used for distinct symbol concrete implementations
        }
    }
    else{ // if no symbol with matching identifier found in the symbol table, simply insert and return true
        (scopeTable->back()).insert(make_pair(s->identifier, s));
        return true;
    }
}

// Convenience function for adding a new scope to the scope table.
void symbol_table::push_scope(){
    scopeTable->push_back(*(new scope));
}

// Convenience function for removing the top--most scope from the scope table, with protection from deletion of the
// global scope.
void symbol_table::pop_scope(){
    if(scopeTable->size() > 1){
        (scopeTable->back()).clear();
        scopeTable->pop_back();
    }
}