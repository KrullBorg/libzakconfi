/*
 *  Copyright (C) 2005-2016 Andrea Zagli <azagli@libero.it>
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

#ifndef __LIBZAKCONFI_H__
#define __LIBZAKCONFI_H__

#include <glib-object.h>

#include <libpeas/peas.h>

#include "commons.h"
#include "confipluggable.h"


G_BEGIN_DECLS


#define ZAK_TYPE_CONFI                 (zak_confi_get_type ())
#define ZAK_CONFI(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), ZAK_TYPE_CONFI, ZakConfi))
#define ZAK_CONFI_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), ZAK_TYPE_CONFI, ZakConfiClass))
#define ZAK_IS_CONFI(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ZAK_TYPE_CONFI))
#define ZAK_IS_CONFI_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), ZAK_TYPE_CONFI))
#define ZAK_CONFI_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), ZAK_TYPE_CONFI, ZakConfiClass))


typedef struct _ZakConfi ZakConfi;
typedef struct _ZakConfiClass ZakConfiClass;

struct _ZakConfi
	{
		GObject parent;
	};

struct _ZakConfiClass
	{
		GObjectClass parent_class;
	};

GType zak_confi_get_type (void);

ZakConfi *zak_confi_new (const gchar *cnc_string);

PeasPluginInfo *zak_confi_get_plugin_info (ZakConfi *confi);

GList *zak_confi_get_configs_list (const gchar *cnc_string,
								   const gchar *filter);

ZakConfiConfi *zak_confi_add_config (const gchar *cnc_string,
									 const gchar *name,
									 const gchar *description);

GNode *zak_confi_get_tree (ZakConfi *confi);

gchar *zak_confi_normalize_root (const gchar *root);
gboolean zak_confi_set_root (ZakConfi *confi, const gchar *root);

gboolean zak_confi_set_config (ZakConfi *confi,
							   const gchar *name,
							   const gchar *description);

ZakConfiKey *zak_confi_add_key (ZakConfi *confi,
                         const gchar *parent,
                         const gchar *key,
                         const gchar *value);

gboolean zak_confi_key_set_key (ZakConfi *confi,
                            ZakConfiKey *ck);

gboolean zak_confi_remove_path (ZakConfi *confi,
                            const gchar *path);

gchar *zak_confi_path_get_value (ZakConfi *confi,
                             const gchar *path);
gboolean zak_confi_path_set_value (ZakConfi *confi,
                               const gchar *path,
                               const gchar *value);

ZakConfiKey *zak_confi_path_get_confi_key (ZakConfi *confi,
                                    const gchar *path);

gboolean zak_confi_remove (ZakConfi *confi);

void zak_confi_destroy (ZakConfi *confi);

gchar *zak_confi_path_normalize (ZakConfiPluggable *pluggable, const gchar *path);


G_END_DECLS

#endif /* __LIBZAKCONFI_H__ */
