/* binchecker-diff-dialog.h
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

#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>
#include "libbindiff.h"
#include <glib/gi18n.h>

G_BEGIN_DECLS

#define BINCHECKER_TYPE_DIFF_DIALOG (binchecker_diff_dialog_get_type())
G_DECLARE_FINAL_TYPE(BincheckerDiffDialog, binchecker_diff_dialog, BINCHECKER, DIFF_DIALOG, AdwWindow)

BincheckerDiffDialog *binchecker_diff_dialog_new(GtkWindow *parent);
void binchecker_diff_dialog_set_data(BincheckerDiffDialog *dialog,
                                    const char *filename,
                                    struct diffChunk *diffs);

G_END_DECLS
