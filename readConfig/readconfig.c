#include "readconfig.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define AST_INCLUDE_GLOB 1
#ifdef AST_INCLUDE_GLOB
#if defined(__Darwin__) || defined(__CYGWIN__)
#define GLOB_ABORTED GLOB_ABEND
#endif

#include <glob.h>

#endif

#define warnLog printf

#define MAX_INCLUDE_LEVEL 10
#define MAX_NESTED_COMMENTS 128
#define COMMENT_META ';'
#define COMMENT_TAG '-'

pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;

void ast_copy_string(char *dst, const char *src, size_t size) {
    while (*src && size) {
        *dst++ = *src++;
        size--;
    }
    if (__builtin_expect(!size, 0))
        dst--;
    *dst = '\0';
}

char *ast_skip_blanks(char *str) {
    while (*str && *str < 33)
        str++;
    return str;
}

char *ast_trim_blanks(char *str) {
    char *work = str;
    if (work) {
        work += strlen(work) - 1;
        while ((work >= str) && *work < 33)
            *(work--) = '\0';
    }
    return str;
}

int ast_strlen_zero(const char *s) {
    return (!s || (*s == '\0'));
}

char *ast_skip_nonblanks(char *str) {
    while (*str && *str > 32)
        str++;
    return str;
}

char *ast_strip(char *s) {
    s = ast_skip_blanks(s);
    if (s)
        ast_trim_blanks(s);
    return s;
}

struct ast_variable *ast_variable_new(const char *name, const char *value) {
    struct ast_variable *variable;
    int length = strlen(name) + strlen(value) + 2 + sizeof(struct ast_variable);

    variable = (struct ast_variable *) malloc(length);
    if (variable) {
        memset(variable, 0, length);
        variable->name = variable->stuff;
        variable->value = variable->stuff + strlen(name) + 1;
        strcpy(variable->name, name);
        strcpy(variable->value, value);
    }

    return variable;
}

void ast_variable_append(struct ast_category *category, struct ast_variable *variable) {
    if (category->last)
        category->last->next = variable;
    else
        category->root = variable;
    category->last = variable;
}

void ast_variables_destroy(struct ast_variable *variable) {
    struct ast_variable *variable_temp;

    while (variable) {
        variable_temp = variable;
        variable = variable->next;
        memset(variable_temp, 0, sizeof(struct ast_variable));
        free(variable_temp);
    }
}

struct ast_variable *ast_variable_browse(const struct ast_config *config, const char *category) {
    struct ast_category *pstCategory = NULL;

    if (category && config->last_browse && (config->last_browse->name == category))
        pstCategory = config->last_browse;
    else
        pstCategory = ast_category_get(config, category);

    if (pstCategory)
        return pstCategory->root;
    else
        return NULL;
}

char *ast_variable_retrieve(const struct ast_config *config, const char *category, const char *variable) {
    struct ast_variable *pstVariable;

    if (category) {
        for (pstVariable = ast_variable_browse(config, category); pstVariable; pstVariable = pstVariable->next) {
            if (!strcasecmp(variable, pstVariable->name))
                return pstVariable->value;
        }
    } else {
        struct ast_category *pstCategory;

        for (pstCategory = config->root; pstCategory; pstCategory = pstCategory->next)
            for (pstVariable = pstCategory->root; pstVariable; pstVariable = pstVariable->next)
                if (!strcasecmp(variable, pstVariable->name))
                    return pstVariable->value;
    }

    return NULL;
}

static struct ast_variable *variable_clone(const struct ast_variable *oldVariable) {
    struct ast_variable *newVariable = ast_variable_new(oldVariable->name, oldVariable->value);

    if (newVariable) {
        newVariable->line_no = oldVariable->line_no;
        newVariable->object = oldVariable->object;
        newVariable->blank_lines = oldVariable->blank_lines;
        /* TODO: clone comments? */
    }

    return newVariable;
}

static void move_variables(struct ast_category *oldCategory, struct ast_category *newCategory) {
    struct ast_variable *variable;
    struct ast_variable *variable_next;

    variable_next = oldCategory->root;
    oldCategory->root = NULL;
    for (variable = variable_next; variable; variable = variable_next) {
        variable_next = variable->next;
        variable->next = NULL;
        ast_variable_append(newCategory, variable);
    }
}

struct ast_category *ast_category_new(const char *name) {
    struct ast_category *category;

    category = (struct ast_category *) malloc(sizeof(struct category));
    if (category) {
        memset(category, 0, sizeof(struct ast_category));
        ast_copy_string(category->name, name, sizeof(category->name));
    }

    return category;
}

static struct ast_category *category_get(const struct ast_config *config, const char *category_name, int ignored) {
    struct ast_category *category;

    for (category = config->root; category; category = category->next) {
        if (!strcasecmp(category->name, category_name) && (ignored || !category->ignored))
            return category;
    }

    return NULL;
}

struct ast_category *ast_category_get(const struct ast_config *config, const char *category_name) {
    return category_get(config, category_name, 0);
}

void ast_category_append(struct ast_config *config, struct ast_category *category) {
    if (config->last)
        config->last->next = category;
    else
        config->root = category;
    config->last = category;
    config->current = category;
}

void ast_category_destroy(struct ast_category *category) {
    ast_variables_destroy(category->root);
    free(category);
}

static void inherit_category(struct ast_category *newCategory, const struct ast_category *base) {
    struct ast_variable *variable;

    for (variable = base->root; variable; variable = variable->next) {
        struct ast_variable *variable_temp;

        variable_temp = variable_clone(variable);
        if (variable_temp)
            ast_variable_append(newCategory, variable);
    }
}

struct ast_config *ast_config_new(void) {
    struct ast_config *config;

    config = (struct ast_config *) malloc(sizeof(struct ast_config));
    if (config) {
        memset(config, 0, sizeof(struct ast_config));
        config->max_include_level = MAX_INCLUDE_LEVEL;
    }

    return config;
}

void ast_config_destroy(struct ast_config *config) {
    struct ast_category *category, *category_temp;

    if (!config)
        return;

    category = config->root;
    while (category) {
        ast_variables_destroy(category->root);
        category_temp = category;
        category = category->next;
        free(category_temp);
    }
    free(config);
}

static int process_text_line(struct ast_config *config, struct ast_category **category, char *buf, int line_no,
                             const char *configfile) {
    char *split;
    char *current = buf;
    struct ast_variable *variable;
    int object;

    /* Actually parse the entry */
    if (current[0] == '[') {
        struct ast_category *newCategory = NULL;
        char *category_name;

        /* A category header */
        split = strchr(current, ']');
        if (!split) {
            warnLog("parse error: no closing ']', line %d of %s\n", line_no, configfile);
            return -1;
        }
        *split++ = '\0';
        current++;
        if (*split++ != '(')
            split = NULL;
        category_name = current;
        *category = newCategory = ast_category_new(category_name);
        if (!newCategory) {
            warnLog("Out of memory, line %d of %s\n", line_no, configfile);
            return -1;
        }
        /* If there are options or categories to inherit from, process them now */
        if (split) {
            if (!(current = strchr(split, ')'))) {
                warnLog("parse error: no closing ')', line %d of %s\n", line_no, configfile);
                return -1;
            }
            *current = '\0';
            while ((current = strsep(&split, ","))) {
                if (!strcasecmp(current, "!"))
                    (*category)->ignored = 1;
                else if (!strcasecmp(current, "+")) {
                    *category = category_get(config, category_name, 1);
                    if (!*category) {
                        ast_config_destroy(config);
                        if (NULL != newCategory)
                            ast_category_destroy(newCategory);
                        warnLog("Category addition requested, but category '%s' does not exist, line %d of %s",
                                category_name, line_no, configfile);
                        return -1;
                    }
                } else {
                    struct ast_category *base;
                    base = category_get(config, current, 1);
                    if (!base) {
                        warnLog("Inheritance requested, but category '%s' does not exist, line %d of %s", current,
                                line_no, configfile);
                        return -1;
                    }
                    inherit_category(*category, base);
                }
            }
        }
        if (NULL != newCategory)
            ast_category_append(config, *category);
    } else if (current[0] == '#')
        return 0;
    else {
        // Just a line (variable = value)
        if (!*category) {
            warnLog("parse error: No category context for line %d of %s", line_no, configfile);
            return -1;
        }
        split = strchr(current, '=');
        if (split) {
            *split = 0;
            split++;
            // Ignore > in =>
            if (*split == '>') {
                object = 1;
                split++;
            } else
                object = 0;
            variable = ast_variable_new(ast_strip(current), ast_strip(split));
            if (variable) {
                variable->line_no = line_no;
                variable->object = object;
                // put and reset comments
                variable->blank_lines = 0;
                ast_variable_append(*category, variable);
            } else {
                warnLog("Out of memory, line %d", line_no);
                return -1;
            }
        } else
            warnLog("No '=' (equal sign) in line %d of %s", line_no, configfile);
    }
    return 0;
}

static struct ast_config *config_text_file_load(const char *filename, struct ast_config *config) {
    char fn[256];
    char buffer[8192];
    char *newBuffer, *pComment, *processBuffer;
    FILE *f;
    int line_no = 0;
    int comment = 0, nest[MAX_NESTED_COMMENTS];
    struct ast_category *category = NULL;
    int count = 0;
    struct stat statBuffer;

    category = config->current;
    /*
    if (filename[0] == '/')
        ast_copy_string(fn, filename, sizeof(fn));
    else
        snprintf(fn, sizeof(fn), "/%s/etc/%s", RUN_CONFIG_DIR, filename);
    */
    ast_copy_string(fn, filename, sizeof(fn));

#ifdef AST_INCLUDE_GLOB
    {
        int glob_ret;
        glob_t glob_buffer;
        glob_buffer.gl_offs = 0; // Initialize it to silence gcc
#ifdef SOLARIS
        glob_ret = glob(fn, GLOB_NOCHECK, NULL, &glob_buffer);
#else
        glob_ret = glob(fn, GLOB_NOMAGIC | GLOB_BRACE, NULL, &glob_buffer);
#endif
        if (glob_ret == GLOB_NOSPACE)
            warnLog("Glob Expansion of pattern '%s' failed: Not enough memory", fn);
        else if (glob_ret = GLOB_ABORTED)
            warnLog("Glob Expansion of pattern '%s' failed: Read error", fn);
        else {
            // loop over expanded files
            unsigned int i;
            for (i = 0; i < glob_buffer.gl_pathc; i++) {
                ast_copy_string(fn, glob_buffer.gl_pathv[i], sizeof(fn));
#endif
                do {
                    if (stat(fn, &statBuffer))
                        continue;
                    if (!S_ISREG(statBuffer.st_mode)) {
                        warnLog("'%s' is not a regular file, ignoring", fn);
                        continue;
                    }
                    if (!(f = fopen(fn, "r"))) {
                        warnLog("No file to parse: %s", fn);
                        continue;
                    }
                    count++;
                    warnLog("Parsing file:%s", fn);
                    while (!feof(f)) {
                        line_no++;
                        if (fgets(buffer, sizeof(buffer), f)) {
                            newBuffer = buffer;
                            if (comment)
                                processBuffer = NULL;
                            else
                                processBuffer = buffer;
                            while ((pComment = strchr(newBuffer, COMMENT_META))) {
                                if ((pComment > newBuffer) && (*(pComment - 1) == '\\')) {
                                    // Yuck, gotta memmove
                                    memmove(pComment - 1, pComment, strlen(pComment) + 1);
                                    newBuffer = pComment;
                                } else if (pComment[1] == COMMENT_TAG && pComment[2] == COMMENT_TAG &&
                                           (pComment[3] != '-')) {
                                    // Meta-Comment start detected ";--"
                                    if (comment < MAX_NESTED_COMMENTS) {
                                        *pComment = '\0';
                                        newBuffer = pComment + 3;
                                        comment++;
                                        nest[comment - 1] = line_no;
                                    } else
                                        warnLog("Maximum nest limit of %d reached.", MAX_NESTED_COMMENTS);
                                } else if ((pComment >= newBuffer + 2) &&
                                           (*(pComment - 1) == COMMENT_TAG) &&
                                           (*(pComment - 2) == COMMENT_TAG)) {
                                    // Meta-Comment end detected
                                    comment--;
                                    newBuffer = pComment + 1;
                                    if (!comment) {
                                        // Back to non-comment now
                                        if (processBuffer) {
                                            // Actually have to move what's left over the top, then continue
                                            char *oldPtr;
                                            oldPtr = processBuffer + strlen(processBuffer);
                                            memmove(oldPtr, newBuffer, strlen(newBuffer) + 1);
                                            newBuffer = oldPtr;
                                        } else
                                            processBuffer = newBuffer;
                                    }
                                } else {
                                    if (!comment) {
                                        // If ; is found, and we are not nested in a comment, we immediately stop all comment processing
                                        *pComment = '\0';
                                        newBuffer = pComment;
                                    } else
                                        newBuffer = pComment + 1;
                                }
                            }
                            if (processBuffer) {
                                char *buf = ast_strip(processBuffer);
                                if (!ast_strlen_zero(buf)) {
                                    if (process_text_line(config, &category, buf, line_no, filename)) {
                                        config = NULL;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    fclose(f);
                } while (0);
                if (comment)
                    warnLog("Unterminated comment detected beginning on line %d", nest[comment]);
#ifdef AST_INCLUDE_GLOB
                if (!config)
                    break;
            }
            globfree(&glob_buffer);
        }
    };
#endif
    if (count == 0)
        return NULL;
    return config;
}

struct ast_confog *ast_config_internal_load(const char *filename, struct ast_config *config) {
    struct ast_config *result;
    if (config->include_level == config->max_include_level) {
        warnLog("Maximum Include level (%d) exceeded", config->max_include_level);
        return NULL;
    }

    config->include_level++;
    result = config_text_file_load(filename, config);
    if (result)
        result->include_level--;

    return result;
}

struct ast_config *ast_config_load(const char *filename) {
    struct ast_config *config;
    struct ast_config *result;

    config = ast_config_new();
    if (!config)
        return NULL;
    result = ast_config_internal_load(filename, config);
    if (!result)
        ast_config_destroy(config);

    return result;
}

