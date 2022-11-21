#include <stdbool.h>
#include <unistd.h>

#include "ini.h"  // makeheaders doesn't handle well
#include "auto_conf_ini.h"

extern char *ini_file;

#if INTERFACE
struct ini_config_s {
    UT_array *headers;
    UT_array *types;
    UT_array *functions;
};
#endif

struct ini_config_s ini_config = {
};

extern bool debug;
extern bool verbose;
UT_string *ini_path;

bool ini_error;

/* #define INI_FILE "xconfig.ini" */

int strsort(const void *_a, const void *_b)
{
    const char *a = *(const char* const *)_a;
    const char *b = *(const char* const *)_b;
    return strcmp(a,b);
}

int _ini_handler(void* config, const char* section,
                 const char* key, const char* value)
{
    fprintf(stderr, "err _ini_handler\n");
    fprintf(stdout, "_ini_handler section: '%s'   key: '%s'  value: '%s'\n",
            section, key, value);

#if defined(DEBUG_TRACE)
    if (trace)
        /* log_debug("config_handler section %s: %s=%s", section, key, value); */
#endif

    struct ini_config_s *pconfig = (struct ini_config_s*)config;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(key, n) == 0

    if (MATCH("obazl", "version")) {
        if (verbose)
            /* log_debug("obazl version: %s", value); */
        return 1;
    }

    /* FIXME: normalize filepaths. remove leading ./ and embedded ../ */
    /* disallow leading / and ../ */
    if (MATCH("features", "types")) {
        /* log_debug("section: features; entry: types"); */
        /* log_debug("\t%s", value); */
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
            if (token[0] == '/') {
                /* log_error("Ini file: 'types' values in section 'features' must be relative paths: %s", token); */
                ini_error = true;
                return 0;
            } else {
                /* FIXME: support concats, e.g. foo/bar:foo/baz */
                utarray_push_back(pconfig->types, &token);
                token = strtok(NULL, sep);
            }
        }
        return 1;
    }

    if (MATCH("features", "headers")) {
        fprintf(stdout, "features: headers\n");
        /* log_debug("section: features; entry: headers"); */
        /* log_debug("\t%s", value); */
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
            if (token[0] == '/') {
                /* log_error("Ini file: 'headers' values in section 'features' must be relative paths: %s", token); */
                /* ini_error = true; */
                return 0;
            } else {
                /* log_debug("pushing headers dir: %s", token); */
                utarray_push_back(pconfig->headers, &token);
                token = strtok(NULL, sep);
            }
        }
        return 1;
    }

    if (MATCH("features", "functions")) {
        /* log_debug("section: features; entry: functions"); */
        /* log_debug("\t%s", value); */
        char *token, *sep = " ,\t";
        token = strtok((char*)value, sep);
        while( token != NULL ) {
            if (token[0] == '/') {
                /* log_error("Ini file: 'functions' values in section 'features' must be relative paths: %s", token); */
                ini_error = true;
                return 0;
            } else {
                /* FIXME: support concats, e.g. foo/bar:foo/baz */
                utarray_push_back(pconfig->functions, &token);
                token = strtok(NULL, sep);
            }
        }
        return 1;
    }

    /* if (MATCH("obazl", "repos")) { */
    /*     resolve_repos((char*)value); */
    /* } */

    /* if (MATCH("repos", "coq")) { */
    /*     resolve_coq_repos((char*)value); */
    /* } */

    /* if (MATCH("repos", "ocaml")) { */
    /*     resolve_ocaml_repos((char*)value); */
    /* } */

    /* if ( strncmp(section, "repo:", 5) == 0 ) { */
    /*     /\* printf("REPO section: %s (%s = %s)\n", section, key, value); *\/ */
    /*     char *the_repo = &section[5]; */

    /*     char *repo_dir = get_workspace_dir(the_repo); */
    /*     printf("repo: %s -> %s\n", the_repo, repo_dir); */

    /*     /\* tmp_repo = NULL; *\/ */
    /*     /\* HASH_FIND_STR(repo_map, the_repo, tmp_repo);  /\\* already in the hash? *\\/ *\/ */
    /*     /\* if (tmp_repo) { *\/ */
    /*     /\*     printf("%s -> %s\n", tmp_repo->key, tmp_repo->base_path); *\/ */
    /*     /\* } else { *\/ */
    /*     /\*     fprintf(stderr, "No WS repo found for '%s' listed in .obazlrc\n", the_repo); *\/ */
    /*     /\*     exit(EXIT_FAILURE); *\/ */
    /*     /\* } *\/ */
    /* } */

    return 1;
}

void ini_configure(void)
{
    fprintf(stdout, "ini_configure: %s\n", ini_file);
    int rc;

    utarray_new(ini_config.headers, &ut_str_icd);
    utarray_new(ini_config.types, &ut_str_icd);
    utarray_new(ini_config.functions, &ut_str_icd);

    utstring_new(ini_path);
    utstring_printf(ini_path, "%s",
                    ini_file);

    rc = access(utstring_body(ini_path), R_OK);
    if (rc == 0) {
        fprintf(stdout, "ini file accessible: %s\n", ini_file);
        ini_error = false;
        /* if (verbose) */
            /* log_info("loading inirc config file: %s", */
            /*          utstring_body(ini_path)); */
            /* PARSE INI FILE */
        rc = ini_parse(utstring_body(ini_path), _ini_handler, &ini_config);

        if (rc < 0) {
            //FIXME: deal with missing .config/inirc
            perror("ini_parse");
            /* log_fatal("Can't load/parse ini file: %s", */
            /*           utstring_body(ini_path)); */
            exit(EXIT_FAILURE);
        }
        if (ini_error) {
            /* log_error("Error parsing ini file"); */
            exit(EXIT_FAILURE);
            /* } else { */
            /*     log_debug("Config loaded from %s", utstring_body(ini_path)); */
        }
        /* if (verbose) */
            /* log_info("loaded inirc config file: %s", */
            /*          utstring_body(ini_path)); */
    } else { // rc != 0
        fprintf(stdout, "NOT FOUND: xconfig.ini file %s\n",
                utstring_body(ini_path));
        /* if (verbose) */
            /* log_warn("NOT FOUND: xconfig.ini file %s", */
            /*          utstring_body(ini_path)); */
    }

    utarray_sort(ini_config.types, strsort);
    utarray_sort(ini_config.headers, strsort);

#if defined(DEBUG_INI)
    if (debug)
        dump_ini_config();
#endif

    char **p = NULL;
    p = NULL;
    while ( (p=(char**)utarray_next(ini_config.headers, p))) {
        printf("header: %s\n",*p);
    }
}
