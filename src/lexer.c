#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"

static SymbolToken KEYWORDS[] = {
    {"int", TOKEN_INT},
    {"return", TOKEN_RETURN},
    {"char", TOKEN_CHAR},
    {"float", TOKEN_FLOAT},
    {"double", TOKEN_DOUBLE},
    {"struct", TOKEN_STRUCT},
    {"union", TOKEN_UNION},
    {"long", TOKEN_LONG},
    {"short", TOKEN_SHORT},
    {"unsigned", TOKEN_UNSIGNED},
    {"auto", TOKEN_AUTO},
    {"register", TOKEN_REGISTER},
    {"typedef", TOKEN_TYPEDEF},
    {"static", TOKEN_STATIC},
    {"goto", TOKEN_GOTO},
    {"sizeof", TOKEN_SIZEOF},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"for", TOKEN_FOR},
    {"do", TOKEN_DO},
    {"while", TOKEN_WHILE},
    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},
    {"void", TOKEN_VOID},
    {"const", TOKEN_CONST},
    {"asm", TOKEN_ASM},
    {"volatile", TOKEN_VOLATILE},
};

static SymbolToken SINGLE_SYMBOLS[] = {
    {";", TOKEN_SEMICOLON},
    {"(", TOKEN_LEFT_PAREN},
    {")", TOKEN_RIGHT_PAREN},
    {"{", TOKEN_LEFT_BRACE},
    {"}", TOKEN_RIGHT_BRACE},
    {"=", TOKEN_SINGLE_EQUALS},
    {",", TOKEN_COMMA},
    {"#", TOKEN_HASHTAG},
    {"+", TOKEN_PLUS},
    {"-", TOKEN_MINUS},
    {"*", TOKEN_STAR},
    {"/", TOKEN_SLASH},
    {"%", TOKEN_MODULO},
    {"~", TOKEN_BITWISE_NOT},
    {"!", TOKEN_EXCLAMATION},
    {">", TOKEN_GREATER_THAN},
    {"<", TOKEN_LESS_THAN},
    {".", TOKEN_DOT},
    {"[", TOKEN_SQUARE_BRACKET_LEFT},
    {"]", TOKEN_SQUARE_BRACKET_RIGHT},
    {"?", TOKEN_QUESTION},
    {":", TOKEN_COLON},
    {"&", TOKEN_BITWISE_AND},
    {"|", TOKEN_BITWISE_OR},
    {"^", TOKEN_BITWISE_XOR},
};

static SymbolToken DOUBLE_SYMBOLS[] = {
    {">=", TOKEN_GREATER_THAN_EQUALS},
    {"<=", TOKEN_LESS_THAN_EQUALS},
    {"==", TOKEN_EQUALS},
    {"!=", TOKEN_NOT_EQUALS},
    {"&&", TOKEN_AND},
    {"||", TOKEN_OR},
    {"++", TOKEN_INCREMENT},
    {"--", TOKEN_DECREMENT},
    {"->", TOKEN_ARROW_OP},
    {">>", TOKEN_BITWISE_RIGHT_SHIFT},
    {"<<", TOKEN_BITWISE_LEFT_SHIFT},
    {"+=", TOKEN_PLUS_EQUALS},
    {"-=", TOKEN_MINUS_EQUALS},
    {"*=", TOKEN_STAR_EQUALS},
    {"/=", TOKEN_SLASH_EQUALS},
    {"%=", TOKEN_MODULO_EQUALS},
    {"&=", TOKEN_BITWISE_AND_EQUALS},
    {"|=", TOKEN_BITWISE_OR_EQUALS},
    {"^=", TOKEN_BITWISE_XOR_EQUALS},
};

static SymbolToken TRIPLE_SYMBOLS[] = {
    {"<<=", TOKEN_BITWISE_LEFT_SHIFT_EQUALS},
    {">>=", TOKEN_BITWISE_RIGHT_SHIFT_EQUALS},
    {"...", TOKEN_ELLIPSIS},
};


static const int KEYWORDS_COUNT = sizeof(KEYWORDS) / sizeof(SymbolToken);

static const int SINGLE_SYMBOL_COUNT = sizeof(SINGLE_SYMBOLS) / sizeof(SymbolToken);
static const int DOUBLE_SYMBOL_COUNT = sizeof(DOUBLE_SYMBOLS) / sizeof(SymbolToken);
static const int TRIPLE_SYMBOL_COUNT = sizeof(TRIPLE_SYMBOLS) / sizeof(SymbolToken);

Lexer *init_lexer(char *source, int debug) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (!lexer) {
        perror("Error allocating lexer");
        return NULL;
    }

    lexer->source = strdup(source);
    lexer->current = 0;
    lexer->token_capacity = 1;
    lexer->token_count = 0;
    lexer->tokens = (Token *)malloc(sizeof(Token));
    lexer->err = NO_LEXER_ERROR;
    lexer->debug = debug;
    lexer->line = 1;

    return lexer;
}

char *lexer_err_to_str(LexErr err) {
    switch (err) {
        case EMPTY_CHAR_LITERAL: return "a char literal cannot be empty\n";
        case INVALID_ESCAPE_SEQUENCE: return "invalid escape character\n";
        case UNTERMINATED_STRING_LITERAL: return "unterminated string literal\n";
        case TOO_MANY_CHARS_IN_CHAR_LITERAL: return "too many characters in char literal\n";
        case INVALID_NUMERIC_TOKEN: return "invalid numeric declaration\n";
        case INVALID_SYMBOL: return "invalid symbol\n";
        case NO_LEXER_ERROR: return "no lexer error\n";
        default: return "unknown error - uhhhh, oops\n";
    }
}

static inline int is_end(Lexer *lexer) {
    return lexer->current >= (int)strlen(lexer->source);
}

Token *init_token(const char *lexeme, TokenType type, Lexer *lexer) {
    Token *token = (Token *)malloc(sizeof(Token));
    token->lexeme = strdup(lexeme);
    token->type = type;
    token->line = lexer->line;
    token->has_whitespace_after = !is_end(lexer) ? lexer->source[lexer->current + 1] == ' ' : 0;

    return token;
}

static inline char current_char(Lexer *lexer) {
    if (lexer->source == NULL) return '\0';

    if (lexer->current >= (int)strlen(lexer->source)) return '\0';
    return lexer->source[lexer->current];
}

static inline int match(char c, Lexer* lexer) {
    return current_char(lexer) == c;
}

static inline void advance(Lexer *lexer) {
    lexer->current++;
}

static inline void recede(Lexer *lexer) {
    lexer->current--;
}

static inline void lexer_err(LexErr error, Lexer *lexer) {
    lexer->err = error;
}

static Token *parse_symbol(Lexer *lexer) {
    for (int i = 0; i < TRIPLE_SYMBOL_COUNT; i++) {
        const char *symbol = TRIPLE_SYMBOLS[i].symbol;
        size_t len = strlen(symbol);
        if (strncmp(&lexer->source[lexer->current], symbol, len) == 0) {
            Token *token = init_token(symbol, TRIPLE_SYMBOLS[i].type, lexer);
            lexer->current += len - 1;
            return token;
        }
    }

    for (int i = 0; i < DOUBLE_SYMBOL_COUNT; i++) {
        const char *symbol = DOUBLE_SYMBOLS[i].symbol;
        size_t len = strlen(symbol);
        if (strncmp(&lexer->source[lexer->current], symbol, len) == 0) {
            Token *token = init_token(symbol, DOUBLE_SYMBOLS[i].type, lexer);
            lexer->current += len - 1;
            return token;
        }
    }

    for (int i = 0; i < SINGLE_SYMBOL_COUNT; i++) {
        const char *symbol = SINGLE_SYMBOLS[i].symbol;
        size_t len = strlen(symbol);
        if (strncmp(&lexer->source[lexer->current], symbol, len) == 0) {
            return init_token(symbol, SINGLE_SYMBOLS[i].type, lexer);
        }
    }

    lexer_err(INVALID_SYMBOL, lexer);
    return NULL;
}

static inline int is_valid_esc(char c) {
    return c == 'a' 
        || c == 'b'
        || c == 'f'
        || c == 'n' 
        || c == 't'
        || c == 'v'
        || c == '\\'
        || c == '\''
        || c == '"'
        || c == '?'
        || c == '0';
}

static Token* parse_char(Lexer *lexer) {
    advance(lexer);

    char *str;
    char esc = current_char(lexer);
    if (esc == '\'') {
        lexer_err(EMPTY_CHAR_LITERAL, lexer);
        return NULL;
    }

    if (esc == '\\') {
        advance(lexer);

        char c = current_char(lexer);
        if (!is_valid_esc(c)) {
            lexer_err(INVALID_ESCAPE_SEQUENCE, lexer);
            return NULL;
        }

        str = (char *)malloc(3);
        str[0] = '\\';
        str[1] = c;
        str[2] = '\0';
    }
    else {
        str = (char *)malloc(2);
        str[0] = esc;
        str[1] = '\0';
    }

    advance(lexer);
    if (current_char(lexer) != '\'') {
        lexer_err(TOO_MANY_CHARS_IN_CHAR_LITERAL, lexer);
        return NULL;
    }

    Token *token = init_token(str, TOKEN_CHAR_LITERAL, lexer);
    free(str);

    return token;
}

static Token* parse_string(Lexer *lexer) {
    int start = lexer->current;

    advance(lexer);
    while (current_char(lexer)) {
        char c = current_char(lexer);
        if (c == '\\') {
            advance(lexer);
            if (!is_valid_esc(current_char(lexer))) {
                lexer_err(INVALID_ESCAPE_SEQUENCE, lexer);
                return NULL;
            }
        }
        else if (c == '\"') {
            break;
        }

        advance(lexer);
    }

    if (current_char(lexer) == '\0') {
        lexer_err(UNTERMINATED_STRING_LITERAL, lexer);
        return NULL;
    }
    
    int len = lexer->current- start;
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, lexer->source + start + 1, len);
    lexeme[len - 1] = '\0';

    Token *token = init_token(lexeme, TOKEN_STRING_LITERAL, lexer);
    free(lexeme);

    return token;
}

static inline int is_valid_binary_char(char c) {
    return c == '1' || c == '0';
}

static Token* parse_numeric(Lexer *lexer) {
    int start = lexer->current;

    int is_decimal = 0;
    TokenType type = TOKEN_INTEGER_LITERAL;

    while (isdigit(current_char(lexer)) || (match('.', lexer))) {
        if (match('0', lexer)) {
            advance(lexer);

            if (match('x', lexer) || match('X', lexer)) {
                type = TOKEN_HEX_LITERAL;
                advance(lexer);

                while (isxdigit(current_char(lexer))) {
                    advance(lexer);
                }
            }
            else if (match('b', lexer) || match('B', lexer)) {
                type = TOKEN_BINARY_LITERAL;
                advance(lexer);

                while (match('1', lexer) || match('0', lexer)) {
                    advance(lexer);
                }
            }
            else if (isdigit(current_char(lexer))) {
                while (current_char(lexer) >= '0' && current_char(lexer) <= '7') {
                    advance(lexer);
                }
                type = TOKEN_OCTAL_LITERAL;
            }
            else {
                recede(lexer);
            }
        }

        if ((match('.', lexer)) && is_decimal) {
            lexer_err(INVALID_NUMERIC_TOKEN, lexer);
            return NULL;
        }
        else if ((match('.', lexer))) {
            is_decimal = 1;
            type = TOKEN_FLOAT_LITERAL;
        }

        advance(lexer);
    }

    int len = lexer->current - start;
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, lexer->source + start, len);
    lexeme[len] = '\0';

    recede(lexer);

    Token *token = init_token(lexeme, type, lexer);
    free(lexeme);

    return token;
}

static Token* parse_identifier(Lexer *lexer) {
    int start = lexer->current;
    while (isalnum(current_char(lexer)) || current_char(lexer) == '_') {
        advance(lexer);
    }

    int len = lexer->current - start;
    char *lexeme = (char *)malloc(len + 1);
    strncpy(lexeme, lexer->source + start, len);
    lexeme[len] = '\0';

    recede(lexer);

    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (strcmp(lexeme, KEYWORDS[i].symbol) == 0) {
            Token *token = init_token(lexeme, KEYWORDS[i].type, lexer);
            free(lexeme);

            return token;
        }
    }

    Token *token = init_token(lexeme, TOKEN_IDENTIFIER, lexer);
    free(lexeme);

    return token;
}

static Token *parse_token(Lexer *lexer) {
    char c = current_char(lexer);

    if (isalpha(c) || c == '_') {
        return parse_identifier(lexer);
    }
    else if (isdigit(c)) {
        return parse_numeric(lexer);
    }
    else if (c == '\"') {
        return parse_string(lexer);
    }
    else if (c == '\'') {
        return parse_char(lexer);
    }
    else {
        return parse_symbol(lexer);
    }
}

void skip_comments(Lexer *lexer) {
    while (1) {
        if (current_char(lexer) == '/' && lexer->source[lexer->current + 1] == '/') {
            advance(lexer);
            advance(lexer);
            while (current_char(lexer) != '\n' && current_char(lexer) != '\0') {
                advance(lexer);
            }
        } else if (current_char(lexer) == '/' && lexer->source[lexer->current + 1] == '*') {
            advance(lexer);
            advance(lexer);
            while (current_char(lexer) && !(current_char(lexer) == '*' && lexer->source[lexer->current + 1] == '/')) {
                advance(lexer);
            }
            if (current_char(lexer) == '*' && lexer->source[lexer->current + 1] == '/') {
                advance(lexer);
                advance(lexer);
            }
        } else {
            break;
        }

        while (isspace(current_char(lexer))) {
            advance(lexer);
        }
    }
}

void print_lexer(Lexer *lexer) {
  printf("\n\nLEXER SUCCESS\n");
  for (int i = 0; i < lexer->token_count; i++) {
        printf("%d:%d ws:%d | '%s': %s\n", i, lexer->tokens[i].line, lexer->tokens[i].has_whitespace_after, lexer->tokens[i].lexeme, token_type_to_str(lexer->tokens[i].type));
    }
}

void add_token(Token *token, Lexer *lexer) {
    if (lexer->token_count >= lexer->token_capacity) {
        lexer->token_capacity *= 2;
        lexer->tokens = realloc(lexer->tokens, sizeof(Token) * lexer->token_capacity);
    }
    lexer->tokens[lexer->token_count++] = *token;
    free(token);
}

void tokenize(Lexer *lexer) {
    while (lexer->source[lexer->current]) {
        while (isspace(current_char(lexer))) {
            if (current_char(lexer) == '\n') {
                lexer->line++;
            }

            advance(lexer);
        }
        skip_comments(lexer);
        if (is_end(lexer)) break;

        Token *token = parse_token(lexer);
        if (!token) break;
        add_token(token, lexer);

        advance(lexer);
    }

    add_token(init_token("", TOKEN_EOF, lexer), lexer);

    if (lexer->err != NO_LEXER_ERROR) {
        printf("%s",lexer_err_to_str(lexer->err));
        return;
    }

    if (lexer->debug) print_lexer(lexer);
}

void free_lexer(Lexer *lexer) {
    for (int i = 0; i < lexer->token_count; i++) {
        free(lexer->tokens[i].lexeme);
    }

    free(lexer->source);
    free(lexer->tokens);
    free(lexer);
}