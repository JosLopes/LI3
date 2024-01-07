/*
 * Copyright 2023 Humberto Gomes, José Lopes, José Matos
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file  test_diff.c
 * @brief Implementation of methods in include/testing/test_diff.h
 *
 * #### Examples
 * See [the header file's documentation](@ref test_diff_example).
 */

#include <dirent.h>
#include <glib.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "testing/test_diff.h"
#include "utils/int_utils.h"

/**
 * @brief Differences between generated and expected program output.
 *
 * @var test_diff::extra_files
 *     @brief Files present in the program's results but not in the expected results (strings).
 * @var test_diff::common_files
 *     @brief Files present in the both the program's results and the expected results (strings).
 * @var test_diff::missing_files
 *     @brief Files present in the program's expected results but not in its actual results
 *            (strings).
 * @var test_diff::common_file_errors
 *     @brief An array with an integer for every file in ::test_diff::common_files. Each integer
 *            will be `0` if the files (expected and generated) are the same, the line where they
 *            first differ if that's not the case, or `-1` on IO error.
 */
struct test_diff {
    GPtrArray *extra_files, *common_files, *missing_files;
    ssize_t   *common_file_errors;
};

/**
 * @brief Comparsion function for sorting strings in the array returned by ::__test_diff_read_dir.
 */
gint __test_diff_read_dir_sort_compare(gconstpointer a, gconstpointer b) {
    const char *ap = *(const char *const *) a;
    const char *bp = *(const char *const *) b;
    return strcmp(ap, bp);
}

/**
 * @brief  Creates a sorted array of all regular files (excluding directories) in a directory.
 * @param  path Path to the directory to be listed.
 * @return A sorted array of `char *`, that should be deleted with `g_ptr_array_unref`.
 */
GPtrArray *__test_diff_read_dir(const char *path) {
    GPtrArray *ret = g_ptr_array_new_with_free_func((GDestroyNotify) free);

    DIR *dir = opendir(path);
    if (!dir) {
        g_ptr_array_unref(ret);
        return NULL;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        char full_path[PATH_MAX];
        snprintf(full_path, PATH_MAX, "%s/%s", path, ent->d_name);

        struct stat statbuf; /* Only show directories */
        if (!stat(full_path, &statbuf) && S_ISREG(statbuf.st_mode)) {
            g_ptr_array_add(ret, strdup(ent->d_name));
        }
    }
    closedir(dir);

    g_ptr_array_sort(ret, __test_diff_read_dir_sort_compare);
    return ret;
}

/**
 * @brief Fills three arrays with information about files common between two directories.
 *
 * @param results  Directory where the program's output was placed into.
 * @param expected Directory containing expected program results.
 * @param common   Files present in both @p results and @p expected.
 * @param extra    Files in @p results but not it @p expected.
 * @param missing  Files in @p expected but not in @p results.
 *
 * @retval 0 Success.
 * @retval 1 IO error.
 */
int __test_diff_common_files(const char *results,
                             const char *expected,
                             GPtrArray  *common,
                             GPtrArray  *extra,
                             GPtrArray  *missing) {

    GPtrArray *results_files = __test_diff_read_dir(results);
    if (!results_files)
        return 1;
    GPtrArray *expected_files = __test_diff_read_dir(expected);
    if (!expected_files) {
        g_ptr_array_unref(results_files);
        return 1;
    }

    /* Algorithm similar to a merge, for set partitioning */
    guint ri = 0, ei = 0;
    while (ri < results_files->len || ei < expected_files->len) {
        if (ri == results_files->len) {
            char *e = g_ptr_array_index(expected_files, ei);
            g_ptr_array_add(missing, strdup(e));
            ei++;
        } else if (ei == expected_files->len) {
            char *r = g_ptr_array_index(results_files, ri);
            g_ptr_array_add(extra, strdup(r));
            ri++;
        } else {
            char *r   = g_ptr_array_index(results_files, ri);
            char *e   = g_ptr_array_index(expected_files, ei);
            int   cmp = strcmp(r, e);

            if (cmp < 0) { /* r < e */
                g_ptr_array_add(extra, strdup(r));
                ri++;
            } else if (cmp == 0) { /* r == e */
                g_ptr_array_add(common, strdup(r));
                ri++;
                ei++;
            } else { /* e < r */
                g_ptr_array_add(missing, strdup(e));
                ei++;
            }
        }
    }

    g_ptr_array_unref(results_files);
    g_ptr_array_unref(expected_files);
    return 0;
}

/**
 * @brief Loads the contents of a file to memory.
 *
 * @param path Path to the file.
 * @param n    Where to write the number of bytes read to.
 *
 * @return On success, the array of read bytes, which must be deleted with `free`. On failure,
 *         `NULL` will be returned.
 */
char *__test_diff_read_file(const char *path, size_t *n) {
    FILE *f = fopen(path, "r");
    if (!f)
        return NULL;

    ssize_t len;
    char   *ret = NULL;

    /* Determine lenght of file */
    if (fseek(f, 0, SEEK_END))
        goto END;
    if ((len = ftell(f)) == -1)
        goto END;
    if (fseek(f, 0, 0))
        goto END;

    /* Read file contents */
    ret          = malloc(sizeof(char) * len);
    ssize_t read = fread(ret, 1, len, f);
    if (read != len || ferror(f)) {
        ret = NULL;
        free(ret);
        goto END;
    }

    *n = (size_t) len;
END:
    fclose(f);
    return ret;
}

/**
 * @brief Compares two values to determine if they differ in any line.
 *
 * @param result   Path to file generated by the program.
 * @param expected Path to file that the program is expected to output.
 *
 * @return `0` if the files are the same, `-1` if an IO error occurs while reading from either file,
 *         the line where the files differ otherwise.
 */
ssize_t __test_diff_compare_files(const char *result, const char *expected) {
    size_t result_len, expected_len;
    char  *result_contents = __test_diff_read_file(result, &result_len);
    if (!result_contents)
        return -1;
    char *expected_contents = __test_diff_read_file(expected, &expected_len);
    if (!expected_contents) {
        free(result_contents);
        return -1;
    }

    size_t min_len = min(result_len, expected_len);
    size_t line = 1, ret = 0;
    for (size_t i = 0; i < min_len; ++i) {
        if (result_contents[i] == '\n' && expected_contents[i] == '\n')
            line++;

        if (result_contents[i] != expected_contents[i]) {
            ret = line;
            break;
        }
    }

    if (ret == 0 && result_len != expected_len)
        ret = line;

    free(result_contents);
    free(expected_contents);
    return ret;
}

test_diff_t *test_diff_create(const char *results, const char *expected) {
    test_diff_t *diff = malloc(sizeof(test_diff_t));
    if (!diff)
        return NULL;

    diff->extra_files   = g_ptr_array_new_with_free_func((GDestroyNotify) free);
    diff->missing_files = g_ptr_array_new_with_free_func((GDestroyNotify) free);
    diff->common_files  = g_ptr_array_new_with_free_func((GDestroyNotify) free);

    if (__test_diff_common_files(results,
                                 expected,
                                 diff->common_files,
                                 diff->extra_files,
                                 diff->missing_files)) {
        test_diff_free(diff);
        return NULL;
    }

    diff->common_file_errors = malloc(sizeof(ssize_t) * diff->common_files->len);
    for (guint i = 0; i < diff->common_files->len; ++i) {
        const char *file_name = g_ptr_array_index(diff->common_files, i);

        char result_path[PATH_MAX], expected_path[PATH_MAX];
        snprintf(result_path, PATH_MAX, "%s/%s", results, file_name);
        snprintf(expected_path, PATH_MAX, "%s/%s", expected, file_name);

        diff->common_file_errors[i] = __test_diff_compare_files(result_path, expected_path);
    }

    return diff;
}

const char *const *test_diff_get_extra_files(const test_diff_t *diff, size_t *n) {
    *n = diff->extra_files->len;
    return (const char *const *) diff->extra_files->pdata;
}

const char *const *test_diff_get_missing_files(const test_diff_t *diff, size_t *n) {
    *n = diff->missing_files->len;
    return (const char *const *) diff->missing_files->pdata;
}

size_t test_diff_get_common_file_errors(const test_diff_t  *diff,
                                        const char *const **common_files,
                                        ssize_t const     **errors) {

    *common_files = (const char *const *) diff->common_files->pdata;
    *errors       = diff->common_file_errors;
    return diff->common_files->len;
}

/** @brief A `GCopyFunc` to duplicate a string in a `GPtrArray`. */
gpointer g_strdup_data(gconstpointer src, gpointer data) {
    (void) data;
    return strdup(src);
}

test_diff_t *test_diff_clone(const test_diff_t *diff) {
    test_diff_t *clone = malloc(sizeof(test_diff_t));
    if (!clone)
        return NULL;

    clone->extra_files   = g_ptr_array_copy(diff->extra_files, g_strdup_data, NULL);
    clone->common_files  = g_ptr_array_copy(diff->common_files, g_strdup_data, NULL);
    clone->missing_files = g_ptr_array_copy(diff->missing_files, g_strdup_data, NULL);

    size_t common_file_errors_size = sizeof(ssize_t) * diff->common_files->len;
    clone->common_file_errors      = malloc(common_file_errors_size);
    memcpy(clone->common_file_errors, diff->common_file_errors, common_file_errors_size);

    return clone;
}

void test_diff_free(test_diff_t *diff) {
    g_ptr_array_unref(diff->extra_files);
    g_ptr_array_unref(diff->common_files);
    g_ptr_array_unref(diff->missing_files);
    free(diff->common_file_errors);
    free(diff);
}
