//
// Created by Xandru Mifsud on 16/03/2021.
//

#ifndef CPS2000_LEXER_H
#define CPS2000_LEXER_H

#include <string>
#include <stack>
#include <vector>
#include "grammarDFA.h"

using namespace std;

class lexer{
public:
    struct Token{
        grammarDFA::Symbol symbol;
        string lexeme;
        unsigned int line;
    };

    bool getNextToken(Token*);
    bool peekTokens(Token*, int);
    explicit lexer(string);

private:
    stack<grammarDFA::State> states_stack;

    unsigned long int index;
    unsigned int line;
    string source;

    bool rollback(Token*, grammarDFA::State*);
    void states_stack_clear();
};

#endif //CPS2000_LEXER_H
