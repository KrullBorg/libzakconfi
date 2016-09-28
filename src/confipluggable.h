/*
 * confipluggable.h
 * This file is part of libconfi
 *
 * Copyright (C) 2014-2016 - Andrea Zagli <azagli@libero.it>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __ZAK_CONFI_PLUGGABLE_H__
#define __ZAK_CONFI_PLUGGABLE_H__

#include <glib-object.h>

#include "commons.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define ZAK_CONFI_TYPE_PLUGGABLE             (zak_confi_pluggable_get_type ())
#define ZAK_CONFI_PLUGGABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ZAK_CONFI_TYPE_PLUGGABLE, ZakConfiPluggable))
#define ZAK_CONFI_PLUGGABLE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), ZAK_CONFI_TYPE_PLUGGABLE, ZakConfiPluggableInterface))
#define ZAK_CONFI_IS_PLUGGABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ZAK_CONFI_TYPE_PLUGGABLE))
#define ZAK_CONFI_PLUGGABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), ZAK_CONFI_TYPE_PLUGGABLE, ZakConfiPluggableInterface))

/**
 * ZakConfiPluggable:
 *
 * Interface for pluggable plugins.
 */
typedef struct _ZakConfiPluggable           ZakConfiPluggable; /* dummy typedef */
typedef struct _ZakConfiPluggableInterface  ZakConfiPluggableInterface;

/**
 * ZakConfiPluggableInterface:
 * @g_iface: The parent interface.
 * @initialize: Construct the plugin.
 *
 * Provides an interface for pluggable plugins.
 */
struct _ZakConfiPluggableInterface {
	GTypeInterface g_iface;

	/* Virtual public methods */
	gboolean (*initialize) (ZakConfiPluggable *pluggable, const gchar *cnc_string);
	GList *(*get_configs_list) (ZakConfiPluggable *pluggable,
	                            const gchar *filter);
	gchar *(*path_get_value) (ZakConfiPluggable *pluggable, const gchar *path);
	gboolean (*path_set_value) (ZakConfiPluggable *pluggable,
	                            const gchar *path,
	                            const gchar *value);
	GNode *(*get_tree) (ZakConfiPluggable *pluggable);
	ZakConfiConfi *(*add_config) (ZakConfiPluggable *pluggable,
	                      const gchar *name,
	                      const gchar *description);
	gboolean (*set_config) (ZakConfiPluggable *pluggable,
	                      const gchar *name,
	                      const gchar *description);
	ZakConfiKey *(*add_key) (ZakConfiPluggable *pluggable,
	                      const gchar *parent,
	                      const gchar *key,
	                      const gchar *value);
	ZakConfiKey *(*path_get_confi_key) (ZakConfiPluggable *pluggable, const gchar *path);
	gboolean (*remove_path) (ZakConfiPluggable *pluggable,
	                         const gchar *path);
	gboolean (*remove) (ZakConfiPluggable *pluggable);
	gboolean (*key_set_key) (ZakConfiPluggable *pluggable,
	                         ZakConfiKey *ck);
};

/*
 * Public methods
 */
GType zak_confi_pluggable_get_type (void) G_GNUC_CONST;

gboolean zak_confi_pluggable_initialize (ZakConfiPluggable *pluggable,
                                     const gchar *cnc_string);

GList *zak_confi_pluggable_get_configs_list (ZakConfiPluggable *pluggable,
                                         const gchar *filter);
gchar *zak_confi_pluggable_path_get_value (ZakConfiPluggable *pluggable,
                                       const gchar *path);
gboolean zak_confi_pluggable_path_set_value (ZakConfiPluggable *pluggable,
                               const gchar *path,
                               const gchar *value);
GNode *zak_confi_pluggable_get_tree (ZakConfiPluggable *pluggable);
ZakConfiConfi *zak_confi_pluggable_add_config (ZakConfiPluggable *pluggable,
                                   const gchar *name,
                                   const gchar *description);
gboolean zak_confi_pluggable_set_config (ZakConfiPluggable *pluggable,
                                   const gchar *name,
                                   const gchar *description);
ZakConfiKey *zak_confi_pluggable_add_key (ZakConfiPluggable *pluggable,
                                   const gchar *parent,
                                   const gchar *key,
                                   const gchar *value);
gboolean zak_confi_pluggable_key_set_key (ZakConfiPluggable *pluggable,
                   ZakConfiKey *ck);
ZakConfiKey *zak_confi_pluggable_path_get_confi_key (ZakConfiPluggable *pluggable, const gchar *path);
gboolean zak_confi_pluggable_remove_path (ZakConfiPluggable *pluggable, const gchar *path);
gboolean zak_confi_pluggable_remove (ZakConfiPluggable *pluggable);


G_END_DECLS

#endif /* __ZAK_CONFI_PLUGGABLE_H__ */
