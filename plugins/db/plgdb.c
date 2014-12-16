/*
 * plgdb.c
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include <libgdaex/libgdaex.h>

#include "../../src/libconfi.h"
#include "../../src/confipluggable.h"

#include "plgdb.h"

static void confi_pluggable_iface_init (ConfiPluggableInterface    *iface);

#define CONFI_DB_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CONFI_TYPE_DB_PLUGIN, ConfiDBPluginPrivate))

typedef struct _ConfiDBPluginPrivate ConfiDBPluginPrivate;
struct _ConfiDBPluginPrivate
	{
		gchar *cnc_string;

		GdaEx *gdaex;

		gint id_config;
		gchar *name;
		gchar *description;
		gchar *root;
		GHashTable *values;

		gchar chrquot;
	};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (ConfiDBPlugin,
                                confi_db_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (CONFI_TYPE_PLUGGABLE,
                                                               confi_pluggable_iface_init))

enum {
	PROP_0,
	PROP_CNC_STRING,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_ROOT
};

static void
confi_db_plugin_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	ConfiDBPlugin *plugin = CONFI_DB_PLUGIN (object);
	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	switch (prop_id)
		{
			case PROP_CNC_STRING:
				confi_db_plugin_initialize ((ConfiPluggable *)plugin, g_value_get_string (value));
				break;

			case PROP_NAME:
				break;

			case PROP_DESCRIPTION:
				break;

			case PROP_ROOT:
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
				break;
		}
}

static void
confi_db_plugin_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	ConfiDBPlugin *plugin = CONFI_DB_PLUGIN (object);
	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	switch (prop_id)
		{
			case PROP_CNC_STRING:
				g_value_set_string (value, priv->cnc_string);
				break;

			case PROP_NAME:
				g_value_set_string (value, priv->name);
				break;

			case PROP_DESCRIPTION:
				g_value_set_string (value, priv->description);
				break;

			case PROP_ROOT:
				g_value_set_string (value, priv->root);
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
				break;
		}
}

static void
confi_db_plugin_init (ConfiDBPlugin *plugin)
{
}

static void
confi_db_plugin_finalize (GObject *object)
{
	ConfiDBPlugin *plugin = CONFI_DB_PLUGIN (object);

	G_OBJECT_CLASS (confi_db_plugin_parent_class)->finalize (object);
}

gboolean
confi_db_plugin_initialize (ConfiPluggable *pluggable, const gchar *cnc_string)
{
	ConfiDBPlugin *plugin = CONFI_DB_PLUGIN (pluggable);
	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	GString *gstr_cnc_string;
	gchar **strs;
	guint i;
	guint l;

	strs = g_strsplit (cnc_string, ";", -1);

	gstr_cnc_string = g_string_new ("");

	l = g_strv_length (strs);
	for (i = 0; i < l; i++)
		{
			if (g_str_has_prefix (strs[i], "CONFI_NAME="))
				{
					priv->name = g_strdup (strs[i] + strlen ("CONFI_NAME="));
					if (priv->name[strlen (priv->name)] == ';')
						{
							priv->name[strlen (priv->name)] = '\0';
						}
				}
			else
				{
					g_string_append (gstr_cnc_string, strs[i]);
					g_string_append (gstr_cnc_string, ";");
				}
		}
	if (priv->cnc_string != NULL)
		{
			g_free (priv->cnc_string);
		}
	priv->cnc_string = g_strdup (gstr_cnc_string->str);

	g_string_free (gstr_cnc_string, TRUE);
	g_strfreev (strs);

	priv->gdaex = gdaex_new_from_string (priv->cnc_string);

	return (priv->gdaex != NULL ? TRUE : FALSE);
}

static GList
*confi_db_plugin_get_configs_list (ConfiPluggable *pluggable,
                                   const gchar *filter)
{
	GList *lst;

	gchar *sql;
	gchar *where = "";

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	lst = NULL;

	if (priv->gdaex == NULL)
		{
			return NULL;
		}

	if (filter != NULL && strcmp (g_strstrip (g_strdup (filter)), "") != 0)
		{
			where = g_strdup_printf (" WHERE name LIKE '%s'", filter);
		}

	sql = g_strdup_printf ("SELECT * FROM configs%s", where);

	GdaDataModel *dmConfigs = gdaex_query (priv->gdaex, sql);
	if (dmConfigs != NULL)
		{
			guint row;
			guint id;
			guint rows;

			rows = gda_data_model_get_n_rows (dmConfigs);
			if (rows > 0)
				{
					for (row = 0; row < rows; row++)
						{
							ConfiConfi *confi;
							confi = g_new0 (ConfiConfi, 1);
							confi->name = gdaex_data_model_get_field_value_stringify_at (dmConfigs, row, "name");
							confi->description = gdaex_data_model_get_field_value_stringify_at (dmConfigs, row, "description");
							lst = g_list_append (lst, confi);
						}
				}
			else
				{
					lst = g_list_append (lst, NULL);
				}
		}

	return lst;
}

static void
confi_db_plugin_class_init (ConfiDBPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ConfiDBPluginPrivate));

	object_class->set_property = confi_db_plugin_set_property;
	object_class->get_property = confi_db_plugin_get_property;
	object_class->finalize = confi_db_plugin_finalize;

	g_object_class_override_property (object_class, PROP_CNC_STRING, "cnc_string");
	g_object_class_override_property (object_class, PROP_NAME, "name");
	g_object_class_override_property (object_class, PROP_DESCRIPTION, "description");
	g_object_class_override_property (object_class, PROP_ROOT, "root");
}

static void
confi_pluggable_iface_init (ConfiPluggableInterface *iface)
{
	iface->initialize = confi_db_plugin_initialize;
	iface->get_configs_list = confi_db_plugin_get_configs_list;
}

static void
confi_db_plugin_class_finalize (ConfiDBPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	confi_db_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            CONFI_TYPE_PLUGGABLE,
	                                            CONFI_TYPE_DB_PLUGIN);
}
