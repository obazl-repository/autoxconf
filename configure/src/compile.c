#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "feature_tester.h"

bool xheader_check(char *header,
             char *compiler, UT_array *compile_args, UT_array *compile_env)
{
    /* 1. write tmp c file */
    /* 2. try to compile it */
    /* 3. delete tmp file */

    char *result = NULL;

    char **p = NULL;

    int envc = utarray_len(compile_env);
    char *env[envc + 1];
    int i = 0;
    p = NULL;
    while ( (p=(char**)utarray_next(compile_env, p))) {
        /* printf("Compile env var: %s\n",*p); */
        env[i] = *p;
        i++;
    }
    env[envc] = NULL;

    /* ################ */
    int argc = utarray_len(compile_args);

    char *argv[argc + 4];

    argv[0] = basename(compiler);

    i = 1;
    while ( (p=(char**)utarray_next(compile_args, p))) {
        /* printf("Compile arg: %s\n",*p); */
        argv[i] = *p;
        /* printf("stored[%d]: %s\n", i, argv[i]); */
        i++;
    }
    argv[argc + 1] = strdup("-c");
    argv[argc + 2] = strdup("/Users/gar/obazl/auto_conf/test/test.c");
    argv[argc + 3] = NULL;

    /* for (i=0; i < argc + 3; i++) { */
    /*     printf("argv[%d]: %s\n", i, argv[i]); */
    /* } */

    // [EFAULT] (14) Path, argv, or envp point to an illegal address.

    int cmd_rc = run_cmd(compiler, &argv, &env);
    fprintf(stdout, "RC from run_cmd: %d\n", cmd_rc);
    if (cmd_rc != 0) {
        fprintf(stdout, "FAIL: run_cmd rc: %d\n", cmd_rc);
    } else {
        /* utstring_printf(opam_switch_id, "%s", result); */
/* #if defined(DEBUG_TRACE) */
        fprintf(stdout, "cmd result ok\n");
            /* utstring_body(opam_switch_id)); */
/* #endif */
    }
    fprintf(stdout, "o writing outfile: %s\n", out_file);
    fprintf(stderr, "e writing outfile\n");

    errno = 0;
    char *cwd = getcwd(NULL,0);
    if (cwd != NULL) {
        fprintf(stdout, "cwd: %s\n", cwd);
    } else {
        fprintf(stdout, "FAIL: getcwd\n");
    }

    FILE* ostream;
    errno = 0;
    ostream = fopen(out_file, "w");
    if (ostream == NULL) {
        fprintf(stdout, "%s", strerror(errno));
        perror(out_file);
        exit(EXIT_SUCCESS);
    } else {
        fprintf(stdout, "OPENED %s\n", out_file);
        fprintf(stderr, "eOPENED %s\n", out_file);
    }
    fprintf(ostream, "/* generated file - DO NOT EDIT */\n");
    if (cmd_rc == 0)
        fprintf(ostream, "#define HAVE_FOO_H\n");
    else
        fprintf(ostream, "/* #define HAVE_FOO_H */\n");

    fclose(ostream);
}

void compile(int argc, char *argv[])
{
    utarray_new(compile_args, &ut_str_icd);
    utarray_new(compile_env, &ut_str_icd);
    utarray_new(link_exe_args, &ut_str_icd);
    utarray_new(link_exe_env, &ut_str_icd);
    utarray_new(link_dso_args, &ut_str_icd);
    utarray_new(link_dso_env, &ut_str_icd);
    utarray_new(link_static_args, &ut_str_icd);
    utarray_new(link_static_env, &ut_str_icd);

    int i, j;

    for (i=0; i < argc; i++) {
        /* printf("ARG: %s\n", argv[i]); */
        if (strncmp(argv[i], "--xconf_compiler", 17) == 0) {
            /* printf("XCONF COMPILER: %s\n", argv[i+1]); */
            compiler = strdup(argv[i+1]);
            i++;
        }
        else if (strncmp(argv[i], "--xconf_env_compile", 19) == 0) {
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(compile_env, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_compile_args", 20) == 0) {
            /* printf("XCONF COMPILE ARGS\n"); */
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(compile_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_linker", 24) == 0) {
            /* printf("XCONF LINKER\n"); */
            linker = strdup(argv[i+1]);
            i++;
        }
        else if (strncmp(argv[i], "--xconf_args_link_exe", 21) == 0) {
            /* printf("XCONF LINK EXE\n"); */
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_exe_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_env_link_exe", 20) == 0) {
            printf("XCONF ENV LINK EXE\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_exe_env, &argv[j]);
                }
            }
        }

        else if (strncmp(argv[i], "--xconf_link_dso_args", 21) == 0) {
            /* printf("XCONF LINK DSO\n"); */
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_dso_args, &argv[j]);
                }
            }
        }

        else if (strncmp(argv[i], "--xconf_env_link_dso", 20) == 0) {
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_dso_env, &argv[j]);
                }
            }
        }

        else if (strncmp(argv[i], "--xconf_link_static_args", 24) == 0) {
            /* printf("XCONF LINK STATIC\n"); */
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_static_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_env_link_static", 21) == 0) {
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(link_static_env, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_out", 11) == 0) {
            printf("XCONF OUT\n");
            out_file = strdup(argv[i+1]);
            i++;
        }
        else if (strncmp(argv[i], "--xconf_ini", 11) == 0) {
            printf("XCONF INI\n");
            ini_file = strdup(argv[i+1]);
            i++;
        }
    }

    ini_configure();

    /* for each feature run feature_test
       accum results
       emit config.h  (using template?)
    */

    char **p = NULL;
    p = NULL;
    bool ok;
    while ( (p=(char**)utarray_next(ini_config.headers, p))) {
        ok = header_check(*p, compiler, compile_args, compile_env);
        if (ok) {
            /* HAS_?_H */
        }
    }
    /* compile(compiler, compile_args, compile_env); */

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

    fprintf(stderr, "STDERR: bye\n");
    fprintf(stdout, "STDOUT: bye\n");
}
