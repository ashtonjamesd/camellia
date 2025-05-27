#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef enum {
    INVALID_ESCAPE_SEQUENCE,
    EMPTY_CHAR_LITERAL,
    UNTERMINATED_STRING_LITERAL,
    TOO_MANY_CHARS_IN_CHAR_LITERAL,
    INVALID_NUMERIC_TOKEN,
    INVALID_SYMBOL,
    NO_LEXER_ERROR
} LexErr;

typedef struct {
    char  *source;
    Token *tokens;
    int    current;
    int    token_capacity;
    int    token_count;
    int    line;

    // whether to print debug information, used in development
    int    debug;

    // the error that occurred, null if none took place
    LexErr err;
} Lexer;

Lexer *init_lexer(char *source, int debug);
void tokenize(Lexer *lexer);
void free_lexer(Lexer *lexer);

#endif