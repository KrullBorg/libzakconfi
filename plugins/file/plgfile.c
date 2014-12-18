/*
 * plgfile.c
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

#include "../../src/libconfi.h"
#include "../../src/confipluggable.h"

#include "plgfile.h"

static void confi_pluggable_iface_init (ConfiPluggableInterface *iface);

static gboolean confi_file_plugin_path_get_group_and_key (ConfiPluggable *pluggable, const gchar *path, gchar **group, gchar **key);
static gchar *confi_file_plugin_path_get_value_from_file (ConfiPluggable *pluggable, const gchar *path);
static gchar *confi_file_plugin_path_get_value (ConfiPluggable *pluggable, const gchar *path);
static gboolean confi_file_plugin_path_set_value (ConfiPluggable *pluggable, const gchar *path, const gchar *value);
static void confi_file_plugin_get_children (ConfiPluggable *pluggable, GNode *parentNode);

#define CONFI_FILE_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), CONFI_TYPE_FILE_PLUGIN, ConfiFilePluginPrivate))

typedef struct _ConfiFilePluginPrivate ConfiFilePluginPrivate;
struct _ConfiFilePluginPrivate
	{
		gchar *cnc_string;

		GKeyFile *kfile;

		gchar *name;
		gchar *description;
		gchar *root;
	};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (ConfiFilePlugin,
                                confi_file_plugin,
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
confi_file_plugin_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	ConfiFilePlugin *plugin = CONFI_FILE_PLUGIN (object);
	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

	switch (prop_id)
		{
			case PROP_CNC_STRING:
				confi_file_plugin_initialize ((ConfiPluggable *)plugin, g_value_get_string (value));
				break;

			case PROP_NAME:
				priv->name = g_strdup (g_value_get_string (value));
				confi_file_plugin_path_set_value ((ConfiPluggable *)plugin, "/CONFI/name", priv->name);
				break;

			case PROP_DESCRIPTION:
				priv->description = g_strdup (g_value_get_string (value));
				confi_file_plugin_path_set_value ((ConfiPluggable *)plugin, "/CONFI/description", priv->description);
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
confi_file_plugin_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	ConfiFilePlugin *plugin = CONFI_FILE_PLUGIN (object);
	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

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
confi_file_plugin_init (ConfiFilePlugin *plugin)
{
	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

	priv->cnc_string = NULL;
	priv->kfile = NULL;
	priv->name = NULL;
	priv->description = NULL;
}

static void
confi_file_plugin_finalize (GObject *object)
{
	ConfiFilePlugin *plugin = CONFI_FILE_PLUGIN (object);

	G_OBJECT_CLASS (confi_file_plugin_parent_class)->finalize (object);
}

gboolean
confi_file_plugin_initialize (ConfiPluggable *pluggable, const gchar *cnc_string)
{
	gboolean ret;
	GError *error;

	ConfiFilePlugin *plugin = CONFI_FILE_PLUGIN (pluggable);
	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

	priv->cnc_string = g_strdup (cnc_string);

	priv->kfile = g_key_file_new ();
	error = NULL;
	if (g_key_file_load_from_file (priv->kfile, priv->cnc_string, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error)
	    && error == NULL)
		{
			error = NULL;
			priv->name = g_key_file_get_value (priv->kfile, "CONFI", "name", &error);
			if (priv->name == NULL || error != NULL)
				{
					priv->name = g_strdup ("Default");
				}
			priv->description = g_key_file_get_value (priv->kfile, "CONFI", "description", NULL);
		}
	else
		{
			g_warning ("Error: %s", error != NULL && error->message != NULL ? error->message : "no details");
			g_key_file_free (priv->kfile);
			priv->kfile = NULL;
			ret = FALSE;
		}

	return ret;
}

static gboolean
confi_file_plugin_path_get_group_and_key (ConfiPluggable *pluggable, const gchar *path, gchar **group, gchar **key)
{
	gchar *path_;
	gchar **tokens;

	guint l;
	guint i;
	guint c;

	if (path == NULL) return FALSE;

	path_ = g_strdup_printf ("%s/", path);
	tokens = g_strsplit (path_, "/", -1);
	if (tokens == NULL) return FALSE;

	l = g_strv_length (tokens);
	c = 1;
	for (i = 0; i < l; i++)
		{
			if (g_strcmp0 (tokens[i], "") != 0)
				{
					if (c == 1)
						{
							*group = g_strdup (tokens[i]);
							g_strstrip (*group);
							c = 2;
						}
					else if (c == 2)
						{
							*key = g_strdup (tokens[i]);
							g_strstrip (*key);
							c = 3;
						}
					if (c > 2)
						{
							break;
						}
				}
		}
	g_strfreev (tokens);
	g_free (path_);

	return TRUE;
}

static gchar
*confi_file_plugin_path_get_value_from_file (ConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;

	gchar *group;
	gchar *key;

	GError *error;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	if (path == NULL) return NULL;

	group = NULL;
	key = NULL;
	if (!confi_file_plugin_path_get_group_and_key (pluggable, path, &group, &key))
		{
			return NULL;
		}

	error = NULL;
	ret = g_key_file_get_value (priv->kfile, group, key, &error);
	if (error != NULL)
		{
			if (ret != NULL)
				{
					g_free (ret);
				}
			ret = NULL;
		}
	g_free (group);
	g_free (key);

	return ret;
}

static void
confi_file_plugin_get_children (ConfiPluggable *pluggable, GNode *parentNode)
{
	gchar **groups;
	gchar **keys;
	guint lg;
	guint lk;
	guint g;
	guint k;

	gchar *group;
	gchar *key;

	GError *error;

	GNode *gNode;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	groups = g_key_file_get_groups (priv->kfile, &lg);

	for (g = 0; g < lg; g++)
		{
			ConfiKey *ck = g_new0 (ConfiKey, 1);

			ck->key = g_strdup (groups[g]);
			ck->value = "";
			ck->description = "";
			ck->path = "";

			gNode = g_node_append_data (parentNode, ck);

			error = NULL;
			keys = g_key_file_get_keys (priv->kfile, groups[g], &lk, &error);
			if (keys != NULL && error == NULL && lk > 0)
				{
					for (k = 0; k < lk; k++)
						{
							ConfiKey *ck = g_new0 (ConfiKey, 1);

							ck->key = g_strdup (keys[k]);
							ck->path = g_strdup (groups[g]);
							ck->value = confi_file_plugin_path_get_value (pluggable, g_strdup_printf ("%s/%s", groups[g], keys[k]));
							ck->description = g_key_file_get_comment (priv->kfile, groups[g], keys[k], NULL);

							g_node_append_data (gNode, ck);
						}
				}

			if (keys != NULL)
				{
					g_strfreev (keys);
				}
		}

	if (groups != NULL)
		{
			g_strfreev (groups);
		}
}

static GList
*confi_file_plugin_get_configs_list (ConfiPluggable *pluggable,
                                     const gchar *filter)
{
	GList *lst;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	lst = NULL;

	ConfiConfi *confi;
	confi = g_new0 (ConfiConfi, 1);
	confi->name = g_strdup (priv->name);
	confi->description = g_strdup (priv->description);
	lst = g_list_append (lst, confi);

	return lst;
}

static gchar
*confi_file_plugin_path_get_value (ConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;
	gchar *path_;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ret = NULL;

	path_ = confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	ret = confi_file_plugin_path_get_value_from_file (pluggable, path_);

	return ret;
}

static gboolean
confi_file_plugin_path_set_value (ConfiPluggable *pluggable, const gchar *path, const gchar *value)
{
	gboolean ret;

	gchar *group;
	gchar *key;

	GError *error;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	g_return_val_if_fail (value != NULL, FALSE);

	group = NULL;
	key = NULL;
	if (!confi_file_plugin_path_get_group_and_key (pluggable, path, &group, &key))
		{
			return FALSE;
		}

	g_key_file_set_value (priv->kfile, group, key, value);
	g_free (group);
	g_free (key);

	error = NULL;
	ret = g_key_file_save_to_file (priv->kfile, priv->cnc_string, &error);
	if (error != NULL)
		{
			ret = FALSE;
		}

	return ret;
}

GNode
*confi_file_plugin_get_tree (ConfiPluggable *pluggable)
{
	GNode *node;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ConfiKey *ck = g_new0 (ConfiKey, 1);

	ck->path = "";
	ck->description = "";
	ck->key = g_strdup ("/");
	ck->value = "";

	node = g_node_new (ck);

	confi_file_plugin_get_children (pluggable, node);

	return node;
}

static ConfiKey
*confi_file_plugin_add_key (ConfiPluggable *pluggable, const gchar *parent, const gchar *key, const gchar *value)
{
	ConfiKey *ck;
	gchar *path;

	gchar *group;
	gchar *key_;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ck = NULL;
	path = g_strdup_printf ("%s/%s", parent, key);
	if (confi_file_plugin_path_set_value (pluggable, path, value))
		{
			group = NULL;
			key_ = NULL;
			if (confi_file_plugin_path_get_group_and_key (pluggable, path, &group, &key_))
				{
					ck = g_new0 (ConfiKey, 1);
					ck->key = g_strdup (key);
					ck->value = confi_file_plugin_path_get_value (pluggable, path);
					ck->description = g_key_file_get_comment (priv->kfile, group, key, NULL);;
					ck->path = g_strdup (path);

					g_free (group);
					g_free (key_);
				}
		}
	g_free (path);

	return ck;
}

static ConfiKey
*confi_file_plugin_path_get_confi_key (ConfiPluggable *pluggable, const gchar *path)
{
	gchar *path_;
	ConfiKey *ck;

	gchar *group;
	gchar *key;

	ConfiFilePluginPrivate *priv = CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	path_ = confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	group = NULL;
	key = NULL;
	if (confi_file_plugin_path_get_group_and_key (pluggable, path_, &group, &key))
		{
			ck = g_new0 (ConfiKey, 1);
			ck->key = g_strdup (key);
			ck->value = confi_file_plugin_path_get_value (pluggable, path_);
			ck->description = g_key_file_get_comment (priv->kfile, group, key, NULL);
			ck->path = g_strdup (group);

			g_free (group);
			g_free (key);
		}
	else
		{
			ck = NULL;
		}
	g_free (path_);

	return ck;
}

static void
confi_file_plugin_class_init (ConfiFilePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ConfiFilePluginPrivate));

	object_class->set_property = confi_file_plugin_set_property;
	object_class->get_property = confi_file_plugin_get_property;
	object_class->finalize = confi_file_plugin_finalize;

	g_object_class_override_property (object_class, PROP_CNC_STRING, "cnc_string");
	g_object_class_override_property (object_class, PROP_NAME, "name");
	g_object_class_override_property (object_class, PROP_DESCRIPTION, "description");
	g_object_class_override_property (object_class, PROP_ROOT, "root");
}

static void
confi_pluggable_iface_init (ConfiPluggableInterface *iface)
{
	iface->initialize = confi_file_plugin_initialize;
	iface->get_configs_list = confi_file_plugin_get_configs_list;
	iface->path_get_value = confi_file_plugin_path_get_value;
	iface->path_set_value = confi_file_plugin_path_set_value;
	iface->get_tree = confi_file_plugin_get_tree;
	iface->add_key = confi_file_plugin_add_key;
	iface->path_get_confi_key = confi_file_plugin_path_get_confi_key;
}

static void
confi_file_plugin_class_finalize (ConfiFilePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	confi_file_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            CONFI_TYPE_PLUGGABLE,
	                                            CONFI_TYPE_FILE_PLUGIN);
}
