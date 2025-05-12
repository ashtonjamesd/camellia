#ifndef PPD_H
#define PPD_H

typedef struct {
    char *name;
    char *value;
} Macro;

typedef struct {
    Macro **macros;
    char  *source;
    int    count;
    int    capacity;
    int    current;
} PreProcessor;

extern char *preprocess(char *source);
extern void free_preprocessor(PreProcessor *ppd);

#endif