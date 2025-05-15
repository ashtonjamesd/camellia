#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

#include "camc.h"

int camc_init(int argc, char *argv[]) {
    if (argc < 3) {
        printf("project name required after 'init'.\n");
        return 1;
    }

    mkdir("camc", 0700);

    // mkdir("src", 0700);
    // mkdir("build", 0700);
    
    char *main_path = "camc/main.c";
    // char *main_path = "src/main.c";
    FILE *fptr = fopen(main_path, "w");
    if (!fptr) {
        printf("Error creating '%s'\n", main_path);
        return 1;
    }
    char *buff = "int main() {\n\treturn 0;\n}\0";
    fwrite(buff, 1, strlen(buff), fptr);
    fclose(fptr);

    char *project_path = "camc/camc.yaml";
    FILE *yamlFptr = fopen(project_path, "w");
    if (!yamlFptr) {
        printf("Error creating '%s'\n", project_path);
        return 1;
    }
    char yamlBuff[256];

    char *project = argv[2];
    snprintf(
        yamlBuff, sizeof(yamlBuff), 
        "project:\n  name: \"%s\"\n  version: 0.0.0\n  exe: \"out\"\n", 
        project
    );

    fwrite(yamlBuff, 1, strlen(yamlBuff), yamlFptr);
    fclose(yamlFptr);

    return 0;
}

int camc_build(int argc, char *argv[]) {
    // check camc.yaml exists
    // check /src directory exists
    // run with all .c and .h files
    // produce executable in /build

    return 0;
}

int camc_run(int argc, char *argv[]) {
    // call camc_build
    // run the executable

    return 0;
}

int camc_bump(int argc, char *argv[]) {

    return 0;
}