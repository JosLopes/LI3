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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "testing/test_diff.h"

/**
 * @brief Differences between generated and expected program output.
 *
 * @var test_diff::extra_files
 *     @brief Files present in the program's results but not in the expected results (strings).
 * @var test_diff::missing_files
 *     @brief Files present in the program's expected results but not in its actual results
 *            (strings).
 */
struct test_diff {
    GPtrArray *extra_files, *missing_files;
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
 * @brief  Creates a sorted array of all regular files (not directories) in a directory.
 * @param  path Path to the directory to be listed.
 * @return A sorted array of `char *`.
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

test_diff_t *test_diff_create(const char *results, const char *expected) {
    test_diff_t *diff = malloc(sizeof(test_diff_t));
    if (!diff)
        return NULL;

    diff->extra_files   = g_ptr_array_new_with_free_func((GDestroyNotify) free);
    diff->missing_files = g_ptr_array_new_with_free_func((GDestroyNotify) free);

    GPtrArray *common = g_ptr_array_new_with_free_func((GDestroyNotify) free);

    if (__test_diff_common_files(results,
                                 expected,
                                 common,
                                 diff->extra_files,
                                 diff->missing_files)) {
        test_diff_free(diff);
        g_ptr_array_unref(common);
        return NULL;
    }

    g_ptr_array_unref(common);
    return diff;
}

const char *const *test_diff_get_extra_files(const test_diff_t *diff, size_t *n) {
    *n = diff->extra_files->len;
    return (const char **) diff->extra_files->pdata;
}

const char *const *test_diff_get_missing_files(const test_diff_t *diff, size_t *n) {
    *n = diff->missing_files->len;
    return (const char **) diff->missing_files->pdata;
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
    clone->missing_files = g_ptr_array_copy(diff->missing_files, g_strdup_data, NULL);
    return clone;
}

void test_diff_free(test_diff_t *diff) {
    g_ptr_array_unref(diff->extra_files);
    g_ptr_array_unref(diff->missing_files);
    free(diff);
}
