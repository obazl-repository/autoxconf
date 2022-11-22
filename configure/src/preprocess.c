#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "preprocess.h"

UT_array *header_specs;
UT_array *headers;

UT_array *found_headers;

char **p = NULL;

bool header_check(char *header_spec,
                  char *compiler,
                  UT_array *compile_args, UT_array *compile_env)
                  /* UT_array **_found_headers) */
{
    fprintf(stdout, "HEADER_check\n");
    /* 1. parse header string: foo.h>bar.h,baz.h */
    /* 2. preprocess null.c, passing -include for headers under test */

    char *token, *header_h;
    char *sep1 = ">";
    char *sep2 = ",";

    utarray_clear(headers);

    token = strtok(header_spec, sep1);
    fprintf(stdout, "TOKEN 1: %s\n", token);
    char *main_hdr = strdup(token);
    token = strtok(NULL, sep2);
    while( token != NULL ) {
        fprintf(stdout, "TOKEN: %s\n", token);
        utarray_push_back(headers, &token);
        token = strtok(NULL, sep2);
    }
    utarray_push_back(headers, &main_hdr);

    /* now env vars */
    char **p = NULL;
    int envc = utarray_len(compile_env) + 1;
    char *env = calloc(envc, sizeof(char*));
    int i = 0;
    p = NULL;
    char **envp = env;
    while ( (p=(char**)utarray_next(compile_env, p))) {
        /* fprintf(stdout, "adding compile env[%d]: %s\n", i, *p); */
        *envp = strdup(*p);
        envp++;
        i++;
    }
    *envp = NULL;
    envp = env;
    /* while (*envp) { */
    /*     fprintf(stdout, "envp: %s\n", *envp); */
    /*     envp++; */
    /* } */
    /* printf("ENVC: %d\n", envc); */

    /* ################ */
    int argc = utarray_len(compile_args) + (2 * utarray_len(headers)) + 1;
    char *argv = calloc(argc, sizeof(char*));
    char **argvp = argv;

    *argvp = basename(compiler);
    argvp++;
    /* i = 1; */
    p = NULL;
    while ( (p=(char**)utarray_next(compile_args, p))) {
        /* printf("adding compile arg: %s\n",*p); */
        *argvp++ = strdup(*p);
        /* printf("stored[%d]: %s\n", i, argv[i]); */
        /* i++; */
    }
    /* now add -include <hdr> */
    p = NULL;
    while ( (p=(char**)utarray_next(headers, p))) {
        /* printf("adding hdr compile arg: %s\n",*p); */
        *argvp++ = "-include";
        *argvp++ = *p;
    }
    *argvp = NULL;
    argvp = argv;
    /* printf("xxxxxxxxxxxxxxxx: %s\n", *argvp); */
    /* while (*argvp) { */
    /*     fprintf(stdout, "argvp: %s\n", *argvp); */
    /*     argvp++; */
    /* } */

    int cmd_rc = run_cmd(compiler, argv, env);

    fprintf(stdout, "RC from run_cmd: %d\n", cmd_rc);

    if (cmd_rc != 0) {
        fprintf(stdout, "FAIL: run_cmd rc: %d\n", cmd_rc);
    } else {
        fprintf(stdout, "cmd result ok\n");
        utarray_concat(found_headers, headers);
    }

    free(argv);
    free(env);
    if (cmd_rc == 0) {
        /* found_headers = headers; */
        /* _found_headers = &headers; */
        return true;
    } else {
        /* _found_headers = NULL; */
        return false;
    }
}

void preprocess(int argc, char *argv[])
{
    utarray_new(compile_args, &ut_str_icd);
    utarray_new(compile_env, &ut_str_icd);
    utarray_new(header_specs, &ut_str_icd);
    utarray_new(headers, &ut_str_icd);
    utarray_new(found_headers, &ut_str_icd);

    int i, j;

    for (i=2; i < argc; i++) {
        printf("pp ARG: %s\n", argv[i]);
        if (strncmp(argv[i], "--xconf_compiler", 17) == 0) {
            i++;
            printf("XCONF COMPILER: %s\n", argv[i]);
            compiler = strdup(argv[i]);
        }
        else if (strncmp(argv[i], "--xconf_args_compile", 20) == 0) {
            printf("XCONF ARGS: COMPILE\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(compile_args, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_env_compile", 19) == 0) {
            printf("XCONF ENV: COMPILE\n");
            for (j=i+1; j < argc; j++) {
                if (strncmp(argv[j], "--xconf", 7) == 0) {
                    i = j - 1; break;
                } else {
                    utarray_push_back(compile_env, &argv[j]);
                }
            }
        }
        else if (strncmp(argv[i], "--xconf_hdr", 11) == 0) {
            printf("XCONF HDR\n");
            i++;
            utarray_push_back(header_specs, &argv[i]);
        }

        else if (strncmp(argv[i], "--xconf_src", 11) == 0) {
            printf("XCONF SRC\n");
            i++;
            utarray_push_back(compile_args, &argv[i]);
        }

        else if (strncmp(argv[i], "--xconf_out", 11) == 0) {
            printf("XCONF OUT\n");
            out_file = strdup(argv[i+1]);
            i++;
        }
    }

    /* ini_configure(); */

    /* for each feature run feature_test
       accum results
       emit config.h  (using template?)
    */

    char **p = NULL;
    p = NULL;
    bool ok;
    while ( (p=(char**)utarray_next(header_specs, p))) {
        fprintf(stdout, "\nHEADER CHECK for %s\n", *p);
        ok = header_check(*p, compiler, compile_args, compile_env);
        /* , &found_headers); */
        if (ok) {
            fprintf(stdout, "HEADER CHECK ok: %s\n", *p);
        } else {
            fprintf(stdout, "HEADER CHECK failed: %s\n", *p);
        }
    }
    while ( (p=(char**)utarray_next(found_headers, p))) {
        fprintf(stdout, "found: %s\n", *p);
    }

    fprintf(stdout, "o writing outfile: %s\n", out_file);
    fprintf(stderr, "e writing outfile\n");

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
    fprintf(ostream, "{\"headers\":\n  [\n");
    p = NULL;
    int ct = utarray_len(found_headers);
    for (int i = 0; i < ct; i++) {
        fprintf(ostream, "    {\"header\": \"%s\"}",
                *(char**)utarray_eltptr(found_headers, i));
        if (ct > i +1)
            fprintf(ostream, ",\n");
        else
            fprintf(ostream, "\n");
    }
    fprintf(ostream, "  ]\n}\n");

    fclose(ostream);

    /* compile(compiler, compile_args, compile_env); */

    free(compiler);
    utarray_free(compile_args);
    utarray_free(compile_env);
    utarray_free(header_specs);
    utarray_free(headers);
}
