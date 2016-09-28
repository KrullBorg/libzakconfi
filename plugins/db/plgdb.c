/*
 * plgdb.c
 * This file is part of libzakconfi
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <glib.h>
#include <glib-object.h>
#include <gmodule.h>

#include <libpeas/peas.h>

#include <libgdaex/libgdaex.h>

#include "../../src/libzakconfi.h"
#include "../../src/confipluggable.h"

#include "plgdb.h"

static void zak_confi_pluggable_iface_init (ZakConfiPluggableInterface *iface);

static gboolean zak_confi_db_plugin_initialize (ZakConfiPluggable *pluggable, const gchar *cnc_string);

static GdaDataModel *zak_confi_db_plugin_path_get_data_model (ZakConfiPluggable *pluggable, const gchar *path);
static gchar *zak_confi_db_plugin_path_get_value_from_db (ZakConfiPluggable *pluggable, const gchar *path);
static void zak_confi_db_plugin_get_children (ZakConfiPluggable *pluggable, GNode *parentNode, gint idParent, gchar *path);

#define ZAK_CONFI_DB_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ZAK_CONFI_TYPE_DB_PLUGIN, ZakConfiDBPluginPrivate))

typedef struct _ZakConfiDBPluginPrivate ZakConfiDBPluginPrivate;
struct _ZakConfiDBPluginPrivate
	{
		gchar *cnc_string;

		GdaEx *gdaex;

		gint id_config;
		gchar *name;
		gchar *description;
		gchar *root;

		gchar chrquot;
	};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (ZakConfiDBPlugin,
                                zak_confi_db_plugin,
                                PEAS_TYPE_EXTENSION_BASE,
                                0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (ZAK_CONFI_TYPE_PLUGGABLE,
                                                               zak_confi_pluggable_iface_init))

enum {
	PROP_0,
	PROP_CNC_STRING,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_ROOT
};

static void
zak_confi_db_plugin_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	gchar *sql;

	ZakConfiDBPlugin *plugin = ZAK_CONFI_DB_PLUGIN (object);
	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	switch (prop_id)
		{
			case PROP_CNC_STRING:
				zak_confi_db_plugin_initialize ((ZakConfiPluggable *)plugin, g_value_get_string (value));
				break;

			case PROP_NAME:
				break;

			case PROP_DESCRIPTION:
				break;

			case PROP_ROOT:
				priv->root = zak_confi_normalize_root (g_value_get_string (value));
				break;

			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
				break;
		}
}

static void
zak_confi_db_plugin_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	ZakConfiDBPlugin *plugin = ZAK_CONFI_DB_PLUGIN (object);
	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

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
zak_confi_db_plugin_init (ZakConfiDBPlugin *plugin)
{
	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	priv->cnc_string = NULL;
	priv->gdaex = NULL;
	priv->name = NULL;
	priv->description = NULL;
}

static void
zak_confi_db_plugin_finalize (GObject *object)
{
	ZakConfiDBPlugin *plugin = ZAK_CONFI_DB_PLUGIN (object);

	G_OBJECT_CLASS (zak_confi_db_plugin_parent_class)->finalize (object);
}

static gboolean
zak_confi_db_plugin_initialize (ZakConfiPluggable *pluggable, const gchar *cnc_string)
{
	ZakConfiDBPlugin *plugin = ZAK_CONFI_DB_PLUGIN (pluggable);
	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (plugin);

	GString *gstr_cnc_string;
	gchar **strs;
	guint i;
	guint l;

	gchar *sql;
	GdaDataModel *dm;

	gchar *cnc_string_;

	cnc_string_ = g_strdup_printf ("%s;", cnc_string);
	strs = g_strsplit (cnc_string_, ";", -1);
	g_free (cnc_string_);

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
*zak_confi_db_plugin_path_get_data_model (ZakConfiPluggable *pluggable, const gchar *path)
{
	gchar **tokens;
	gchar *sql;
	gchar *token;
	guint i;
	guint id_parent;
	GdaDataModel *dm;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

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
*zak_confi_db_plugin_path_get_value_from_db (ZakConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;
	GdaDataModel *dm;

	ret = NULL;

	dm = zak_confi_db_plugin_path_get_data_model (pluggable, path);
	if (dm != NULL)
		{
			ret = gdaex_data_model_get_field_value_stringify_at (dm, 0, "value");
			g_object_unref (dm);
		}

	return ret;
}

static void
zak_confi_db_plugin_get_children (ZakConfiPluggable *pluggable, GNode *parentNode, gint idParent, gchar *path)
{
	gchar *sql;
	GdaDataModel *dm;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	sql = g_strdup_printf ("SELECT *"
	                       " FROM %cvalues%c"
	                       " WHERE id_configs = %d"
	                       " AND id_parent = %d",
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
					ZakConfiKey *ck = g_new0 (ZakConfiKey, 1);

					ck->config = g_strdup (priv->name);
					ck->key = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "key"));
					ck->value = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "value"));
					ck->description = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "description"));
					ck->path = g_strdup (path);

					newNode = g_node_append_data (parentNode, ck);

					zak_confi_db_plugin_get_children (pluggable,
													  newNode,
													  gdaex_data_model_get_field_value_integer_at (dm, i, "id"),
													  g_strconcat (path, (g_strcmp0 (path, "") == 0 ? "" : "/"), ck->key, NULL));
				}
			g_object_unref (dm);
		}
}

static GList
*zak_confi_db_plugin_get_configs_list (ZakConfiPluggable *pluggable,
                                   const gchar *filter)
{
	GList *lst;

	GdaDataModel *dmZakConfigs;

	gchar *sql;
	gchar *where;

	guint row;
	guint rows;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

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

	dmZakConfigs = gdaex_query (priv->gdaex, sql);
	g_free (sql);
	if (dmZakConfigs != NULL)
		{
			rows = gda_data_model_get_n_rows (dmZakConfigs);
			if (rows > 0)
				{
					for (row = 0; row < rows; row++)
						{
							ZakConfiConfi *confi;
							confi = g_new0 (ZakConfiConfi, 1);
							confi->name = gdaex_data_model_get_field_value_stringify_at (dmZakConfigs, row, "name");
							confi->description = gdaex_data_model_get_field_value_stringify_at (dmZakConfigs, row, "description");
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
*zak_confi_db_plugin_path_get_value (ZakConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;
	gchar *path_;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	ret = NULL;

	path_ = zak_confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	ret = zak_confi_db_plugin_path_get_value_from_db (pluggable, path_);

	return ret;
}

static gboolean
zak_confi_db_plugin_path_set_value (ZakConfiPluggable *pluggable, const gchar *path, const gchar *value)
{
	GdaDataModel *dm;
	gchar *sql;
	gboolean ret;

	dm = zak_confi_db_plugin_path_get_data_model (pluggable, zak_confi_path_normalize (pluggable, path));

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	ret = FALSE;
	if (dm != NULL && gda_data_model_get_n_rows (dm) > 0)
		{
			sql = g_strdup_printf ("UPDATE %cvalues%c"
			                       " SET value = '%s'"
			                       " WHERE id_configs = %d"
			                       " AND id = %d",
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
*zak_confi_db_plugin_get_tree (ZakConfiPluggable *pluggable)
{
	gchar *path;
	GNode *node;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	path = g_strdup ("");

	ZakConfiKey *ck = g_new0 (ZakConfiKey, 1);

	ck->config = g_strdup (priv->name);
	ck->path = "";
	ck->key = g_strdup ("/");
	ck->value = "";
	ck->description = "";

	node = g_node_new (ck);

	zak_confi_db_plugin_get_children (pluggable, node, 0, path);

	return node;
}

static ZakConfiConfi
*zak_confi_db_plugin_add_config (ZakConfiPluggable *pluggable, const gchar *name, const gchar *description)
{
	ZakConfiConfi *cc;

	gchar *sql;
	gint id;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	cc = NULL;

	id = gdaex_get_new_id (priv->gdaex, "configs", "id", NULL);

	sql = g_strdup_printf ("INSERT INTO configs"
						   " VALUES (%d, '%s', '%s')",
						   id,
						   gdaex_strescape (name, NULL),
						   gdaex_strescape (description, NULL));
	if (gdaex_execute (priv->gdaex, sql) > 0)
		{
			cc = g_new0 (ZakConfiConfi, 1);
			cc->name = g_strdup (name);
			cc->description = g_strdup (description);
		}

	g_free (sql);

	return cc;
}

static gboolean
zak_confi_db_plugin_set_config (ZakConfiPluggable *pluggable,
                                   const gchar *name,
                                   const gchar *description)
{
	gboolean ret;
	gchar *sql;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	g_return_val_if_fail (name != NULL, FALSE);

	ret = TRUE;

	sql = g_strdup_printf ("UPDATE configs"
						   " SET name = '%s'"
						   " WHERE id = %d",
						   gdaex_strescape (name, NULL),
						   priv->id_config);
	if (gdaex_execute (priv->gdaex, sql) < 1)
		{
			ret = FALSE;
		}
	g_free (sql);
	priv->name = g_strdup (name);

	if (description != NULL)
		{
			sql = g_strdup_printf ("UPDATE configs"
								   " SET description = '%s'"
								   " WHERE id = %d",
								   gdaex_strescape (description, NULL),
								   priv->id_config);
			if (gdaex_execute (priv->gdaex, sql) < 1)
				{
					ret = FALSE;
				}
			g_free (sql);
			priv->description = g_strdup (description);
		}

	return ret;
}

static ZakConfiKey
*zak_confi_db_plugin_add_key (ZakConfiPluggable *pluggable, const gchar *parent, const gchar *key, const gchar *value)
{
	ZakConfiKey *ck;
	GdaDataModel *dmParent;

	gchar *sql;
	gint id;
	GdaDataModel *dm;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

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
					dmParent = zak_confi_db_plugin_path_get_data_model (pluggable, zak_confi_path_normalize (pluggable, parent_));
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
			key_ = g_strdup (key);
			g_strstrip (key_);

			/* find if key exists */
			sql = g_strdup_printf ("SELECT id"
			                       " FROM %cvalues%c"
			                       " WHERE id_configs = %d"
			                       " AND key = '%s'",
			                       priv->chrquot, priv->chrquot,
			                       priv->id_config,
			                       gdaex_strescape (key_, NULL));
			dm = gdaex_query (priv->gdaex, sql);
			g_free (sql);
			if (dm != NULL && gda_data_model_get_n_rows (dm) > 0)
				{
					id = gdaex_data_model_get_value_integer_at (dm, 0, 0);
					g_object_unref (dm);

					sql = g_strdup_printf ("UPDATE %cvalues%c"
					                       " SET %ckey%c = '%s',"
					                       " value = '%s',"
					                       " WHERE id_configs = %d"
					                       " AND id = %d",
					                       priv->chrquot, priv->chrquot,
					                       priv->chrquot, priv->chrquot,
					                       gdaex_strescape (key_, NULL),
					                       gdaex_strescape (ck->value, NULL),
					                       priv->id_config,
					                       id);
					if (gdaex_execute (priv->gdaex, sql) == -1)
						{
							/* TO DO */
							g_free (sql);
							return NULL;
						}
					g_free (sql);
				}
			else
				{
					id = 0;

					/* find new id */
					sql = g_strdup_printf ("SELECT MAX(id)"
					                       " FROM %cvalues%c"
					                       " WHERE id_configs = %d",
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

					sql = g_strdup_printf ("INSERT INTO %cvalues%c"
					                       " (id_configs, id, id_parent, %ckey%c, value)"
					                       " VALUES (%d, %d, %d, '%s', '%s')",
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
				}

			ck = g_new0 (ZakConfiKey, 1);
			ck->config = g_strdup (priv->name);
			if (id_parent == 0)
				{
					ck->path = g_strdup ("");
				}
			else
				{
					ck->path = g_strdup (parent_);
				}
			ck->key = g_strdup (key_);
			ck->value = g_strdup ("");
			ck->description = g_strdup ("");

			g_free (key_);
		}
	g_free (parent_);

	if (ck != NULL)
		{
			if (g_strcmp0 (ck->path, "") != 0)
				{
					path = g_strconcat (ck->path, "/", key, NULL);
				}
			else
				{
					path = g_strdup (ck->key);
				}

			if (!zak_confi_db_plugin_path_set_value (pluggable, path, value))
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

static gboolean
zak_confi_db_plugin_key_set_key (ZakConfiPluggable *pluggable,
                             ZakConfiKey *ck)
{
	gboolean ret;
	gchar *sql;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	sql = g_strdup_printf ("UPDATE %cvalues%c"
	                       " SET %ckey%c = '%s',"
	                       " value = '%s',"
	                       " description = '%s'"
	                       " WHERE id_configs = %d"
	                       " AND id = (SELECT id FROM %cconfigs%c WHERE name = '%s')",
	                       priv->chrquot, priv->chrquot,
	                       priv->chrquot, priv->chrquot,
	                       gdaex_strescape (ck->key, NULL),
	                       gdaex_strescape (ck->value, NULL),
	                       gdaex_strescape (ck->description, NULL),
	                       priv->id_config,
	                       priv->chrquot, priv->chrquot,
	                       ck->config);

	ret = (gdaex_execute (priv->gdaex, sql) >= 0);
	g_free (sql);

	return ret;
}

static ZakConfiKey
*zak_confi_db_plugin_path_get_confi_key (ZakConfiPluggable *pluggable, const gchar *path)
{
	GdaDataModel *dm;
	gchar *path_;
	ZakConfiKey *ck;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	path_ = zak_confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	dm = zak_confi_db_plugin_path_get_data_model (pluggable, path_);
	if (dm == NULL || gda_data_model_get_n_rows (dm) <= 0)
		{
			if (dm != NULL)
				{
					g_object_unref (dm);
				}
			return NULL;
		}

	ck = g_new0 (ZakConfiKey, 1);
	ck->config = g_strdup (priv->name);
	ck->path = g_strdup (path_);
	ck->key = gdaex_data_model_get_field_value_stringify_at (dm, 0, "key");
	ck->value = gdaex_data_model_get_field_value_stringify_at (dm, 0, "value");
	ck->description = gdaex_data_model_get_field_value_stringify_at (dm, 0, "description");

	if (dm != NULL)
		{
			g_object_unref (dm);
		}

	return ck;
}

static gboolean
zak_confi_db_plugin_delete_id_from_db_values (ZakConfiPluggable *pluggable, gint id)
{
	gboolean ret;
	gchar *sql;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	sql = g_strdup_printf ("DELETE FROM %cvalues%c"
	                       " WHERE id_configs = %d"
	                       " AND id = %d",
	                       priv->chrquot, priv->chrquot,
	                       priv->id_config,
	                       id);

	if (gdaex_execute (priv->gdaex, sql) >= 0)
		{
			ret = TRUE;
		}
	else
		{
			ret = FALSE;
		}
	g_free (sql);

	return ret;
}

static gboolean
zak_confi_db_plugin_remove_path_traverse_func (GNode *node, gpointer data)
{
	GdaDataModel *dm;
	ZakConfiPluggable* pluggable;

	pluggable = (ZakConfiPluggable *)data;
	ZakConfiKey *ck = (ZakConfiKey *)node->data;
	if (g_strcmp0 (ck->value, "") != 0)
		{
			dm = zak_confi_db_plugin_path_get_data_model (pluggable, g_strdup_printf ("%s/%s", ck->path, ck->key));
			zak_confi_db_plugin_delete_id_from_db_values (pluggable, gdaex_data_model_get_field_value_integer_at (dm, 0, "id"));
		}

	return FALSE;
}

static gboolean
zak_confi_db_plugin_remove_path (ZakConfiPluggable *pluggable, const gchar *path)
{
	gboolean ret = FALSE;
	GdaDataModel *dm;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	dm = zak_confi_db_plugin_path_get_data_model (pluggable, zak_confi_path_normalize (pluggable, path));

	if (dm != NULL && gda_data_model_get_n_rows (dm) > 0)
		{
			gchar *path_ = g_strdup (path);

			/* removing every child key */
			GNode *node, *root;
			gint id = gdaex_data_model_get_field_value_integer_at (dm, 0, "id");

			node = g_node_new (path_);
			zak_confi_db_plugin_get_children (pluggable, node, id, path_);

			root = g_node_get_root (node);

			if (g_node_n_nodes (root, G_TRAVERSE_ALL) > 1)
				{
					g_node_traverse (root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, zak_confi_db_plugin_remove_path_traverse_func, (gpointer)pluggable);
				}

			/* removing the path */
			ret = zak_confi_db_plugin_delete_id_from_db_values (pluggable, id);
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

static gboolean
zak_confi_db_plugin_remove (ZakConfiPluggable *pluggable)
{
	gboolean ret;
	gchar *sql;

	ZakConfiDBPluginPrivate *priv = ZAK_CONFI_DB_PLUGIN_GET_PRIVATE (pluggable);

	ret = TRUE;
	sql = g_strdup_printf ("DELETE FROM %cvalues%c WHERE id_configs = %d",
                           priv->chrquot,
                           priv->chrquot,
                           priv->id_config);
	if (gdaex_execute (priv->gdaex, sql) == -1)
		{
			g_free (sql);
			ret = FALSE;
		}
	else
		{
			g_free (sql);
			sql = g_strdup_printf ("DELETE FROM configs WHERE id = %d",
			                       priv->id_config);
			if (gdaex_execute (priv->gdaex, sql) == -1)
				{
					ret = FALSE;
				}
			g_free (sql);
		}

	return ret;
}

static void
zak_confi_db_plugin_class_init (ZakConfiDBPluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ZakConfiDBPluginPrivate));

	object_class->set_property = zak_confi_db_plugin_set_property;
	object_class->get_property = zak_confi_db_plugin_get_property;
	object_class->finalize = zak_confi_db_plugin_finalize;

	g_object_class_override_property (object_class, PROP_CNC_STRING, "cnc_string");
	g_object_class_override_property (object_class, PROP_NAME, "name");
	g_object_class_override_property (object_class, PROP_DESCRIPTION, "description");
	g_object_class_override_property (object_class, PROP_ROOT, "root");
}

static void
zak_confi_pluggable_iface_init (ZakConfiPluggableInterface *iface)
{
	iface->initialize = zak_confi_db_plugin_initialize;
	iface->get_configs_list = zak_confi_db_plugin_get_configs_list;
	iface->path_get_value = zak_confi_db_plugin_path_get_value;
	iface->path_set_value = zak_confi_db_plugin_path_set_value;
	iface->get_tree = zak_confi_db_plugin_get_tree;
	iface->add_config = zak_confi_db_plugin_add_config;
	iface->set_config = zak_confi_db_plugin_set_config;
	iface->add_key = zak_confi_db_plugin_add_key;
	iface->key_set_key = zak_confi_db_plugin_key_set_key;
	iface->path_get_confi_key = zak_confi_db_plugin_path_get_confi_key;
	iface->remove_path = zak_confi_db_plugin_remove_path;
	iface->remove = zak_confi_db_plugin_remove;
}

static void
zak_confi_db_plugin_class_finalize (ZakConfiDBPluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	zak_confi_db_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            ZAK_CONFI_TYPE_PLUGGABLE,
	                                            ZAK_CONFI_TYPE_DB_PLUGIN);
}
