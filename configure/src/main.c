#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#if INTERFACE
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

bool debug;
bool verbose;

UT_array *compile_args;
UT_array *compile_env;
UT_array *link_exe_args;
UT_array *link_exe_env;
UT_array *link_dso_args;
UT_array *link_dso_env;
UT_array *link_static_args;
UT_array *link_static_env;

char *compiler = NULL;
char *linker = NULL;
char *out_file = NULL;
char *ini_file = NULL;

extern struct ini_config_s ini_config;

int main(int argc, char *argv[])
{

    extern char **environ;
    char *e;
    for (char **env = environ; *env != 0; env++) {
        e = *env;
        fprintf(stdout, "Env: %s\n", e);
    }

    /* dispatch on 1st arg: action. */
    if (strncmp(argv[1], "--xconf_action_preprocess", 25) == 0) {
        preprocess(argc, argv);
    }
    else if (strncmp(argv[1], "--xconf_action_compile", 25) == 0) {
    }
    else if (strncmp(argv[1], "--xconf_action_link_exe", 25) == 0) {
    }
    else {
        fprintf(stdout, "ERROR: arg 1 must be --xconf-action-preprocess ...\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
