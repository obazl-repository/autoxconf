#include <errno.h>
#include <fcntl.h>
/* #include <getopt.h> */
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

#include "utarray.h"

#include "spawner.h"

static int debug;
static int verbose;

UT_array *compile_args;
UT_array *link_exe_args;
UT_array *link_dso_args;
UT_array *link_static_args;

void compile(char *compiler, UT_array *compile_args)
{
    char *exe = NULL, *result = NULL;

    int argc = utarray_len(compile_args);

    char *argv[argc + 3];
    char **p = NULL;

    argv[0] = basename(compiler);

    int i = 1;
    while ( (p=(char**)utarray_next(compile_args, p))) {
        /* printf("Compile arg: %s\n",*p); */
        argv[i] = *p;
        /* printf("stored[%d]: %s\n", i, argv[i]); */
        i++;
    }
    argv[argc] = strdup("-c");
    argv[argc + 1] = strdup("/Users/gar/obazl/auto_conf/test/test.c");
    argv[argc + 2] = NULL;

    for (i=0; i < argc + 2; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    // [EFAULT] (14) Path, argv, or envp point to an illegal address.

    result = run_cmd(compiler, &argv);
    if (result == NULL) {
        fprintf(stderr, "FAIL: run_cmd\n");
    } else {
        /* utstring_printf(opam_switch_id, "%s", result); */
/* #if defined(DEBUG_TRACE) */
        fprintf(stdout, "cmd result ok: '%s'\n", result);
            /* utstring_body(opam_switch_id)); */
/* #endif */
    }
}

int main(int argc, char *argv[])
{
    /* int opt; */

    /* char *opts = ":chdtv"; */
    char *compiler = NULL;
    utarray_new(compile_args, &ut_str_icd);
    utarray_new(link_exe_args, &ut_str_icd);
    utarray_new(link_dso_args, &ut_str_icd);
    utarray_new(link_static_args, &ut_str_icd);

    int option_index = 0;
    int ct = 0;
    int i, j;

    for (i=0; i < argc; i++) {
        printf("ARG: %s\n", argv[i]);
        if (strncmp(argv[i], "--xconf_compiler", 17) == 0) {
            printf("XCONF COMPILER: %s\n", argv[i+1]);
            compiler = strdup(argv[i+1]);
            i++;
        }
        else if (strncmp(argv[i], "--xconf_compile_args", 20) == 0) {
            printf("XCONF COMPILE ARGS\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(compile_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_linker", 24) == 0) {
            printf("XCONF LINKER\n");
        }
        else if (strncmp(argv[i], "--xconf_link_exe_args", 21) == 0) {
            printf("XCONF LINK EXE\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_exe_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_link_dso_args", 21) == 0) {
            printf("XCONF LINK DSO\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_dso_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_link_static_args", 24) == 0) {
            printf("XCONF LINK STATIC\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_static_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_env", 11) == 0) {
            printf("XCONF ENV VAR\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_dso_args, &argv[j]);
                }
            }
        }
    }

    printf("FINISHING\n");
    char **p = NULL;

    /* while ( (p=(char**)utarray_next(compile_args, p))) { */
    /*     printf("Compile arg: %s\n",*p); */
    /* } */
    compile(compiler, compile_args);
    free(compiler);
    utarray_free(compile_args);

    /* p = NULL; */
    /* while ( (p=(char**)utarray_next(link_exe_args, p))) { */
    /*     printf("Link exe arg: %s\n",*p); */
    /* } */
    utarray_free(link_exe_args);

    /* p = NULL; */
    /* while ( (p=(char**)utarray_next(link_dso_args, p))) { */
    /*     printf("Link dso arg: %s\n",*p); */
    /* } */
    utarray_free(link_dso_args);

    /* p = NULL; */
    /* while ( (p=(char**)utarray_next(link_static_args, p))) { */
    /*     printf("Link static arg: %s\n",*p); */
    /* } */
    utarray_free(link_static_args);

    exit(EXIT_SUCCESS);
}
