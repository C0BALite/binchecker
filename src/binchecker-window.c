/* binchecker-window.c
 *
 * Copyright 2025 C0BA
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

#include "binchecker-window.h"

struct _BincheckerWindow {
        AdwApplicationWindow parent_instance;

        /* Template widgets */
        GtkButton * dir1_button;
        GtkButton * dir2_button;
        GtkButton * scan_button;
        AdwSpinRow * padding_spin;
        GtkListBox * file_list;
        AdwNavigationPage * results_page;
        AdwNavigationView * navigation_view;
        AdwActionRow * original_ar;
        AdwActionRow * corrupted_ar;
        /* State */
        char * dir1_path;
        char * dir2_path;
        GHashTable * file_diffs;
        GCancellable * cancellable;
};

typedef struct {
        BincheckerWindow * self;
        char * file1_path;
        char * file2_path;
        char * basename;
        int padding;
}
CompareTaskData;

typedef struct {
        char * basename;
        struct diffChunk * diffs;
}
CompareResult;

G_DEFINE_FINAL_TYPE(BincheckerWindow, binchecker_window, ADW_TYPE_APPLICATION_WINDOW)

static void
free_diff_chunk(gpointer data) {
        struct diffChunk * diffs = data;
        if (diffs) {
                int i = 0;
                while (diffs[i].pos != 0 || diffs[i].length != 0) {
                        free(diffs[i].diffFile1);
                        free(diffs[i].diffFile2);
                        i++;
                }
                free(diffs);
        }
}

static void
compare_task_thread_func(GTask * task, gpointer source_object, gpointer task_data, GCancellable * cancellable) {
        CompareTaskData * data = task_data;
        struct diffChunk * diffs;
        CompareResult * result;
        if (g_cancellable_is_cancelled(cancellable)) {
                return;
        }

        diffs = compare_files(data -> file1_path, data -> file2_path, data -> padding);

        if (g_cancellable_is_cancelled(cancellable)) {
                free_diff_chunk(diffs);
                return;
        }
        result = g_new0(CompareResult, 1);
        result -> basename = g_strdup(data -> basename);
        result -> diffs = diffs;

        g_task_return_pointer(task, result, NULL);
}

static void
compare_task_complete(GObject * source_object, GAsyncResult * res, gpointer user_data) {
        BincheckerWindow * self = BINCHECKER_WINDOW(user_data);
        GError * error = NULL;
        CompareResult * result = g_task_propagate_pointer(G_TASK(res), & error);
        GtkWidget * row;
        GtkWidget * label;
        if (error) {
                g_warning("Task failed: %s", error -> message);
                g_error_free(error);
                return;
        }

        if (!result) {
                return;
        }

        if (result -> diffs) {
                g_hash_table_insert(self -> file_diffs, g_strdup(result -> basename), result -> diffs)
                row = gtk_list_box_row_new();
                label = gtk_label_new(result -> basename);
                gtk_widget_set_margin_start(label, 12);
                gtk_widget_set_margin_end(label, 12);
                gtk_widget_set_margin_top(label, 6);
                gtk_widget_set_margin_bottom(label, 6);
                gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
                gtk_list_box_append(self -> file_list, row);
        } else {
                free_diff_chunk(result -> diffs);
        }

        g_free(result -> basename);
        g_free(result);
}

static void
free_task_data(gpointer data) {
        CompareTaskData * task_data = data;
        if (task_data) {
                g_free(task_data -> file1_path);
                g_free(task_data -> file2_path);
                g_free(task_data -> basename);
                g_free(task_data);
        }
}

static void
show_diff_dialog(BincheckerWindow * self,
        const char * filename, struct diffChunk * diffs) {
        BincheckerDiffDialog * dialog = binchecker_diff_dialog_new(GTK_WINDOW(self));
        binchecker_diff_dialog_set_data(dialog, filename, diffs);
        gtk_window_present(GTK_WINDOW(dialog));
}

static void
on_file_row_clicked(GtkListBox * list_box, GtkListBoxRow * row, BincheckerWindow * self) {
        const char * filename;
        struct diffChunk * diffs;
        GtkWidget * child = gtk_list_box_row_get_child(row);
        if (!GTK_IS_LABEL(child)) return;

        filename = gtk_label_get_text(GTK_LABEL(child));
        diffs = g_hash_table_lookup(self -> file_diffs, filename);

        if (diffs) {
                show_diff_dialog(self, filename, diffs);
        }
}

static void
on_dir1_folder_selected(GObject * source, GAsyncResult * result, gpointer user_data) {
        BincheckerWindow * self = user_data;
        GFile * file = gtk_file_dialog_select_folder_finish(GTK_FILE_DIALOG(source), result, NULL);

        if (file) {
                if (self -> dir1_path) g_free(self -> dir1_path);
                self -> dir1_path = g_file_get_path(file);
                adw_action_row_set_subtitle(self -> original_ar, self -> dir1_path);
                g_object_unref(file);
        }
}

static void
on_dir2_folder_selected(GObject * source, GAsyncResult * result, gpointer user_data) {
        BincheckerWindow * self = user_data;
        GFile * file = gtk_file_dialog_select_folder_finish(GTK_FILE_DIALOG(source), result, NULL);

        if (file) {
                if (self -> dir2_path) g_free(self -> dir2_path);
                self -> dir2_path = g_file_get_path(file);
                adw_action_row_set_subtitle(self -> corrupted_ar, self -> dir2_path);
                g_object_unref(file);
        }
}

static void
on_dir1_button_clicked(GtkButton * button, BincheckerWindow * self) {
        GtkFileDialog * dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, _("Select First Directory"));
        gtk_file_dialog_set_modal(dialog, TRUE);

        gtk_file_dialog_select_folder(dialog, GTK_WINDOW(self), NULL, on_dir1_folder_selected, self);
}

static void
on_dir2_button_clicked(GtkButton * button, BincheckerWindow * self) {
        GtkFileDialog * dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, _("Select Second Directory"));
        gtk_file_dialog_set_modal(dialog, TRUE);

        gtk_file_dialog_select_folder(dialog, GTK_WINDOW(self), NULL, on_dir2_folder_selected, self);
}

static void
on_scan_button_clicked(GtkButton * button, BincheckerWindow * self) {
        GtkWidget * child;
        char ** files1 = NULL;
        int count1 = 0;
        int capacity1 = 0;
        double padding_value;
        int padding;
        int i;
        GTask * task;
        if (!self -> dir1_path || !self -> dir2_path) {
                GtkAlertDialog * dialog = gtk_alert_dialog_new(_("Please select both directories first"));
                gtk_alert_dialog_show(dialog, GTK_WINDOW(self));
                g_object_unref(dialog);
                return;
        }
        while ((child = gtk_widget_get_first_child(GTK_WIDGET(self -> file_list)))) {
                gtk_list_box_remove(self -> file_list, child);
        }
        if (self -> file_diffs) {
                g_hash_table_remove_all(self -> file_diffs);
        } else {
                self -> file_diffs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free_diff_chunk);
        }
        get_all_file_paths(self -> dir1_path, & files1, & count1, & capacity1);
        padding_value = adw_spin_row_get_value(self -> padding_spin);
        padding = (int) padding_value;
        if (self -> cancellable) {
                g_cancellable_cancel(self -> cancellable);
        }
        self -> cancellable = g_cancellable_new();
        for (i = 0; i < count1; i++) {
                CompareTaskData * task_data = g_new0(CompareTaskData, 1);
                char * basename = g_path_get_basename(files1[i]);
                char * fp1 = g_build_filename(self -> dir1_path, basename, NULL);
                char * fp2 = g_build_filename(self -> dir2_path, basename, NULL);

                task_data -> self = self;
                task_data -> file1_path = fp1;
                task_data -> file2_path = fp2;
                task_data -> basename = g_strdup(basename);
                task_data -> padding = padding;

                task = g_task_new(NULL, self -> cancellable, compare_task_complete, self);
                g_task_set_task_data(task, task_data, free_task_data);
                g_task_run_in_thread(task, compare_task_thread_func);
                g_object_unref(task);

                g_free(basename);
        }
        for (i = 0; i < count1; i++) {
                free(files1[i]);
        }
        free(files1);
        g_object_unref(self -> cancellable);
        adw_navigation_view_push(self -> navigation_view, self -> results_page);
}

static void
binchecker_window_dispose(GObject * object) {
        BincheckerWindow * self = BINCHECKER_WINDOW(object);

        g_clear_pointer( & self -> dir1_path, g_free);
        g_clear_pointer( & self -> dir2_path, g_free);
        g_clear_pointer( & self -> file_diffs, g_hash_table_destroy);
        g_cancellable_cancel(self -> cancellable);
        g_clear_pointer( & self -> cancellable, g_free);
        G_OBJECT_CLASS(binchecker_window_parent_class) -> dispose(object);
}

static void
binchecker_window_class_init(BincheckerWindowClass * klass) {
        GtkWidgetClass * widget_class = GTK_WIDGET_CLASS(klass);
        GObjectClass * object_class = G_OBJECT_CLASS(klass);
        object_class -> dispose = binchecker_window_dispose;
        gtk_widget_class_set_template_from_resource(widget_class, "/io/github/BinChecker/binchecker-window.ui");
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, dir1_button);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, dir2_button);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, scan_button);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, padding_spin);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, navigation_view);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, results_page);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, file_list);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, original_ar);
        gtk_widget_class_bind_template_child(widget_class, BincheckerWindow, corrupted_ar);
}

static void
binchecker_window_init(BincheckerWindow * self) {
        gtk_widget_init_template(GTK_WIDGET(self));
        g_signal_connect(self -> dir1_button, "clicked", G_CALLBACK(on_dir1_button_clicked), self);
        g_signal_connect(self -> dir2_button, "clicked", G_CALLBACK(on_dir2_button_clicked), self);
        g_signal_connect(self -> scan_button, "clicked", G_CALLBACK(on_scan_button_clicked), self);
        g_signal_connect(self -> file_list, "row-activated", G_CALLBACK(on_file_row_clicked), self);
}

GtkWidget *
        binchecker_window_new(GtkApplication * app) {
                return g_object_new(BINCHECKER_TYPE_WINDOW, "application", app, NULL);
        }
