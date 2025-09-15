/* binchecker-window.c
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

#include "binchecker-window.h"

#include "libbindiff.h"

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
};

G_DEFINE_FINAL_TYPE(BincheckerWindow, binchecker_window, ADW_TYPE_APPLICATION_WINDOW)

static void
show_diff_dialog(BincheckerWindow * self,
        const char * filename, struct diffChunk * diffs) {
        GtkWidget * dialog;
        GtkWidget * scrolled_window;
        GtkWidget * box;
        char header[256];
        GtkWidget * header_label;
        GtkWidget * separator;
        int diff_count;
        int i;
        GtkWidget * content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        GtkWidget * button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        GtkWidget * close_button = gtk_button_new_with_label("Close");

        dialog = adw_window_new();
        gtk_window_set_title(GTK_WINDOW(dialog), "File Differences");
        gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(self));
        gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 500);

        // Create main content box
        gtk_widget_set_hexpand(content_box, TRUE);
        gtk_widget_set_vexpand(content_box, TRUE);
        gtk_widget_set_margin_top(content_box, 12);
        gtk_widget_set_margin_bottom(content_box, 12);
        gtk_widget_set_margin_start(content_box, 12);
        gtk_widget_set_margin_end(content_box, 12);

        // Add filename header
        snprintf(header, sizeof(header), "Differences in: %s", filename);
        header_label = gtk_label_new(header);
        gtk_widget_add_css_class(header_label, "title-3");
        gtk_widget_set_halign(header_label, GTK_ALIGN_START);
        gtk_box_append(GTK_BOX(content_box), header_label);

        // Add separator
        separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_box_append(GTK_BOX(content_box), separator);

        // Create scrolled window for diff content
        scrolled_window = gtk_scrolled_window_new();
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                GTK_POLICY_AUTOMATIC,
                GTK_POLICY_AUTOMATIC);
        gtk_widget_set_hexpand(scrolled_window, TRUE);
        gtk_widget_set_vexpand(scrolled_window, TRUE);

        // Create box for diff items
        box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
        gtk_widget_set_hexpand(box, TRUE);

        diff_count = 0;
        if (diffs) {
                while (diff_count < 10 && (diffs[diff_count].pos != 0 || diffs[diff_count].length != 0 ||
                                diffs[diff_count].diffFile1 != NULL || diffs[diff_count].diffFile2 != NULL)) {
                        diff_count++;
                }
        }

        for (i = 0; i < diff_count; i++) {
                char diff_text[1024];
                GtkWidget * diff_label;
                GtkWidget * frame;
                GtkWidget * frame_content;

                const char * file1_hex = diffs[i].diffFile1 ? (const char * ) diffs[i].diffFile1 : "NULL";
                const char * file2_hex = diffs[i].diffFile2 ? (const char * ) diffs[i].diffFile2 : "NULL";
                printf("%s",diffs[i].diffFile1);
                snprintf(diff_text, sizeof(diff_text),
                        "Difference %d:\nPosition: %d\nLength: %d bytes\n\nFile 1: %s\nFile 2: %s",
                        i + 1,
                        diffs[i].pos,
                        diffs[i].length,
                        file1_hex,
                        file2_hex);
                frame_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
                gtk_widget_set_hexpand(frame_content, TRUE);

                diff_label = gtk_label_new(diff_text);
                gtk_label_set_selectable(GTK_LABEL(diff_label), TRUE);
                gtk_label_set_wrap(GTK_LABEL(diff_label), TRUE);
                gtk_label_set_xalign(GTK_LABEL(diff_label), 0.0);
                gtk_label_set_yalign(GTK_LABEL(diff_label), 0.0);
                gtk_widget_set_hexpand(diff_label, TRUE);
                gtk_widget_set_halign(diff_label, GTK_ALIGN_FILL);
                gtk_label_set_markup(GTK_LABEL(diff_label), diff_text);
                gtk_box_append(GTK_BOX(frame_content), diff_label);

                frame = gtk_frame_new(NULL);
                gtk_widget_set_margin_top(frame, 6);
                gtk_widget_set_margin_bottom(frame, 6);
                gtk_widget_set_hexpand(frame, TRUE);
                gtk_frame_set_child(GTK_FRAME(frame), frame_content);

                gtk_box_append(GTK_BOX(box), frame);
        }
        if (diff_count < 10) {
                GtkWidget * spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
                gtk_widget_set_vexpand(spacer, TRUE);
                gtk_box_append(GTK_BOX(box), spacer);
        }

        gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), box);
        gtk_box_append(GTK_BOX(content_box), scrolled_window);

        gtk_widget_set_halign(button_box, GTK_ALIGN_END);
        gtk_widget_set_margin_top(button_box, 12);

        g_signal_connect_swapped(close_button, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
        gtk_box_append(GTK_BOX(button_box), close_button);

        gtk_box_append(GTK_BOX(content_box), button_box);

        adw_window_set_content(ADW_WINDOW(dialog), content_box);
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
                adw_action_row_set_subtitle(self->original_ar, self -> dir1_path);
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
                adw_action_row_set_subtitle(self->corrupted_ar, self -> dir2_path);
                g_object_unref(file);
        }
}

static void
on_dir1_button_clicked(GtkButton * button, BincheckerWindow * self) {
        GtkFileDialog * dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, "Select First Directory");
        gtk_file_dialog_set_modal(dialog, TRUE);

        gtk_file_dialog_select_folder(dialog, GTK_WINDOW(self), NULL, on_dir1_folder_selected, self);
}

static void
on_dir2_button_clicked(GtkButton * button, BincheckerWindow * self) {
        GtkFileDialog * dialog = gtk_file_dialog_new();
        gtk_file_dialog_set_title(dialog, "Select Second Directory");
        gtk_file_dialog_set_modal(dialog, TRUE);

        gtk_file_dialog_select_folder(dialog, GTK_WINDOW(self), NULL, on_dir2_folder_selected, self);
}

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
on_scan_button_clicked(GtkButton * button, BincheckerWindow * self) {
        GtkWidget * child;
        char ** files1 = NULL;
        int count1 = 0;
        int capacity1 = 0;
        double padding_value;
        int padding;
        int i;

        if (!self -> dir1_path || !self -> dir2_path) {
                GtkAlertDialog * dialog = gtk_alert_dialog_new("Please select both directories first");
                gtk_alert_dialog_show(dialog, GTK_WINDOW(self));
                g_object_unref(dialog);
                return;
        }

        // Clear previous results
        while ((child = gtk_widget_get_first_child(GTK_WIDGET(self -> file_list)))) {
                gtk_list_box_remove(self -> file_list, child);
        }

        // Clear previous diffs
        if (self -> file_diffs) {
                g_hash_table_remove_all(self -> file_diffs);
        } else {
                self -> file_diffs = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free_diff_chunk);
        }

        // Get file list
        get_all_file_paths(self -> dir1_path, & files1, & count1, & capacity1);

        // Get padding value
        padding_value = adw_spin_row_get_value(self -> padding_spin);
        padding = (int) padding_value;

        for (i = 0; i < count1; i++) {
                const char * basename;
                char fp1[512], fp2[512];
                struct diffChunk * diffs;

                basename = g_path_get_basename(files1[i]);
                snprintf(fp1, sizeof(fp1), "%s/%s", self -> dir1_path, basename);
                snprintf(fp2, sizeof(fp2), "%s/%s", self -> dir2_path, basename);
                diffs = compare_files(fp1, fp2, padding);
                if (diffs) {
                        GtkWidget * row;
                        GtkWidget * label;
                        g_hash_table_insert(self -> file_diffs, g_strdup(basename), diffs);
                        row = gtk_list_box_row_new();
                        label = gtk_label_new(basename);
                        gtk_widget_set_margin_start(label, 12);
                        gtk_widget_set_margin_end(label, 12);
                        gtk_widget_set_margin_top(label, 6);
                        gtk_widget_set_margin_bottom(label, 6);
                        gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), label);
                        gtk_list_box_append(self -> file_list, row);
                }

                g_free((char * ) basename);
        }

        // Cleanup
        for (i = 0; i < count1; i++) free(files1[i]);
        free(files1);
        adw_navigation_view_push(self->navigation_view, self->results_page);
}

static void
binchecker_window_dispose(GObject * object) {
        BincheckerWindow * self = BINCHECKER_WINDOW(object);

        g_clear_pointer( & self -> dir1_path, g_free);
        g_clear_pointer( & self -> dir2_path, g_free);
        g_clear_pointer( & self -> file_diffs, g_hash_table_destroy);

        G_OBJECT_CLASS(binchecker_window_parent_class) -> dispose(object);
}

static void
binchecker_window_class_init(BincheckerWindowClass * klass) {
        GtkWidgetClass * widget_class = GTK_WIDGET_CLASS(klass);
        GObjectClass * object_class = G_OBJECT_CLASS(klass);

        object_class -> dispose = binchecker_window_dispose;

        gtk_widget_class_set_template_from_resource(widget_class, "/org/gnome/BinChecker/binchecker-window.ui");
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
