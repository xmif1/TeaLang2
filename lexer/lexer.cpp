//
// Created by Xandru Mifsud on 16/03/2021.
//

#include "lexer.h"

#include <utility>

/* Constructor initialising a lexer instance, by initialising the index (i.e. # of characters
 * read in the source file) to 0 and the current line number to 1.
 */
 lexer::lexer(string input_source){
    source = std::move(input_source);
    index = 0;
    line = 1;
}

/* A simple function which fetches the next token, by maintaining a stack of states as traversed
 * on the DFA defined in grammarDFA. Returns true on success, false otherwise.
 */
bool lexer::getNextToken(Token* token_ptr){
    // initialisation
    Token token;
    token.line = 0;
    grammarDFA::State state = grammarDFA::S0; // initial state is the starting state S0
    auto* dfa = new grammarDFA();

    states_stack_clear(); // simple convenience function to clear the state stack

    // before traversing the DFA, we explicitly clear any initial sequence of whitespaces
    while(isspace(source[index])){
        // in particular we ensure that is a newline character is encountered, we increment the line counter
        // otherwise this would yield incorrect syntax and semantic error line numbers later on
        if(source[index] == '\n'){
            line++;
        }
        index++;
    }

    /* Fetch characters one by one from the input source, constructing the lexeme and traversing the
     * DFA, until the resulting state is the error state S_E i.e. an invalid transition occured.
     *
     * While doing so, we append the characters to the token.lexeme string, constructing the lexeme.
     */
    while(state != grammarDFA::S_E){
        token.lexeme += source[index]; // construct lexeme

        // we maintain a cleared state stack unless the current lexeme has no attributed meaning (T_INVALID)
        // i.e. if not at some final state of the DFA
        if(dfa->state_tok(state, &token.lexeme) != grammarDFA::T_INVALID){
            states_stack_clear();
        }

        states_stack.push(state);
        state = dfa->transition(state, source[index]);

        index++;
    }

    // Call the rollback loop, which restores the lexer to the last encountered valid final state
    while(rollback(&token, &state));

    // If rollback was successful, then the current state should not be attributed to a T_INVALID token
    if(dfa->state_tok(state, &token.lexeme) != grammarDFA::T_INVALID){
        // populate token with the final attributed meaning of the lexeme and line number
        token.symbol = dfa->state_tok(state, &token.lexeme);
        token.line = line;

        // comment may terminate with \n or \0; these must be truncated from lexeme
        if(token.symbol == grammarDFA::T_COMMENT && (token.lexeme.back() == '\n' || token.lexeme.back() == '\0')){
            token.lexeme = token.lexeme.substr(0,token.lexeme.length()-1);
            index--;
        }
        else if(token.symbol == grammarDFA::T_STRING || token.symbol == grammarDFA::T_COMMENT){
            for(char c : token.lexeme){
                if(c == '\n'){
                    line++;
                }
            }
        }

        // lastly we set the passed Token pointer to the resulting Token instance
        *token_ptr = token;
        return true; // and return true on successful fetching of the token
    }
    else{ // otherwise rollback failed and we return false
        return false;
    }
}

/* Fetches the next k tokens, and then restores the internal state of the lexer to that
 * before the call to peekTokens. Note that we inherently ignore comment tokens here, i.e.
 * we fetch k non-comment tokens.
 *
 * Returns true on success, false otherwise (when tokenisation fails, incl. if the EOF is
 * reached before the requested k tokens are fetched).
 */
 bool lexer::peekTokens(Token* token_array, int k){
     // maintain internal state of lexer before call to peekTokens
    unsigned long int curr_index = index;
    unsigned int curr_line = line;
    bool ret = true;

    // Repeatedly call getNextToken until k non-comment tokens are fetched or tokensiation fails
    for(int i = 0; i < k; i++){
        if(!getNextToken(token_array + i)){ // this call changes the internal state of the lexer
            ret = false;
            break;
        }

        if((token_array + i)->symbol == grammarDFA::T_COMMENT){
            i--; // ignore comment
        }
    }

    // restore the internal state of the lexer
    index = curr_index;
    line = curr_line;
    return ret;
}

// Simple function that 'clears' the state stack
void lexer::states_stack_clear(){
    while(!states_stack.empty()){
        states_stack.pop();
    }
}

/* The rollback function, which pops off the top state from the state stack, decrements the char
 * pointer in the soource file by 1 and truncates the lexeme by 1 character.
 *
 * Returns true if internal state of the lexer has not been restored to the last occurring final
 * state of the DFA, otherwise it returns false.
 */
bool lexer::rollback(Token* token_ptr, grammarDFA::State* state_ptr){
    if(!states_stack.empty()){ // if not restored to last occurring final state
        *state_ptr = states_stack.top(); // maintain reference to state

        states_stack.pop(); // pop off state
        // truncate the lexeme by 1 character to restore it to a valid lexeme
        token_ptr->lexeme = token_ptr->lexeme.substr(0,token_ptr->lexeme.length()-1);

        index--; // and decrement char reference in source file

        return true;
    }
    else{
        return false;
    }
}