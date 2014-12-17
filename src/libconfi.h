/*
 *  Copyright (C) 2005-2014 Andrea Zagli <azagli@libero.it>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __LIBCONFI_H__
#define __LIBCONFI_H__

#include <glib-object.h>

#include "commons.h"
#include "confipluggable.h"


G_BEGIN_DECLS


#define TYPE_CONFI                 (confi_get_type ())
#define CONFI(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CONFI, Confi))
#define CONFI_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CONFI, ConfiClass))
#define IS_CONFI(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CONFI))
#define IS_CONFI_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CONFI))
#define CONFI_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CONFI, ConfiClass))


typedef struct _Confi Confi;
typedef struct _ConfiClass ConfiClass;

struct _Confi
	{
		GObject parent;
	};

struct _ConfiClass
	{
		GObjectClass parent_class;
	};

GType confi_get_type (void);

Confi *confi_new (const gchar *cnc_string);

GList *confi_get_configs_list (const gchar *cnc_string,
                               const gchar *filter);

GNode *confi_get_tree (Confi *confi);

gchar *confi_normalize_root (const gchar *root);

ConfiKey *confi_add_key (Confi *confi,
                         const gchar *parent,
                         const gchar *key,
                         const gchar *value);

gboolean confi_key_set_key (Confi *confi,
                            ConfiKey *ck);

gboolean confi_remove_path (Confi *confi,
                            const gchar *path);

gchar *confi_path_get_value (Confi *confi,
                             const gchar *path);
gboolean confi_path_set_value (Confi *confi,
                               const gchar *path,
                               const gchar *value);

gboolean confi_path_move (Confi *confi,
                          const gchar *path,
                          const gchar *parent);

ConfiKey *confi_path_get_confi_key (Confi *confi,
                                    const gchar *path);

gboolean confi_remove (Confi *confi);

void confi_destroy (Confi *confi);

gchar *confi_path_normalize (ConfiPluggable *pluggable, const gchar *path);


G_END_DECLS

#endif /* __LIBCONFI_H__ */
