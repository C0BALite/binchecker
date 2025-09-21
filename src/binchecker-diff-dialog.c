/* binchecker-diff-dialog.c
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


#include "binchecker-diff-dialog.h"
#include <stdio.h>

struct _BincheckerDiffDialog {
    AdwWindow parent_instance;

    GtkLabel *header_label;
    GtkBox *diff_container;
    GtkButton *close_button;
};

G_DEFINE_FINAL_TYPE(BincheckerDiffDialog, binchecker_diff_dialog, ADW_TYPE_WINDOW)

static void
binchecker_diff_dialog_class_init(BincheckerDiffDialogClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(widget_class, "/io/github/BinChecker/binchecker-dialog.ui");

    gtk_widget_class_bind_template_child(widget_class, BincheckerDiffDialog, header_label);
    gtk_widget_class_bind_template_child(widget_class, BincheckerDiffDialog, diff_container);
    gtk_widget_class_bind_template_child(widget_class, BincheckerDiffDialog, close_button);
}

static void
binchecker_diff_dialog_init(BincheckerDiffDialog *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect_swapped(self->close_button, "clicked",
                           G_CALLBACK(gtk_window_destroy), self);
}

BincheckerDiffDialog *
binchecker_diff_dialog_new(GtkWindow *parent)
{
    BincheckerDiffDialog *dialog = g_object_new(BINCHECKER_TYPE_DIFF_DIALOG, NULL);

    if (parent) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    }

    return dialog;
}

static void
add_diff_chunk(BincheckerDiffDialog *self, const struct diffChunk *diff, int index)
{
    char diff_text[1024];
    GtkWidget *diff_label;
    GtkWidget *frame;
    GtkWidget *frame_content;

    const char *file1_hex = diff->originalDiffFile ? (const char *)diff->originalDiffFile : "NULL";
    const char *file2_hex = diff->corruptedDiffFile ? (const char *)diff->corruptedDiffFile : "NULL";

    snprintf(diff_text, sizeof(diff_text),
            _("Difference %d:\nPosition: %d\nLength: %d bytes\n\nFile 1: %s\nFile 2: %s"),
            index + 1,
            diff->pos,
            diff->length,
            file1_hex,
            file2_hex);

    frame_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(frame_content, TRUE);

    diff_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(diff_label), TRUE);
    gtk_label_set_use_markup(GTK_LABEL(diff_label), TRUE);
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

    gtk_box_append(GTK_BOX(self->diff_container), frame);
}

void
binchecker_diff_dialog_set_data(BincheckerDiffDialog *self,
                               const char *filename,
                               struct diffChunk *diffs)
{
    char header[256];
    int diff_count = 0;
    int i;
    GtkWidget *child;

    snprintf(header, sizeof(header), _("Differences in: %s"), filename);
    gtk_label_set_text(self->header_label, header);

    while ((child = gtk_widget_get_first_child(GTK_WIDGET(self->diff_container)))) {
        gtk_box_remove(GTK_BOX(self->diff_container), child);
    }

    if (diffs) {
        while (diff_count < 10 && (diffs[diff_count].pos != 0 || diffs[diff_count].length != 0 ||
                diffs[diff_count].originalDiffFile != NULL || diffs[diff_count].corruptedDiffFile != NULL)) {
            diff_count++;
        }
    }

    for (i = 0; i < diff_count; i++) {
        add_diff_chunk(self, &diffs[i], i);
    }

    if (diff_count < 10) {
        GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_set_vexpand(spacer, TRUE);
        gtk_box_append(GTK_BOX(self->diff_container), spacer);
    }
}
