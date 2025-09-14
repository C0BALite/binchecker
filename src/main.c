/* main.c
 *
 * Copyright 2025 Unknown
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "binchecker-application.h"

#include "libbindiff.h"

#include <libgen.h>

int main (int argc, char *argv[]){
    char *initial_directory = "/home/coba/Projects/BinChecker/TestInput";
    int count = 0;
    int capacity = 0;
    char **file_paths = NULL;
    struct diffChunk* diffs = malloc(10 * sizeof(struct diffChunk));
    char fp1[512], fp2[512]; // Use buffers with sufficient size
    char *fname;
    get_all_file_paths(initial_directory, &file_paths, &count, &capacity);
    printf("Found %d files:\n", count);
    for (int i = 0; i < count; i++) {
        fname = basename(file_paths[i]);
        snprintf(fp1, sizeof(fp1), "/home/coba/Projects/BinChecker/TestInput/%s", fname);
        snprintf(fp2, sizeof(fp2), "/home/coba/Projects/BinChecker/TestOutput/%s", fname);
        diffs = compare_files(fp1, fp2, 4);
        if(diffs != NULL){
        printf("Chunk %d: pos=%d, length=%d\n", i, diffs[0].pos, diffs[0].length);
        printf("File1: %s\n", diffs[0].diffFile1);
        printf("File2: %s\n", diffs[0].diffFile2);
        }
        free(file_paths[i]); // Free individual path strings
    }
    free(file_paths); // Free the array itself

    g_autoptr(BincheckerApplication) app = NULL;
    int ret;
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    app = binchecker_application_new ("org.gnome.BinChecker", G_APPLICATION_DEFAULT_FLAGS);
    ret = g_application_run (G_APPLICATION (app), argc, argv);
    return ret;
}
