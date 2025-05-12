#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "ppd.h"

PreProcessor *init_preprocessor(char *source) {
    PreProcessor *ppd = malloc(sizeof(PreProcessor));
    ppd->macros = malloc(sizeof(Macro));
    ppd->source = strdup(source);
    ppd->count = 0;
    ppd->capacity = 1;
    ppd->current = 0;

    return ppd;
}

void free_macro(Macro *macro) {
    free(macro->name);
    free(macro->value);
    free(macro);
}

void free_preprocessor(PreProcessor *ppd) {
    for (int i = 0; i < ppd->count; i++ ) {
        free_macro(ppd->macros[i]);
    }
    free(ppd->macros);
    free(ppd);
}

void advance(PreProcessor *ppd) {
    ++ppd->current;
}

int is_end(PreProcessor *ppd) {
    return ppd->current >= strlen(ppd->source);
}

char current(PreProcessor *ppd) {
    if (is_end(ppd)) return '\0';
    return ppd->source[ppd->current];
}

void add_macro(PreProcessor *ppd, char *name, char *value) {
    if (ppd->count >= ppd->capacity) {
        ppd->capacity *= 2;
        ppd->macros = realloc(ppd->macros, sizeof(Macro) * ppd->capacity);
    }

    Macro *macro = malloc(sizeof(Macro));
    macro->name = strdup(name);
    macro->value = strdup(value);

    ppd->macros[ppd->count++] = macro;
}

char *replace_text(PreProcessor *ppd) {
    char *output = malloc(strlen(ppd->source) * 2);
    output[0] = '\0';

    ppd->current = 0;

    while (!is_end(ppd)) {
        if (isalpha(current(ppd)) || current(ppd) == '_') {
            int start = ppd->current;
            while (isalnum(current(ppd)) || current(ppd) == '_') advance(ppd);
            int len = ppd->current - start;
            char *lexeme = strndup(&ppd->source[start], len);

            int replaced = 0;
            for (int i = 0; i < ppd->count; i++) {
                if (strcmp(ppd->macros[i]->name, lexeme) == 0) {
                    strcat(output, ppd->macros[i]->value);
                    replaced = 1;
                    break;
                }
            }
            if (!replaced) {
                strcat(output, lexeme);
            }

            free(lexeme);
        } else {
            char c[2] = { current(ppd), '\0' };
            strcat(output, c);
            advance(ppd);
        }
    }

    return output;
}

char *try_parse_macro_name(PreProcessor *ppd) {
    int name_start = ppd->current;
    while (!isspace(current(ppd))) {
        advance(ppd);
    }
    char *name = strndup(&ppd->source[name_start], ppd->current - name_start);

    while (ppd->source[ppd->current] == ' ' || ppd->source[ppd->current] == '\t') {
        ppd->current++;
    }

    return name;
}

char *try_parse_macro_value(PreProcessor *ppd) {
    int value_start = ppd->current;
    while (current(ppd) != '\n' && current(ppd) != '\0') {
        ppd->current++; 
    }
    int value_len = ppd->current - value_start;
    char *value = strndup(&ppd->source[value_start], ppd->current - value_start);

    return value;
}

void process_define(PreProcessor *ppd) {
    while (current(ppd) == ' ' || current(ppd) == '\t') {
        advance(ppd);
    }

    char *name = try_parse_macro_name(ppd);
    char *value = try_parse_macro_value(ppd);

    add_macro(ppd, name, value);

    free(name);
    free(value);
}

void process_include(PreProcessor *ppd) {
    while (current(ppd) == ' ' || current(ppd) == '\t') {
        advance(ppd);
    }

    if (current(ppd) == '\"') {
        advance(ppd);
        int name_start = ppd->current;
        while (!is_end(ppd) && current(ppd) != '\"') {
            advance(ppd);
        }
        char *name = strndup(&ppd->source[name_start], ppd->current - name_start);
        
        FILE *fptr = fopen(name, "r");
        if (!fptr) {
            printf("File not found");
            return;
        }
        fseek(fptr, 0, SEEK_END);
        long sz = ftell(fptr);
        fseek(fptr, 0, SEEK_SET);

        char *content = malloc(sz + 1);
        fread(content, sz, 1, fptr);
        fclose(fptr);
        content[sz] = '\0';

        int include_start = ppd->current;
        while (include_start > 0 && ppd->source[include_start - 1] != '#') {
            include_start--;
        }
        include_start--; // maybe

        int include_end = ppd->current;
        while (ppd->source[include_end] != '\n' && !is_end(ppd) && ppd->source[include_end] != '\0') {
            include_end++;
        }

        int new_len = (strlen(ppd->source) - (include_end - include_start)) + strlen(content) + 1;
        
        char *new_source = malloc(new_len);
        if (!new_source) {
            printf("Error allocating space for include.\n");
            return;
        }

        strncpy(new_source, ppd->source, include_start);
        new_source[include_start] = '\0';
        
        // this line appends only adds chars after 16idx in COntent if the first char is not a new line 
        strcat(new_source, "\n");
        strcat(new_source, content);

        strcat(new_source, &ppd->source[include_end]);
        ppd->source = new_source;

        free(content);
        ppd->current = include_start;
    }
    else if (current(ppd) == '<') {
        // handle global header files
    }
}

void process_undef(PreProcessor *ppd) {
    while (current(ppd) == ' ' || current(ppd) == '\t') {
        advance(ppd);
    }

    char *name = try_parse_macro_name(ppd);
    for (int i = 0; i < ppd->count; i++) {
        if (strcmp(name, ppd->macros[i]->name) == 0) {
            free_macro(ppd->macros[i]);

            for (int j = i; j < ppd->count - 1; j++) {
                ppd->macros[j] = ppd->macros[j + 1];
            }
            ppd->count--;
            i--;
        }
    }
}

void remove_directives(PreProcessor *ppd) {
    int read = 0;
    int write = 0;

    while (ppd->source[read] != '\0') {
        if (ppd->source[read] == '#') {
            while (ppd->source[read] != '\n' && ppd->source[read] != '\0') {
                read++;
            }
        } else {
            ppd->source[write++] = ppd->source[read++];
        }
    }

    ppd->source[write] = '\0';
    ppd->current = 0;
}

void parse_ppd(PreProcessor *ppd) {
    ppd->current++;

    int start = 0;
    while (!is_end(ppd)) {
        char keyword_buff[10];

        // there are no preprocessor directives with more than 7 characters, therefore,
        // longer words are simply ignored and treated as invalid
        if (start == 10) break; 

        if (ppd->source[ppd->current] == ' ') { 
            keyword_buff[start] = '\0';

            if (strcmp("define", keyword_buff) == 0) {
                process_define(ppd);
            }
            else if (strcmp("undef", keyword_buff) == 0) {
                process_undef(ppd);
            }
            else if (strcmp("include", keyword_buff) == 0) {
                process_include(ppd);
            }
        }
        else {
            keyword_buff[start++] = ppd->source[ppd->current];
        }

        ppd->current++;
    }
}

char *preprocess(char *source) {
    PreProcessor *ppd = init_preprocessor(source);

    while (!is_end(ppd)) {
        if (current(ppd) == '#') {
            parse_ppd(ppd);

            // i don't really know why but this makes it work
            ppd->current -= 5;
        }

        ppd->current++;
    }

    remove_directives(ppd);

    char *processed = replace_text(ppd);
    free_preprocessor(ppd);

    return processed;
}