//
// Created by Xandru Mifsud on 14/04/2021.
//

#ifndef CPS2000_SYMBOL_TABLE_H
#define CPS2000_SYMBOL_TABLE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <stack>
#include "../lexer/grammarDFA.h"
#include "symbol.h"

class symbol;
class varSymbol;
class arrSymbol;
class tlsSymbol;
class funcSymbol;

using namespace std;

typedef unordered_multimap<string, symbol*> scope;

class symbol_table{
public:
    funcSymbol* lookup(const string& identifier, vector<symbol*>* fparams);
    symbol* lookup(const string& identifier);
    bool insert(symbol* s);
    void push_scope();
    void pop_scope();

    symbol_table(symbol_table* parent){
        this->parent_symbolTable = parent;
    }

private:
    vector<scope>* scopeTable = new vector<scope>(1);
    symbol_table* parent_symbolTable;
};


#endif //CPS2000_SYMBOL_TABLE_H
