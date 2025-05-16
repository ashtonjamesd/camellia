#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>

VariableSymbols *init_variable_symbols() {
    return NULL;
}

FunctionSymbols *init_function_symbols() {
    return NULL;

}

TypedefSymbols *init_typedef_symbols() {
    return NULL;
}

LabelSymbols *init_label_symbols() {
    return NULL;
}

void free_variable_symbols(VariableSymbols *variable_symbols) {
    free(variable_symbols);
}

void free_function_symbols(FunctionSymbols *function_symbols) {
    free(function_symbols);
}

void free_typedef_symbols(TypedefSymbols *typedef_symbols) {
    free(typedef_symbols);
}

void free_label_symbols(LabelSymbols *label_symbols) {
    free(label_symbols);
}
