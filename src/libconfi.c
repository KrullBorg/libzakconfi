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

#include "libconfi.h"


ConfiKey
*confi_key_copy (ConfiKey *key)
{
	ConfiKey *b;

	b = g_slice_new (ConfiKey);
	b->id_config = key->id_config;
	b->id = key->id;
	b->id_parent = key->id_parent;
	b->key = g_strdup (key->key);
	b->value = g_strdup (key->value);
	b->description = g_strdup (key->description);
	b->path = g_strdup (key->path);

	return b;
}

void
confi_key_free (ConfiKey *key)
{
	g_free (key->key);
	g_free (key->value);
	g_free (key->description);
	g_free (key->path);
	g_slice_free (ConfiKey, key);
}

G_DEFINE_BOXED_TYPE (ConfiKey, confi_key, confi_key_copy, confi_key_free)


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

static gchar *path_normalize (Confi *confi, const gchar *path);
static GdaDataModel *path_get_data_model (Confi *confi, const gchar *path);
static gchar *path_get_value_from_db (Confi *confi, const gchar *path);
static void get_children (Confi *confi, GNode *parentNode, gint idParent, gchar *path);
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
	};

G_DEFINE_TYPE (Confi, confi, G_TYPE_OBJECT)

static void
confi_class_init (ConfiClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ConfiPrivate));

	object_class->set_property = confi_set_property;
	object_class->get_property = confi_get_property;

	g_object_class_install_property (object_class, PROP_ID_CONFIG,
	                                 g_param_spec_int ("id_config",
	                                                   "Configuraton ID",
	                                                   "The configuration ID",
	                                                   0, G_MAXINT, 0,
	                                                   G_PARAM_READABLE));
	g_object_class_install_property (object_class, PROP_NAME,
	                                 g_param_spec_string ("name",
	                                                      "Configuraton Name",
	                                                      "The configuration name",
	                                                      "",
	                                                      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_DESCRIPTION,
	                                 g_param_spec_string ("description",
	                                                      "Configuraton Description",
	                                                      "The configuration description",
	                                                      "",
	                                                      G_PARAM_READWRITE));
	g_object_class_install_property (object_class, PROP_ROOT,
	                                 g_param_spec_string ("root",
	                                                      "Configuraton Root",
	                                                      "The configuration root",
	                                                      "/",
	                                                      G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
}

static void
confi_init (Confi *confi)
{
}

/**
 * confi_new:
 * @cnc_string: the connection string to use to connect to database that
 * contains configuration.
 * @name: configuration's name.
 * @root: (nullable):
 * @create: whether create a config into database if @name doesn't exists.
 *
 * Returns: (transfer none): the newly created #Confi object, or NULL if it fails.
 */
Confi
*confi_new (const gchar *cnc_string,
            const gchar *name,
            const gchar *root,
            gboolean create)
{
	if (name == NULL) return NULL;

	GdaDataModel *dm;
	gchar *sql;
	gint id = 0;

	Confi *confi = CONFI (g_object_new (confi_get_type (), NULL));

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	priv->gdaex = gdaex_new_from_string (cnc_string);
	if (priv->gdaex == NULL)
		{
			/* TO DO */
			return NULL;
		}

	priv->chrquot = gdaex_get_chr_quoting (priv->gdaex);

	/* check if config exists */
	sql = g_strdup_printf ("SELECT id, name FROM configs WHERE name = '%s'",
	                       gdaex_strescape (name, NULL));
	dm = gdaex_query (priv->gdaex, sql);
	if (dm == NULL || gda_data_model_get_n_rows (dm) == 0)
		{
			if (create)
				{
						/* saving a new config into database */
						if (dm != NULL)
							{
								g_object_unref (dm);
							}

						dm = gdaex_query (priv->gdaex, "SELECT MAX(id) FROM configs");
						if (dm != NULL)
							{
								id = gdaex_data_model_get_value_integer_at (dm, 0, 0);
							}
						id++;

						if (gdaex_execute (priv->gdaex, g_strdup_printf ("INSERT INTO configs "
							                                             "(id, name, description) "
							                                             "VALUES (%d, '%s', '')",
							                                             id, gdaex_strescape (name, NULL))) == -1)
							{
								return NULL;
							}
				}
			else
				{
					/* TO DO */
					return NULL;
				}
		}
	else
		{
			id = gdaex_data_model_get_value_integer_at (dm, 0, 0);
		}

	g_object_set (G_OBJECT (confi),
	              "name", name,
	              "root", root,
	              NULL);
	priv->id_config = id;
	priv->values = g_hash_table_new (g_str_hash, g_str_equal);

	if (dm != NULL)
		{
			g_object_unref (dm);
		}

	return confi;
}

/**
 * confi_get_configs_list:
 * @cnc_string: the connection string to use to connect to database that
 * contains configuration.
 * @filter: (nullable):
 *
 * Returns: (element-type Confi) (transfer container):  a #GList of #Confi. If there's no configurations, returns a valid
 * #GList but with a unique NULL element.
 */
GList
*confi_get_configs_list (const gchar *cnc_string,
                         const gchar *filter)
{
	GList *lst = NULL;
	gchar *sql;
	gchar *where = "";

	GdaEx *gdaex = gdaex_new_from_string (cnc_string);

	if (gdaex == NULL)
		{
			return NULL;
		}

	if (filter != NULL && strcmp (g_strstrip (g_strdup (filter)), "") != 0)
		{
			where = g_strdup_printf (" WHERE name LIKE '%s'", filter);
		}

	sql = g_strdup_printf ("SELECT * FROM configs%s", where);

	GdaDataModel *dmConfigs = gdaex_query (gdaex, sql);
	if (dmConfigs != NULL)
		{
			gint row, id,
			     rows = gda_data_model_get_n_rows (dmConfigs);
			Confi *confi;
			ConfiPrivate *priv;

			if (rows > 0)
				{
					for (row = 0; row < rows; row++)
						{
							confi = confi_new (cnc_string,
							                   gdaex_data_model_get_field_value_stringify_at (dmConfigs, row, "name"),
							                   NULL, FALSE);

							priv = CONFI_GET_PRIVATE (confi);
							priv->id_config = gdaex_data_model_get_field_value_integer_at (dmConfigs, row, "id");

							g_object_set (G_OBJECT (confi),
							              "description", gdaex_data_model_get_field_value_stringify_at (dmConfigs, row, "description"),
							              NULL);

							lst = g_list_append (lst, confi);
						}
				}
			else
				{
					lst = g_list_append (lst, NULL);
				}
		}

	gdaex_free (gdaex);

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
	gchar *path = "";
	GNode *node;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	ConfiKey *ck = (ConfiKey *)g_malloc0 (sizeof (ConfiKey));

	ck->id_config = priv->id_config;
	ck->id = 0;
	ck->id_parent = 0;
	ck->key = g_strdup ("/");

	node = g_node_new (ck);

	get_children (confi, node, 0, path);

	return node;
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
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	gchar *root_;

	if (root == NULL)
		{
			root_ = g_strdup ("/");
		}
	else
		{
			root_ = g_strstrip (g_strdup (root));
			if (strcmp (root_, "") == 0)
				{
					root_ = g_strdup ("/");
				}
			else
				{
					if (root_[0] != '/')
						{
							root_ = g_strconcat ("/", root_, NULL);
						}
					if (root_[strlen (root_) - 1] != '/')
						{
							root_ = g_strconcat (root_, "/", NULL);
						}
				}
		}

	priv->root = root_;

	return TRUE;
}

/**
 * confi_add_key:
 * @confi: a #Confi object.
 * @parent: the path where add the key.
 * @key: the key's name.
 *
 * Returns: a #ConfigKey struct filled with data from the key just added.
 */
ConfiKey
*confi_add_key (Confi *confi, const gchar *parent, const gchar *key)
{
	ConfiKey *ck = NULL;
	GdaDataModel *dmParent;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	gint id_parent;
	gchar *parent_ = g_strstrip (g_strdup (parent)),
	      *key_ = g_strstrip (g_strdup (key));
	if (strcmp (parent_, "") == 0)
		{
			id_parent = 0;
		}
	else
		{
			dmParent = path_get_data_model (confi, path_normalize (confi, parent_));
			if (dmParent == NULL)
				{
					id_parent = -1;
				}
			else
				{
					id_parent = gdaex_data_model_get_field_value_integer_at (dmParent, 0, "id");
				}
		}

	if (id_parent > -1)
		{
			gchar *sql;
			gint id = 0;
			GdaDataModel *dm;

			/* find new id */
			sql = g_strdup_printf ("SELECT MAX(id) FROM %cvalues%c "
			                       "WHERE id_configs = %d ",
			                       priv->chrquot, priv->chrquot,
			                       priv->id_config);
			dm = gdaex_query (priv->gdaex, sql);
			if (dm != NULL)
				{
					id = gdaex_data_model_get_value_integer_at (dm, 0, 0);
					g_object_unref (dm);
				}
			id++;

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
					return NULL;
				}

			ck = (ConfiKey *)g_malloc0 (sizeof (ConfiKey));
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
		}

	return ck;
}

/**
 * confi_add_key_with_value:
 * @confi: a #Confi object.
 * @parent: the path where add the key.
 * @key: the key's name.
 * @value: the key's value.
 *
 * Returns: a #ConfigKey struct filled with data from the key just added.
 */
ConfiKey
*confi_add_key_with_value (Confi *confi, const gchar *parent, const gchar *key, const gchar *value)
{
	ConfiKey *ck = confi_add_key (confi, parent, key);

	if (ck != NULL)
		{
			gchar *path = "";
			if (ck->id_parent != 0)
				{
					path = g_strconcat (ck->path, "/", key, NULL);
				}
			else
				{
					path = ck->key;
				}

			if (!confi_path_set_value (confi, path, value))
				{
					ck = NULL;
				}
			else
				{
					ck->value = g_strdup (value);
				}
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
	gboolean ret = FALSE;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	gchar *sql = g_strdup_printf ("UPDATE %cvalues%c SET "
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

	dm = path_get_data_model (confi, path_normalize (confi, path));

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
	gchar *path_;
	gpointer *gp;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	ret = NULL;

	path_ = path_normalize (confi, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	gp = g_hash_table_lookup (priv->values, (gconstpointer)path_);

	if (gp == NULL)
		{
			/* load value from db if path exists */
			ret = path_get_value_from_db (confi, path_);

			if (ret != NULL)
				{
					/* and insert it on values */
					g_hash_table_insert (priv->values, (gpointer)path_, (gpointer)ret);
				}
			else
				{
					/* TO DO */
				}
		}
	else
		{
			ret = g_strdup ((gchar *)gp);
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
	gboolean ret = FALSE;
	GdaDataModel *dm = path_get_data_model (confi, path_normalize (confi, path));

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (dm != NULL && gda_data_model_get_n_rows (dm) > 0)
		{
			gchar *sql = g_strdup_printf ("UPDATE %cvalues%c SET value = '%s' "
			                              "WHERE id_configs = %d "
			                              "AND id = %d ",
			                              priv->chrquot, priv->chrquot,
			                              gdaex_strescape (value, NULL),
			                              priv->id_config,
			                              gdaex_data_model_get_field_value_integer_at (dm, 0, "id"));
			ret = (gdaex_execute (priv->gdaex, sql) >= 0);
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
 * confi_path_move:
 * @confi: a #Confi object.
 * @path: the key's path to move.
 * @parent: the path where add the key.
 *
 */
gboolean
confi_path_move (Confi *confi, const gchar *path, const gchar *parent)
{
	gboolean ret = TRUE;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	GdaDataModel *dmPath = path_get_data_model (confi, path_normalize (confi, path));
	if (dmPath == NULL) return FALSE;

	GdaDataModel *dmParent = path_get_data_model (confi, path_normalize (confi, parent));
	if (dmParent == NULL) return FALSE;

	gchar *sql = g_strdup_printf ("UPDATE %cvalues%c "
	                              "SET id_parent = %d "
	                              "WHERE id_configs = %d "
	                              "AND id = %d",
	                              priv->chrquot, priv->chrquot,
	                              gdaex_data_model_get_field_value_integer_at (dmParent, 0, "id"),
	                              priv->id_config,
	                              gdaex_data_model_get_field_value_integer_at (dmPath, 0, "id"));

	ret = (gdaex_execute (priv->gdaex, sql) >= 0);

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
	gchar *path_;
	ConfiKey *ck;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	path_ = path_normalize (confi, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	GdaDataModel *dm = path_get_data_model (confi, path_);
	if (dm == NULL || gda_data_model_get_n_rows (dm) <= 0)
		{
			return NULL;
		}

	ck = (ConfiKey *)g_malloc0 (sizeof (ConfiKey));
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

/**
 * confi_remove:
 * @confi: a #Confi object.
 *
 * Remove a configuration from databases and destroy the relative object.
 */
gboolean
confi_remove (Confi *confi)
{
	gboolean ret = TRUE;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (gdaex_execute (priv->gdaex,
	                   g_strdup_printf ("DELETE FROM %cvalues%c WHERE id_configs = %d",
	                                   priv->chrquot, priv->chrquot,
	                                   priv->id_config)) == -1)
		{
			ret = FALSE;
		}
	else 
		{
			if (gdaex_execute (priv->gdaex,
			                   g_strdup_printf ("DELETE FROM configs WHERE id = %d",
			                                    priv->id_config)) == -1)
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

	gdaex_free (priv->gdaex);
	g_hash_table_destroy (priv->values);
	g_free (priv->name);
	g_free (priv->description);
	g_free (priv->root);
}

/* PRIVATE */
static gchar
*path_normalize (Confi *confi, const gchar *path)
{
	gchar *ret,
	      *lead;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (path == NULL)
		{
			return NULL;
		}

	ret = g_strstrip (g_strdup (path));
	if (strcmp (ret, "") == 0)
		{
			return NULL;
		}
	else if (ret[strlen (ret) - 1] == '/')
		{
			return NULL;
		}

	/* removing leading '/' */
	for (;;)
		{
			if (ret[0] == '/')
				{
					lead = g_strdup (ret + 1);
					ret = g_strchug (lead);
				}
			else
				{
					break;
				}
		}

	ret = g_strconcat (priv->root, ret, NULL);

	return ret;
}

static GdaDataModel
*path_get_data_model (Confi *confi, const gchar *path)
{
	gchar **tokens, *sql, *token;
	gint i = 0, id_parent = 0;
	GdaDataModel *dm = NULL;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	if (path == NULL) return NULL;

	tokens = g_strsplit (path, "/", 0);
	if (tokens == NULL) return NULL;

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
					if (dm == NULL || gda_data_model_get_n_rows (dm) != 1)
						{
							/* TO DO */
							g_warning ("Unable to find key «%s».", token);
							dm = NULL;
							break;
						}
					id_parent = gdaex_data_model_get_field_value_integer_at (dm, 0, "id");
				}

			i++;
		}

	return dm;
}

static gchar
*path_get_value_from_db (Confi *confi, const gchar *path)
{
	gchar *ret = NULL;
	GdaDataModel *dm;

	dm = path_get_data_model (confi, path);
	if (dm != NULL)
		{
			ret = gdaex_data_model_get_field_value_stringify_at (dm, 0, "value");
		}

	return ret;
}

static void
get_children (Confi *confi, GNode *parentNode, gint idParent, gchar *path)
{
	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	gchar *sql = g_strdup_printf ("SELECT * FROM %cvalues%c "
	                              "WHERE id_configs = %d AND "
	                              "id_parent = %d",
	                              priv->chrquot, priv->chrquot,
	                              priv->id_config,
	                              idParent);

	GdaDataModel *dm = gdaex_query (priv->gdaex, sql);
	if (dm != NULL)
		{
			gint i, rows = gda_data_model_get_n_rows (dm);
			for (i = 0; i < rows; i++)
				{
					GNode *newNode;
					ConfiKey *ck = (ConfiKey *)g_malloc0 (sizeof (ConfiKey));

					ck->id_config = priv->id_config;
					ck->id = gdaex_data_model_get_field_value_integer_at (dm, i, "id");
					ck->id_parent = gdaex_data_model_get_field_value_integer_at (dm, i, "id_parent");
					ck->key = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "key"));
					ck->value = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "value"));
					ck->description = g_strdup (gdaex_data_model_get_field_value_stringify_at (dm, i, "description"));
					ck->path = g_strdup (path);

					newNode = g_node_append_data (parentNode, ck);

					get_children (confi, newNode, ck->id, g_strconcat (path, (strcmp (path, "") == 0 ? "" : "/"), ck->key, NULL));
				}
		}
}

static void
confi_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	Confi *confi = CONFI (object);

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	switch (property_id)
		{
			case PROP_NAME:
				priv->name = g_strdup (g_value_get_string (value));
				gdaex_execute (priv->gdaex, g_strdup_printf ("UPDATE configs "
				                                           "SET name = '%s' "
				                                           "WHERE id = %d",
				                                           gdaex_strescape (priv->name, NULL),
				                                           priv->id_config));
				break;

			case PROP_DESCRIPTION:
				priv->description = g_strdup (g_value_get_string (value));
				gdaex_execute (priv->gdaex, g_strdup_printf ("UPDATE configs "
				                                           "SET description = '%s' "
				                                           "WHERE id = %d",
				                                           gdaex_strescape (priv->description, NULL),
				                                           priv->id_config));
				break;

			case PROP_ROOT:
				confi_set_root (confi, g_value_get_string (value));
				break;

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
			case PROP_ID_CONFIG:
				g_value_set_int (value, priv->id_config);
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
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static gboolean
confi_delete_id_from_db_values (Confi *confi, gint id)
{
	gboolean ret = FALSE;

	ConfiPrivate *priv = CONFI_GET_PRIVATE (confi);

	gchar *sql = g_strdup_printf ("DELETE FROM %cvalues%c "
	                              "WHERE id_configs = %d "
	                              "AND id = %d",
	                              priv->chrquot, priv->chrquot,
	                              priv->id_config,
	                              id);

	return (gdaex_execute (priv->gdaex, sql) >= 0);
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
