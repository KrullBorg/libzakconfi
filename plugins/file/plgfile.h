/*
 * plgfile.h
 * This file is part of confi
 *
 * Copyright (C) 2014 Andrea Zagli
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

#ifndef __CONFI_FILE_PLUGIN_H__
#define __CONFI_FILE_PLUGIN_H__

#include <libpeas/peas.h>

G_BEGIN_DECLS

#define CONFI_TYPE_FILE_PLUGIN         (confi_file_plugin_get_type ())
#define CONFI_FILE_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), CONFI_TYPE_FILE_PLUGIN, ConfiFilePlugin))
#define CONFI_FILE_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), CONFI_TYPE_FILE_PLUGIN, ConfiFilePlugin))
#define CONFI_IS_FILE_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), CONFI_TYPE_FILE_PLUGIN))
#define CONFI_IS_FILE_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), CONFI_TYPE_FILE_PLUGIN))
#define CONFI_FILE_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), CONFI_TYPE_FILE_PLUGIN, ConfiFilePluginClass))

typedef struct _ConfiFilePlugin       ConfiFilePlugin;
typedef struct _ConfiFilePluginClass  ConfiFilePluginClass;

struct _ConfiFilePlugin {
	PeasExtensionBase parent_instance;
};

struct _ConfiFilePluginClass {
	PeasExtensionBaseClass parent_class;
};

GType                 confi_file_plugin_get_type        (void) G_GNUC_CONST;
G_MODULE_EXPORT void  peas_register_types                         (PeasObjectModule *module);

G_END_DECLS

#endif /* __CONFI_FILE_PLUGIN_H__ */
