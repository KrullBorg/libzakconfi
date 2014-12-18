/*
 * confipluggable.h
 * This file is part of libconfi
 *
 * Copyright (C) 2014 - Andrea Zagli <azagli@libero.it>
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

#ifndef __CONFI_PLUGGABLE_H__
#define __CONFI_PLUGGABLE_H__

#include <glib-object.h>

#include "commons.h"

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define CONFI_TYPE_PLUGGABLE             (confi_pluggable_get_type ())
#define CONFI_PLUGGABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CONFI_TYPE_PLUGGABLE, ConfiPluggable))
#define CONFI_PLUGGABLE_IFACE(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), CONFI_TYPE_PLUGGABLE, ConfiPluggableInterface))
#define CONFI_IS_PLUGGABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CONFI_TYPE_PLUGGABLE))
#define CONFI_PLUGGABLE_GET_IFACE(obj)   (G_TYPE_INSTANCE_GET_INTERFACE ((obj), CONFI_TYPE_PLUGGABLE, ConfiPluggableInterface))

/**
 * ConfiPluggable:
 *
 * Interface for pluggable plugins.
 */
typedef struct _ConfiPluggable           ConfiPluggable; /* dummy typedef */
typedef struct _ConfiPluggableInterface  ConfiPluggableInterface;

/**
 * ConfiPluggableInterface:
 * @g_iface: The parent interface.
 * @initialize: Construct the plugin.
 *
 * Provides an interface for pluggable plugins.
 */
struct _ConfiPluggableInterface {
	GTypeInterface g_iface;

	/* Virtual public methods */
	gboolean (*initialize) (ConfiPluggable *pluggable, const gchar *cnc_string);
	GList *(*get_configs_list) (ConfiPluggable *pluggable,
	                            const gchar *filter);
	gchar *(*path_get_value) (ConfiPluggable *pluggable, const gchar *path);
	gboolean (*path_set_value) (ConfiPluggable *pluggable,
	                            const gchar *path,
	                            const gchar *value);
	GNode *(*get_tree) (ConfiPluggable *pluggable);
	ConfiKey *(*add_key) (ConfiPluggable *pluggable,
	                      const gchar *parent,
	                      const gchar *key,
	                      const gchar *value);
	ConfiKey *(*path_get_confi_key) (ConfiPluggable *pluggable, const gchar *path);
	gboolean (*remove_path) (ConfiPluggable *pluggable,
	                         const gchar *path);
	gboolean (*remove) (ConfiPluggable *pluggable);
	gboolean (*key_set_key) (ConfiPluggable *pluggable,
	                         ConfiKey *ck);
};

/*
 * Public methods
 */
GType confi_pluggable_get_type (void) G_GNUC_CONST;

gboolean confi_pluggable_initialize (ConfiPluggable *pluggable,
                                     const gchar *cnc_string);

GList *confi_pluggable_get_configs_list (ConfiPluggable *pluggable,
                                         const gchar *filter);
gchar *confi_pluggable_path_get_value (ConfiPluggable *pluggable,
                                       const gchar *path);
gboolean confi_pluggable_path_set_value (ConfiPluggable *pluggable,
                               const gchar *path,
                               const gchar *value);
GNode *confi_pluggable_get_tree (ConfiPluggable *pluggable);
ConfiKey *confi_pluggable_add_key (ConfiPluggable *pluggable,
                                   const gchar *parent,
                                   const gchar *key,
                                   const gchar *value);
gboolean confi_pluggable_key_set_key (ConfiPluggable *pluggable,
                   ConfiKey *ck);
ConfiKey *confi_pluggable_path_get_confi_key (ConfiPluggable *pluggable, const gchar *path);
gboolean confi_pluggable_remove_path (ConfiPluggable *pluggable, const gchar *path);
gboolean confi_pluggable_remove (ConfiPluggable *pluggable);


G_END_DECLS

#endif /* __CONFI_PLUGGABLE_H__ */
