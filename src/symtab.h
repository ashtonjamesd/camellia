#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

typedef enum {
    AUTO,
    EXTERNAL,
    STATIC,
    REGISTER,
} StorageClass;

typedef struct {
    char        *identifier;
    StorageClass storage_class;
    int          is_global;
} VariableSymbol;

typedef struct {
    VariableSymbol **symbols;
    int              count;
    int              capacity;
} VariableSymbols;

typedef struct {
    char        *identifier;
    StorageClass storage_class;
    AstDataType  return_type;
} FunctionSymbol;

typedef struct {
    FunctionSymbol **symbols;
    int              count;
    int              capacity;
} FunctionSymbols;

typedef struct {
    char *identifier;
} TypedefSymbol;

typedef struct {
    TypedefSymbol **symbols;
    int              count;
    int              capacity;
} TypedefSymbols;

typedef struct {
    char *identifier;
} LabelSymbol;

typedef struct {
    LabelSymbol **symbols;
    int           count;
    int           capacity;
} LabelSymbols;

extern VariableSymbols *init_variable_symbols();
extern FunctionSymbols *init_function_symbols();
extern TypedefSymbols *init_typedef_symbols();
extern LabelSymbols *init_label_symbols();

extern void free_variable_symbols(VariableSymbols *variable_symbols);
extern void free_function_symbols(FunctionSymbols *function_symbols);
extern void free_typedef_symbols(TypedefSymbols *typedef_symbols);
extern void free_label_symbols(LabelSymbols *label_symbols);

#endif