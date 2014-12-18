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

#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <string.h>

#include <libgdaex/libgdaex.h>
#include <libpeas/peas.h>

#include "libconfi.h"


enum
{
	PROP_0,
	PROP_ID_CONFIG,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_ROOT
};

static void confi_class_init (ConfiClass *klass);
static void confi_init (Confi *confi);

static void confi_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec);
static void confi_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec);

static ConfiPluggable *confi_get_confi_pluggable_from_cnc_string (const gchar *cnc_string);

static gboolean confi_delete_id_from_db_values (Confi *confi, gint id);
static gboolean confi_remove_path_traverse_func (GNode *node, gpointer data);

#define CONFI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), TYPE_CONFI, ConfiPrivate))

typedef struct _ConfiPrivate ConfiPrivate;
struct _ConfiPrivate
	{
		GdaEx *gdaex;
		gint id_config;
		gchar *name;
		gchar *description;
		gchar *root;
		GHashTable *values;

		gchar chrquot;

		ConfiPluggable *pluggable;
	};

G_DEFINE_TYPE (Confi, confi, G_TYPE_OBJECT)

static void
confi_class_init (ConfiClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ConfiPrivate));

	object_class->set_property = confi_set_property;
	object_class->get_property = confi_get_property;
}

static void
confi_init (Confi *confi)
{
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	priv->pluggable = NULL;
}

static ConfiPluggable
*confi_get_confi_pluggable_from_cnc_string (const gchar *cnc_string)
{
	ConfiPluggable *pluggable;

	const GList *lst_plugins;

	pluggable = NULL;

	PeasEngine *peas_engine;

	peas_engine = peas_engine_get_default ();
	if (peas_engine == NULL)
		{
			return NULL;
		}

	peas_engine_add_search_path (peas_engine, PLUGINSDIR, NULL);

	lst_plugins = peas_engine_get_plugin_list (peas_engine);
	while (lst_plugins)
		{
			PeasPluginInfo *ppinfo;
			gchar *uri;

			ppinfo = (PeasPluginInfo *)lst_plugins->data;

			uri = g_strdup_printf ("%s://", peas_plugin_info_get_module_name (ppinfo));
			if (g_str_has_prefix (cnc_string, uri))
				{
					if (peas_engine_load_plugin (peas_engine, ppinfo))
						{
							PeasExtension *ext;
							ext = peas_engine_create_extension (peas_engine, ppinfo, CONFI_TYPE_PLUGGABLE,
							                                    "cnc_string", cnc_string + strlen (uri),
							                                    NULL);
							pluggable = (ConfiPluggable *)ext;
							break;
						}
				}
			g_free (uri);

			lst_plugins = g_list_next (lst_plugins);
		}

	if (pluggable == NULL)
		{
			g_warning ("No plugin found for connection string \"%s\".", cnc_string);
		}

	return pluggable;
}

/**
 * confi_new:
 * @cnc_string: the connection string.
 *
 * Returns: (transfer none): the newly created #Confi object, or NULL if it fails.
 */
Confi
*confi_new (const gchar *cnc_string)
{
	Confi *confi;
	ConfiPrivate *priv;
	ConfiPluggable *pluggable;

	g_return_val_if_fail (cnc_string != NULL, NULL);

	confi = NULL;

	pluggable = confi_get_confi_pluggable_from_cnc_string (cnc_string);
	if (pluggable != NULL)
		{
			confi = CONFI (g_object_new (confi_get_type (), NULL));
			priv = CONFI_GET_PRIVATE (confi);
			priv->pluggable = pluggable;
		}

	return confi;
}

/**
 * confi_get_configs_list:
 * @cnc_string: the connection string to use to connect to database that
 * contains configuration.
 * @filter: (nullable):
 *
 * Returns: (element-type Confi) (transfer container):  a #GList of #ConfiConfi. If there's no configurations, returns a valid
 * #GList but with a unique NULL element.
 */
GList
*confi_get_configs_list (const gchar *cnc_string,
                         const gchar *filter)
{
	ConfiPluggable *pluggable;
	GList *lst;

	lst = NULL;

	pluggable = confi_get_confi_pluggable_from_cnc_string (cnc_string);

	if (pluggable != NULL)
		{
			lst = confi_pluggable_get_configs_list (pluggable, filter);
		}

	return lst;
}

/**
 * confi_get_tree:
 * @confi: a #Confi object.
 *
 * Returns: a #GNode.
 */
GNode
*confi_get_tree (Confi *confi)
{
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (priv->pluggable != NULL)
		{
			return confi_pluggable_get_tree (priv->pluggable);
		}
	else
		{
			return NULL;
		}
}

/**
 * confi_normalize_set_root:
 * @confi: a #Confi object.
 * @root: the root.
 *
 * Returns: a correct value for root property.
 */
gchar
*confi_normalize_root (const gchar *root)
{
	GString *root_;
	gchar *strret;

	if (root == NULL)
		{
			root_ = g_string_new ("/");
		}
	else
		{
			root_ = g_string_new (root);
			g_strstrip (root_->str);
			if (g_strcmp0 (root_->str, "") == 0)
				{
					g_string_printf (root_, "/");
				}
			else
				{
					if (root_->str[0] != '/')
						{
							g_string_prepend (root_, "/");
						}
					if (root_->str[root_->len - 1] != '/')
						{
							g_string_append (root_, "/");
						}
				}
		}

	strret = g_strdup (root_->str);
	g_string_free (root_, TRUE);

	return strret;
}

/**
 * confi_set_root:
 * @confi: a #Confi object.
 * @root: the root.
 *
 */
gboolean
confi_set_root (Confi *confi, const gchar *root)
{
	gboolean ret;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = FALSE;
		}
	else
		{
			g_object_set (priv->pluggable, "root", root, NULL);
			ret = TRUE;
		}

	return ret;
}

/**
 * confi_add_key:
 * @confi: a #Confi object.
 * @parent: the path where add the key.
 * @key: the key's name.
 * @value: the key's value.
 *
 * Returns: a #ConfigKey struct filled with data from the key just added.
 */
ConfiKey
*confi_add_key (Confi *confi, const gchar *parent, const gchar *key, const gchar *value)
{
	ConfiKey *ck;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ck = NULL;
		}
	else
		{
			ck = confi_pluggable_add_key (priv->pluggable, parent, key, value);
		}

	return ck;
}

/**
 * confi_key_set_key:
 * @confi: a #Confi object.
 * @ck: a #ConfiKey struct.
 *
 */
gboolean
confi_key_set_key (Confi *confi,
                   ConfiKey *ck)
{
	gboolean ret;
	gchar *sql;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	sql = g_strdup_printf ("UPDATE %cvalues%c SET "
	                              "%ckey%c = '%s', value = '%s', description = '%s' "
	                              "WHERE id_configs = %d "
	                              "AND id = %d",
	                              priv->chrquot, priv->chrquot,
	                              priv->chrquot, priv->chrquot,
	                              gdaex_strescape (ck->key, NULL),
	                              gdaex_strescape (ck->value, NULL),
	                              gdaex_strescape (ck->description, NULL),
	                              priv->id_config,
	                              ck->id);

	ret = (gdaex_execute (priv->gdaex, sql) >= 0);
	g_free (sql);

	return ret;
}

/**
 * confi_remove_path:
 * @confi: a #Confi object.
 * @path: the path to remove.
 *
 * Removes @path and every child key.
 */
gboolean
confi_remove_path (Confi *confi, const gchar *path)
{
	gboolean ret = FALSE;
	GdaDataModel *dm;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	//dm = path_get_data_model (confi, path_normalize (confi, path));

	if (dm != NULL && gda_data_model_get_n_rows (dm) > 0)
		{
			gchar *path_ = g_strdup (path);
		
			/* removing every child key */
			GNode *node, *root;
			gint id = gdaex_data_model_get_field_value_integer_at (dm, 0, "id");

			node = g_node_new (path_);
			get_children (confi, node, id, path_);

			root = g_node_get_root (node);
		
			if (g_node_n_nodes (root, G_TRAVERSE_ALL) > 1)
				{
					g_node_traverse (root, G_PRE_ORDER, G_TRAVERSE_ALL, -1, confi_remove_path_traverse_func, (gpointer)confi);
				}

			/* removing the path */
			ret = confi_delete_id_from_db_values (confi, id);
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

/**
 * confi_path_get_value:
 * @confi: a #Confi object.
 * @path: the path from which retrieving the value.
 *
 * Returns: the configuration's value as a string.
 */
gchar
*confi_path_get_value (Confi *confi, const gchar *path)
{
	gchar *ret;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = NULL;
		}
	else
		{
			ret = confi_pluggable_path_get_value (priv->pluggable, path);
		}

	return ret;
}

/**
 * confi_path_set_value:
 * @confi: a #Confi object.
 * @path: the key's path.
 * @value: the value to set.
 *
 */
gboolean
confi_path_set_value (Confi *confi, const gchar *path, const gchar *value)
{
	gboolean ret;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = FALSE;
		}
	else
		{
			ret = confi_pluggable_path_set_value (priv->pluggable, path, value);
		}

	return ret;
}

/**
 * confi_path_move:
 * @confi: a #Confi object.
 * @path: the key's path to move.
 * @parent: the path where add the key.
 *
 */
gboolean
confi_path_move (Confi *confi, const gchar *path, const gchar *parent)
{
	GdaDataModel *dmPath;
	GdaDataModel *dmParent;

	gboolean ret;
	gchar *sql;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	//dmPath = path_get_data_model (confi, path_normalize (confi, path));
	if (dmPath == NULL) return FALSE;

	//dmParent = path_get_data_model (confi, path_normalize (confi, parent));
	if (dmParent == NULL) return FALSE;

	ret = TRUE;
	sql = g_strdup_printf ("UPDATE %cvalues%c "
	                              "SET id_parent = %d "
	                              "WHERE id_configs = %d "
	                              "AND id = %d",
	                              priv->chrquot, priv->chrquot,
	                              gdaex_data_model_get_field_value_integer_at (dmParent, 0, "id"),
	                              priv->id_config,
	                              gdaex_data_model_get_field_value_integer_at (dmPath, 0, "id"));

	ret = (gdaex_execute (priv->gdaex, sql) >= 0);
	g_free (sql);

	return ret;
}

/**
 * confi_path_get_confi_key:
 * @confi: a #Confi object.
 * @path: the key's path to get.
 *
 * Returns: (transfer full): a #ConfiKey from @path
 */
ConfiKey
*confi_path_get_confi_key (Confi *confi, const gchar *path)
{
	ConfiKey *ck;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ck = NULL;
		}
	else
		{
			ck = confi_pluggable_path_get_confi_key (priv->pluggable, path);
		}

	return ck;
}

/**
 * confi_remove:
 * @confi: a #Confi object.
 *
 * Remove a configuration from databases and destroy the relative object.
 */
gboolean
confi_remove (Confi *confi)
{
	gboolean ret;
	gchar *sql;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

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
			else
				{
					confi_destroy (confi);
				}
		}

	return ret;
}

/**
 * confi_destroy:
 * @confi: a #Confi object.
 *
 * Destroy the #Confi object, freeing memory.
 */
void
confi_destroy (Confi *confi)
{
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	g_free (priv->name);
	g_free (priv->description);
	g_free (priv->root);
}

/**
 * confi_path_normalize:
 * @pluggable: a #ConfiPluggable object.
 *
 * Returns: a normalize path (with root).
 */
gchar
*confi_path_normalize (ConfiPluggable *pluggable, const gchar *path)
{
	GString *ret;
	gchar *strret;

	guint lead;

	gchar *root;

	if (path == NULL)
		{
			return NULL;
		}

	ret = g_string_new (path);
	g_strstrip (ret->str);
	if (g_strcmp0 (ret->str, "") == 0)
		{
			return NULL;
		}
	else if (ret->str[strlen (ret->str) - 1] == '/')
		{
			return NULL;
		}

	/* removing leading '/' */
	lead = 0;
	for (lead = 0; lead < ret->len; lead++)
		{
			if (ret->str[lead] != '/')
				{
					break;
				}
		}

	if (lead < ret->len)
		{
			g_string_erase (ret, 0, lead++);
		}

	g_object_get (pluggable, "root", &root, NULL);
	g_string_prepend (ret, root);

	strret = g_strdup (ret->str);
	g_string_free (ret, TRUE);

	return strret;
}

/* PRIVATE */
static void
confi_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	gchar *sql;

	Confi *confi = CONFI (object);
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
confi_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	Confi *confi = CONFI (object);
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static gboolean
confi_delete_id_from_db_values (Confi *confi, gint id)
{
	gboolean ret;
	gchar *sql;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	sql = g_strdup_printf ("DELETE FROM %cvalues%c "
	                              "WHERE id_configs = %d "
	                              "AND id = %d",
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
confi_remove_path_traverse_func (GNode *node, gpointer data)
{
	ConfiKey *ck = (ConfiKey *)node->data;
	if (ck->id != 0)
		{
			confi_delete_id_from_db_values ((Confi *)data, ck->id);
		}

	return FALSE;
}
