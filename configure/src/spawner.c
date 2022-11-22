#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
/* #include <fts.h> */
#include <glob.h>
#include <libgen.h>
#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>             /* PATH_MAX */
#endif
#include <pwd.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

/* #include "ini.h" */
/* #include "utarray.h" */
/* #include "utstring.h" */

#include "spawner.h"

int run_cmd(char *executable, char **argv, char **env)
{
    printf("run_cmd\n");
/* #if defined(DEBUG_TRACE) */

    char **argvp = argv;
    UT_string *tmp;
    utstring_new(tmp);
    while (*argvp) {
        /* utstring_printf(tmp, "%s ", *argvp); */
        fprintf(stdout, "Argv: %s\n", *argvp);
        argvp++;
    }
    printf("run cmd: %s\n", utstring_body(tmp));

    char **envp = env;
    utstring_renew(tmp);
    while (*envp) {
        printf("Env: %s\n", *envp);
        utstring_printf(tmp, "%s ", *envp);
        *envp++;
    }
    printf("run env: %s\n", utstring_body(tmp));

    utstring_free(tmp);

/* #endif */

    pid_t pid;
    /* char *argv[] = { */
    /*     "codept", */
    /*     "-args", codept_args_file, */
    /*     NULL}; */
    int rc;

    /* extern char **environ; */

    /* FIXME: write stderr to log instead of dev/null? */
    /* int DEVNULL_FILENO = open("/dev/null", O_WRONLY); */

    int cout_pipe[2];
    int cerr_pipe[2];

    if(pipe(cout_pipe) || pipe(cerr_pipe)) {
        printf("pipe returned an error.");
        exit(EXIT_FAILURE);
    }

    posix_spawn_file_actions_t action;
    posix_spawn_file_actions_init(&action);

    /* child inherits open FDs, so: */
    /* close read end of pipes on child */
    posix_spawn_file_actions_addclose(&action, cout_pipe[0]);
    posix_spawn_file_actions_addclose(&action, cerr_pipe[0]);

    /* dup write-ends on child-side, connect stdout/stderr */
    posix_spawn_file_actions_adddup2(&action, cout_pipe[1],
                                     STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&action, cerr_pipe[1],
                                     STDERR_FILENO);

    /* close write end on child side */
    posix_spawn_file_actions_addclose(&action, cout_pipe[1]);
    posix_spawn_file_actions_addclose(&action, cerr_pipe[1]);

    /* now child will not inherit open pipes (fds?), but its
       stdout/stderr FDs will be connected to the write ends of our
       pipes.
     */
    /* posix_spawn_file_actions_addopen(&action, */
    /*                                  STDOUT_FILENO, */
    /*                                  codept_deps_file, */
    /*                                   O_WRONLY | O_CREAT | O_TRUNC, */
    /*                                   S_IRUSR | S_IWUSR | S_IRGRP ); */

    /* if ((rc = posix_spawn_file_actions_adddup2(&action, */
    /*                                            DEVNULL_FILENO, */
    /*                                            STDERR_FILENO))) { */
    /*     perror("posix_spawn_file_actions_adddup2"); */
    /*     posix_spawn_file_actions_destroy(&action); */
    /*     exit(rc); */
    /* } */

    // FIXME: get absolute path of codept
    // FIXME: restrict environ

    rc = access(executable, R_OK);
    if (rc == 0) {
        fprintf(stdout, "SPAWN EXE FOUND: %s\n", executable);
        fprintf(stdout, "arg[0]: %s\n", argv);
    } else {
        fprintf(stdout, "SPAWN EXE NOT FOUND\n");
    }

    char *rp = realpath(executable, NULL);
    fprintf(stdout, "SPAWN EXE realpath: %s\n", rp);

    fprintf(stdout, "spawning %s\n", rp);
    rc = posix_spawnp(&pid, rp, &action, NULL, argv, env);

    if (rc != 0) {
        /* does not set errno */
        fprintf(stderr, "run_command posix_spawn error rc: %d, %s |",
                rc, strerror(rc));
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    /* now close the write end on parent side */
    close(cout_pipe[1]);
    close(cerr_pipe[1]);

    /* https://github.com/pixley/InvestigativeProgramming/blob/114b698339fb0243f50cf5bfbe5d5a701733a125/test_spawn_pipe.cpp */

    /* printf("Read from pipes\n"); */
    static char buffer[1024] = "";
    struct timespec timeout = {5, 0};

    fd_set read_set;
    memset(&read_set, 0, sizeof(read_set));
    FD_SET(cout_pipe[0], &read_set);
    FD_SET(cerr_pipe[0], &read_set);

    int larger_fd = (cout_pipe[0] > cerr_pipe[0])
        ? cout_pipe[0]
        : cerr_pipe[0];

    rc = pselect(larger_fd + 1, &read_set, NULL, NULL, &timeout, NULL);
    //thread blocks until either packet is received or the timeout goes through
    if (rc == 0) {
        fprintf(stderr, "pselect timed out.\n");
        /* return 1; */
        exit(EXIT_FAILURE);
    }

    int bytes_read = read(cerr_pipe[0], &buffer[0], 1024);
    if (bytes_read > 0) {
        fprintf(stdout, "Readed stderr message: %s", buffer);
    }

    bytes_read = read(cout_pipe[0], &buffer[0], 1024);
    /* printf("outpipe bytes_read: %d\n", bytes_read); */
    if (bytes_read > 0){
        /* drop trailing newline */
        if (buffer[bytes_read - 1] == '\n')
            buffer[bytes_read - 1] = '\0';
    }

    pid_t waitrc = waitpid(pid, &rc, 0);
    if (waitrc == -1) {
        fprintf(stdout, "spawn_cmd waitpid error");
        /* log_error("spawn_cmd"); */
        posix_spawn_file_actions_destroy(&action);
        return -1;
    } else {
#if defined(DEBUG_TRACE)
        /* log_trace("waitpid rc: %d", waitrc); */
#endif
        /* if (waitrc == 0) { */
        // child exit OK
        if ( WIFEXITED(rc) ) {
            // terminated normally by a call to _exit(2) or exit(3).
            fprintf(stdout, "WIFEXITED\n");
            fprintf(stdout, "WEXITSTATUS: %d\n", WEXITSTATUS(rc));
#if defined(DEBUG_TRACE)
            /* log_trace("WIFEXITED(rc)"); */
            /* log_trace("WEXITSTATUS(rc): %d", WEXITSTATUS(rc)); */
#endif
            /* log_debug("WEXITSTATUS: %d", WEXITSTATUS(rc)); */
            /* "widow" the pipe (delivers EOF to reader)  */
            /* close(stdout_pipe[1]); */
            /* dump_pipe(STDOUT_FILENO, stdout_pipe[0]); */
            /* close(stdout_pipe[0]); */

            /* "widow" the pipe (delivers EOF to reader)  */
            /* close(stderr_pipe[1]); */
            /* dump_pipe(STDERR_FILENO, stderr_pipe[0]); */
            /* if opam repos need update... */
            /* close(stderr_pipe[0]); */
            posix_spawn_file_actions_destroy(&action);
            return 0;
        }
        else if (WIFSIGNALED(rc)) {
            // terminated due to receipt of a signal
            fprintf(stdout, "WIFSIGNALED\n");
            fprintf(stdout, "WTERMSIG: %d\n",
                    WTERMSIG(rc));
            /* log_error("WIFSIGNALED(rc)"); */
            /* log_error("WTERMSIG: %d", WTERMSIG(rc)); */

            // a failed compile is ok, means "unsupported feature"
            // but we need to catch other possible errors
            posix_spawn_file_actions_destroy(&action);
            return rc;
        } else if (WIFSTOPPED(rc)) {
            /* process has not terminated, but has stopped and can
               be restarted. This macro can be true only if the
               wait call specified the WUNTRACED option or if the
               child process is being traced (see ptrace(2)). */
            /* log_error("WIFSTOPPED(rc)"); */
            /* log_error("WSTOPSIG: %d", WSTOPSIG(rc)); */
            posix_spawn_file_actions_destroy(&action);
            return rc;
        }
    }

    fprintf(stdout,  "run_command rc: %d\n", rc);
    posix_spawn_file_actions_destroy(&action);
    return rc;
}

/* char principal[256]; */

/* int copyfile(char *fromfile, char *tofile) { */
/*     char ch;// source_file[20], target_file[20]; */

/*     FILE *source = fopen(fromfile, "r"); */
/*     if (source == NULL) { */
/*         fprintf(stderr, "copyfile fopen fail on fromfile: %s\n", fromfile); */
/*         exit(EXIT_FAILURE); */
/*     } */
/*     FILE *target = fopen(tofile, "w"); */
/*     if (target == NULL) { */
/*         fclose(source); */
/*         fprintf(stderr, "copyfile fopen fail on tofile: %s\n", tofile); */
/*         exit(EXIT_FAILURE); */
/*     } */
/*     while ((ch = fgetc(source)) != EOF) */
/*         fputc(ch, target); */
/* /\* #if defined(DEBUG_TRACE) *\/ */
/* /\*         printf("File copy successful: %s -> %s.\n", *\/ */
/* /\*                fromfile, tofile); *\/ */
/* /\* #endif *\/ */
/*     fclose(source); */
/*     fclose(target); */
/*     return 0; */
/* } */

