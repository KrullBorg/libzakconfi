/*
 * plgdb.h
 * This file is part of confi
 *
 * Copyright (C) 2014-2016 Andrea Zagli <azagli@libero.it>
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

#ifndef __ZAK_CONFI_DB_PLUGIN_H__
#define __ZAK_CONFI_DB_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define ZAK_CONFI_TYPE_DB_PLUGIN         (zak_confi_db_plugin_get_type ())
#define ZAK_CONFI_DB_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ZAK_CONFI_TYPE_DB_PLUGIN, ZakConfiDBPlugin))
#define ZAK_CONFI_DB_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), ZAK_CONFI_TYPE_DB_PLUGIN, ZakConfiDBPlugin))
#define ZAK_CONFI_IS_DB_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), ZAK_CONFI_TYPE_DB_PLUGIN))
#define ZAK_CONFI_IS_DB_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), ZAK_CONFI_TYPE_DB_PLUGIN))
#define ZAK_CONFI_DB_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), ZAK_CONFI_TYPE_DB_PLUGIN, ZakConfiDBPluginClass))

typedef struct _ZakConfiDBPlugin       ZakConfiDBPlugin;
typedef struct _ZakConfiDBPluginClass  ZakConfiDBPluginClass;

struct _ZakConfiDBPlugin {
	PeasExtensionBase parent_instance;
};

struct _ZakConfiDBPluginClass {
	PeasExtensionBaseClass parent_class;
};

GType                 zak_confi_db_plugin_get_type        (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types                         (PeasObjectModule *module);

G_END_DECLS

#endif /* __ZAK_CONFI_DB_PLUGIN_H__ */
