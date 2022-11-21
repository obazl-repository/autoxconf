#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "feature_tester.h"

bool _check(char *header,
             char *compiler, UT_array *compile_args, UT_array *compile_env)
{
    /* 1. write tmp c file */
    /* 2. try to compile it */
    /* 3. delete tmp file */

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

    int cmd_rc = run_cmd(compiler, argv, env);
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

    if (cmd_rc == 0)
        return true;
    else
        return false;
}

