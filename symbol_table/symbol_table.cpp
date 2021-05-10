//
// Created by Xandru Mifsud on 14/04/2021.
//

#include "symbol_table.h"

funcSymbol* symbol_table::lookup(const string& identifier, vector<symbol*>* fparams){
    for(auto curr_scope = scopeTable->rbegin(); curr_scope != scopeTable->rend(); ++curr_scope){
        if(auto ret_symbols = curr_scope->equal_range(identifier); ret_symbols.first != ret_symbols.second){
            for(auto symb_iter = ret_symbols.first; symb_iter != ret_symbols.second; symb_iter++){
                if((symb_iter->second)->object_class == grammarDFA::FUNCTION){
                    auto* curr_symbol = (funcSymbol*) symb_iter->second;
                    bool same_signature = true;

                    if(fparams->size() == 0 && curr_symbol->fparams->size() == 0){
                        same_signature = true;
                    }
                    else if(fparams->size() == curr_symbol->fparams->size()){
                        for(int i = 0; i < fparams->size(); i++){
                            if(fparams->at(i)->type != curr_symbol->fparams->at(i)->type ||
                               fparams->at(i)->object_class != curr_symbol->fparams->at(i)->object_class){
                                same_signature = false;
                                break;
                            }
                        }
                    }
                    else{
                        same_signature = false;
                    }

                    if(same_signature){
                        return (funcSymbol*) symb_iter->second;
                    }
                }
                else{
                    return nullptr;
                }
            }
        }
    }

    return nullptr;
}

symbol* symbol_table::lookup(const string& identifier){
    for(auto curr_scope = scopeTable->rbegin(); curr_scope != scopeTable->rend(); ++curr_scope){
        if(auto ret_symbols = curr_scope->equal_range(identifier); ret_symbols.first != ret_symbols.second){
            if(((ret_symbols.first)->second)->object_class != grammarDFA::FUNCTION){ // enforce use of special func lookup function with overloading support
                return (ret_symbols.first)->second;
            }
            else{
                return nullptr;
            }
        }
    }

    return nullptr;
}

bool symbol_table::insert(symbol* s){
    pair<scope::iterator, scope::iterator> ret_symbols = (scopeTable->back()).equal_range(s->identifier);

    if(ret_symbols.first != ret_symbols.second){
        auto* func_s = (funcSymbol*) s;

        if(s->object_class == grammarDFA::FUNCTION){
            for(auto symb_iter = ret_symbols.first; symb_iter != ret_symbols.second; symb_iter++){
                if((symb_iter->second)->object_class == grammarDFA::FUNCTION){
                    auto* curr_symbol = (funcSymbol*) symb_iter->second;
                    bool same_signature = true;

                    if(func_s->fparams->size() == curr_symbol->fparams->size()){
                        for(int i = 0; i < func_s->fparams->size(); i++){
                            if(func_s->fparams->at(i)->type != curr_symbol->fparams->at(i)->type ||
                               func_s->fparams->at(i)->object_class != curr_symbol->fparams->at(i)->object_class){
                                same_signature = false;
                                break;
                            }
                        }
                    }
                    else{
                        same_signature = false;
                    }

                    if(!same_signature){
                        (scopeTable->back()).insert(make_pair(s->identifier, s));
                        return true;
                    }
                }
                else{
                    return false;
                }
            }

            return false;
        }
        else{
            return false;
        }
    }
    else{
        (scopeTable->back()).insert(make_pair(s->identifier, s));
        return true;
    }
}

void symbol_table::push_scope(){
    scopeTable->push_back(*(new scope));
}

void symbol_table::pop_scope(){
    if(scopeTable->size() > 1){
        (scopeTable->back()).clear();
        scopeTable->pop_back();
    }
}