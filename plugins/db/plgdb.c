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

static void confi_pluggable_iface_init (ConfiPluggableInterface *iface);

static GdaDataModel *confi_db_plugin_path_get_data_model (ConfiPluggable *pluggable, const gchar *path);
static gchar *confi_db_plugin_path_get_value_from_db (ConfiPluggable *pluggable, const gchar *path);
static void confi_db_plugin_get_children (ConfiPluggable *pluggable, GNode *parentNode, gint idParent, gchar *path);

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
	gchar *sql;

	ConfiDBPlugin *plugin = CONFI_DB_PLUGIN (object);
	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	switch (prop_id)
		{
			case PROP_CNC_STRING:
				confi_db_plugin_initialize ((ConfiPluggable *)plugin, g_value_get_string (value));
				break;

			case PROP_NAME:
				priv->name = g_strdup (g_value_get_string (value));
				sql = g_strdup_printf ("UPDATE configs "
				                       "SET name = '%s' "
				                       "WHERE id = %d",
				                       gdaex_strescape (priv->name, NULL),
				                       priv->id_config);
				gdaex_execute (priv->gdaex, sql);
				g_free (sql);
				break;

			case PROP_DESCRIPTION:
				priv->description = g_strdup (g_value_get_string (value));
				sql = g_strdup_printf ("UPDATE configs "
				                       "SET description = '%s' "
				                       "WHERE id = %d",
				                       gdaex_strescape (priv->description, NULL),
				                       priv->id_config);
				gdaex_execute (priv->gdaex, sql);
				g_free (sql);
				break;

			case PROP_ROOT:
				priv->root = confi_normalize_root (g_value_get_string (value));
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
	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	priv->cnc_string = NULL;
	priv->gdaex = NULL;
	priv->name = NULL;
	priv->description = NULL;
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

	gchar *sql;
	GdaDataModel *dm;

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

	if (priv->name == NULL)
		{
			priv->name = g_strdup ("Default");
		}

	priv->gdaex = gdaex_new_from_string (priv->cnc_string);
	priv->chrquot = gdaex_get_chr_quoting (priv->gdaex);

	/* check if config exists */
	sql = g_strdup_printf ("SELECT id, name"
	                       " FROM configs"
	                       " WHERE name = '%s'",
	                       gdaex_strescape (priv->name, NULL));
	dm = gdaex_query (priv->gdaex, sql);
	g_free (sql);
	if (dm != NULL || gda_data_model_get_n_rows (dm) > 0)
		{
			priv->id_config = gdaex_data_model_get_value_integer_at (dm, 0, 0);
		}
	if (dm != NULL)
		{
			g_object_unref (dm);
		}

	return (priv->gdaex != NULL && priv->name != NULL ? TRUE : FALSE);
}

static GdaDataModel
*confi_db_plugin_path_get_data_model (ConfiPluggable *pluggable, const gchar *path)
{
	gchar **tokens;
	gchar *sql;
	gchar *token;
	guint i;
	guint id_parent;
	GdaDataModel *dm;

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	if (path == NULL) return NULL;

	dm = NULL;

	tokens = g_strsplit (path, "/", 0);
	if (tokens == NULL) return NULL;

	i = 0;
	id_parent = 0;
	while (tokens[i] != NULL)
		{
			token = g_strstrip (g_strdup (tokens[i]));
			if (strcmp (token, "") != 0)
				{
					sql = g_strdup_printf ("SELECT * "
					                       "FROM %cvalues%c "
					                       "WHERE id_configs = %d "
					                       "AND id_parent = %d "
					                       "AND %ckey%c = '%s'",
					                       priv->chrquot, priv->chrquot,
					                       priv->id_config,
					                       id_parent,
					                       priv->chrquot, priv->chrquot,
					                       gdaex_strescape (token, NULL));
					dm = gdaex_query (priv->gdaex, sql);
					g_free (sql);
					if (dm == NULL || gda_data_model_get_n_rows (dm) != 1)
						{
							/* TO DO */
							g_warning ("Unable to find key «%s».", token);
							if (dm != NULL)
								{
									g_object_unref (dm);
									dm = NULL;
								}
							break;
						}
					id_parent = gdaex_data_model_get_field_value_integer_at (dm, 0, "id");
				}

			i++;
		}

	return dm;
}

static gchar
*confi_db_plugin_path_get_value_from_db (ConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;
	GdaDataModel *dm;

	ret = NULL;

	dm = confi_db_plugin_path_get_data_model (pluggable, path);
	if (dm != NULL)
		{
			ret = gdaex_data_model_get_field_value_stringify_at (dm, 0, "value");
			g_object_unref (dm);
		}

	return ret;
}

static void
confi_db_plugin_get_children (ConfiPluggable *pluggable, GNode *parentNode, gint idParent, gchar *path)
{
	gchar *sql;
	GdaDataModel *dm;

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	sql = g_strdup_printf ("SELECT * FROM %cvalues%c "
	                              "WHERE id_configs = %d AND "
	                              "id_parent = %d",
	                              priv->chrquot, priv->chrquot,
	                              priv->id_config,
	                              idParent);

	dm = gdaex_query (priv->gdaex, sql);
	g_free (sql);
	if (dm != NULL)
		{
			guint i;
			guint rows;

			rows = gda_data_model_get_n_rows (dm);
			for (i = 0; i < rows; i++)
				{
					GNode *newNode;
					ConfiKey *ck = g_new0 (ConfiKey, 1);

					ck->id_config = priv->id_config;
					ck->id = gdaex_data_model_get_field_value_integer_at (dm, i, "id");
					ck->id_parent = gdaex_data_model_get_field_value_integer_at (dm, i, "id_parent");
					ck->key = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "key"));
					ck->value = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "value"));
					ck->description = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "description"));
					ck->path = g_strdup (path);

					newNode = g_node_append_data (parentNode, ck);

					confi_db_plugin_get_children (pluggable, newNode, ck->id, g_strconcat (path, (g_strcmp0 (path, "") == 0 ? "" : "/"), ck->key, NULL));
				}
			g_object_unref (dm);
		}
}

static GList
*confi_db_plugin_get_configs_list (ConfiPluggable *pluggable,
                                   const gchar *filter)
{
	GList *lst;

	GdaDataModel *dmConfigs;

	gchar *sql;
	gchar *where;

	guint row;
	guint id;
	guint rows;

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
	else
		{
			where = g_strdup ("");
		}

	sql = g_strdup_printf ("SELECT * FROM configs%s", where);
	g_free (where);

	dmConfigs = gdaex_query (priv->gdaex, sql);
	g_free (sql);
	if (dmConfigs != NULL)
		{
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

static gchar
*confi_db_plugin_path_get_value (ConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;
	gchar *path_;

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	ret = NULL;

	path_ = confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	ret = confi_db_plugin_path_get_value_from_db (pluggable, path_);

	return ret;
}

static gboolean
confi_db_plugin_path_set_value (ConfiPluggable *pluggable, const gchar *path, const gchar *value)
{
	GdaDataModel *dm;
	gchar *sql;
	gboolean ret;

	dm = confi_db_plugin_path_get_data_model (pluggable, confi_path_normalize (pluggable, path));

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	ret = FALSE;
	if (dm != NULL && gda_data_model_get_n_rows (dm) > 0)
		{
			sql = g_strdup_printf ("UPDATE %cvalues%c SET value = '%s' "
			                              "WHERE id_configs = %d "
			                              "AND id = %d ",
			                              priv->chrquot, priv->chrquot,
			                              gdaex_strescape (value, NULL),
			                              priv->id_config,
			                              gdaex_data_model_get_field_value_integer_at (dm, 0, "id"));
			ret = (gdaex_execute (priv->gdaex, sql) >= 0);
			g_free (sql);
		}
	else
		{
			g_warning ("Path %s doesn't exists.", path);
		}
	if (dm != NULL)
		{
			g_object_unref (dm);
		}

	return ret;
}

GNode
*confi_db_plugin_get_tree (ConfiPluggable *pluggable)
{
	gchar *path;
	GNode *node;

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	path = g_strdup ("");

	ConfiKey *ck = g_new0 (ConfiKey, 1);

	ck->id_config = priv->id_config;
	ck->id = 0;
	ck->id_parent = 0;
	ck->key = g_strdup ("/");

	node = g_node_new (ck);

	confi_db_plugin_get_children (pluggable, node, 0, path);

	return node;
}

static ConfiKey
*confi_db_plugin_add_key (ConfiPluggable *pluggable, const gchar *parent, const gchar *key, const gchar *value)
{
	ConfiKey *ck;
	GdaDataModel *dmParent;

	gchar *sql;
	gint id;
	GdaDataModel *dm;

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	gint id_parent;
	gchar *parent_;
	gchar *key_;

	gchar *path;

	ck = NULL;
	if (parent == NULL)
		{
			id_parent = 0;
		}
	else
		{
			parent_ = g_strstrip (g_strdup (parent));
			if (strcmp (parent_, "") == 0)
				{
					id_parent = 0;
				}
			else
				{
					dmParent = confi_db_plugin_path_get_data_model (pluggable, confi_path_normalize (pluggable, parent_));
					if (dmParent == NULL)
						{
							id_parent = -1;
						}
					else
						{
							id_parent = gdaex_data_model_get_field_value_integer_at (dmParent, 0, "id");
						}
				}
		}

	if (id_parent > -1)
		{
			id = 0;

			/* find new id */
			sql = g_strdup_printf ("SELECT MAX(id) FROM %cvalues%c "
			                       "WHERE id_configs = %d ",
			                       priv->chrquot, priv->chrquot,
			                       priv->id_config);
			dm = gdaex_query (priv->gdaex, sql);
			g_free (sql);
			if (dm != NULL)
				{
					id = gdaex_data_model_get_value_integer_at (dm, 0, 0);
					g_object_unref (dm);
				}
			id++;

			key_ = g_strstrip (g_strdup (key));

			sql = g_strdup_printf ("INSERT INTO %cvalues%c "
			                       "(id_configs, id, id_parent, %ckey%c, value) "
			                       "VALUES (%d, %d, %d, '%s', '%s')",
			                       priv->chrquot, priv->chrquot,
			                       priv->chrquot, priv->chrquot,
			                       priv->id_config,
			                       id,
			                       id_parent,
			                       gdaex_strescape (key_, NULL),
			                       "");
			if (gdaex_execute (priv->gdaex, sql) == -1)
				{
					/* TO DO */
					g_free (sql);
					return NULL;
				}
			g_free (sql);

			ck = g_new0 (ConfiKey, 1);
			ck->id_config = priv->id_config;
			ck->id = id;
			ck->id_parent = id_parent;
			ck->key = g_strdup (key_);
			ck->value = g_strdup ("");
			ck->description = g_strdup ("");
			if (id_parent == 0)
				{
					ck->path = g_strdup ("");
				}
			else
				{
					ck->path = g_strdup (parent_);
				}

			g_free (key_);
		}
	g_free (parent_);

	if (ck != NULL)
		{
			if (ck->id_parent != 0)
				{
					path = g_strconcat (ck->path, "/", key, NULL);
				}
			else
				{
					path = g_strdup (ck->key);
				}

			if (!confi_db_plugin_path_set_value (pluggable, path, value))
				{
					ck = NULL;
				}
			else
				{
					ck->value = g_strdup (value);
				}
			g_free (path);
		}

	return ck;
}

static ConfiKey
*confi_db_plugin_path_get_confi_key (ConfiPluggable *pluggable, const gchar *path)
{
	GdaDataModel *dm;
	gchar *path_;
	ConfiKey *ck;

	ConfiDBPluginPrivate *priv = CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	path_ = confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	dm = confi_db_plugin_path_get_data_model (pluggable, path_);
	if (dm == NULL || gda_data_model_get_n_rows (dm) <= 0)
		{
			if (dm != NULL)
				{
					g_object_unref (dm);
				}
			return NULL;
		}

	ck = g_new0 (ConfiKey, 1);
	ck->id_config = gdaex_data_model_get_field_value_integer_at (dm, 0, "id_configs");
	ck->id = gdaex_data_model_get_field_value_integer_at (dm, 0, "id");
	ck->id_parent = gdaex_data_model_get_field_value_integer_at (dm, 0, "id_parent");
	ck->key = gdaex_data_model_get_field_value_stringify_at (dm, 0, "key");
	ck->value = gdaex_data_model_get_field_value_stringify_at (dm, 0, "value");
	ck->description = gdaex_data_model_get_field_value_stringify_at (dm, 0, "description");
	ck->path = g_strdup (path_);

	if (dm != NULL)
		{
			g_object_unref (dm);
		}

	return ck;
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
	iface->path_get_value = confi_db_plugin_path_get_value;
	iface->path_set_value = confi_db_plugin_path_set_value;
	iface->get_tree = confi_db_plugin_get_tree;
	iface->add_key = confi_db_plugin_add_key;
	iface->path_get_confi_key = confi_db_plugin_path_get_confi_key;
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
