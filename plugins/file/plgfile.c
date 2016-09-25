/*
 * plgfile.c
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

#include "../../src/libzakconfi.h"
#include "../../src/confipluggable.h"

#include "plgfile.h"

static void zak_confi_pluggable_iface_init (ZakConfiPluggableInterface *iface);

gboolean zak_confi_file_plugin_initialize (ZakConfiPluggable *pluggable, const gchar *cnc_string);

static gboolean zak_confi_file_plugin_path_get_group_and_key (const gchar *path, gchar **group, gchar **key);
static gchar *zak_confi_file_plugin_path_get_value_from_file (ZakConfiPluggable *pluggable, const gchar *path);
static gchar *zak_confi_file_plugin_path_get_value (ZakConfiPluggable *pluggable, const gchar *path);
static gboolean zak_confi_file_plugin_path_set_value (ZakConfiPluggable *pluggable, const gchar *path, const gchar *value);
static void zak_confi_file_plugin_get_children (ZakConfiPluggable *pluggable, GNode *parentNode);

#define ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ZAK_CONFI_TYPE_FILE_PLUGIN, ZakConfiFilePluginPrivate))

typedef struct _ZakConfiFilePluginPrivate ZakConfiFilePluginPrivate;
struct _ZakConfiFilePluginPrivate
	{
		gchar *cnc_string;

		GKeyFile *kfile;

		gchar *name;
		gchar *description;
		gchar *root;
	};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (ZakConfiFilePlugin,
                                zak_confi_file_plugin,
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
zak_confi_file_plugin_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	ZakConfiFilePlugin *plugin = ZAK_CONFI_FILE_PLUGIN (object);
	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

	switch (prop_id)
		{
			case PROP_CNC_STRING:
				zak_confi_file_plugin_initialize ((ZakConfiPluggable *)plugin, g_value_get_string (value));
				break;

			case PROP_NAME:
				priv->name = g_strdup (g_value_get_string (value));
				zak_confi_file_plugin_path_set_value ((ZakConfiPluggable *)plugin, "/CONFI/name", priv->name);
				break;

			case PROP_DESCRIPTION:
				priv->description = g_strdup (g_value_get_string (value));
				zak_confi_file_plugin_path_set_value ((ZakConfiPluggable *)plugin, "/CONFI/description", priv->description);
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
zak_confi_file_plugin_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	ZakConfiFilePlugin *plugin = ZAK_CONFI_FILE_PLUGIN (object);
	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

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
zak_confi_file_plugin_init (ZakConfiFilePlugin *plugin)
{
	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

	priv->cnc_string = NULL;
	priv->kfile = NULL;
	priv->name = NULL;
	priv->description = NULL;
}

static void
zak_confi_file_plugin_finalize (GObject *object)
{
	ZakConfiFilePlugin *plugin = ZAK_CONFI_FILE_PLUGIN (object);

	G_OBJECT_CLASS (zak_confi_file_plugin_parent_class)->finalize (object);
}

gboolean
zak_confi_file_plugin_initialize (ZakConfiPluggable *pluggable, const gchar *cnc_string)
{
	gboolean ret;
	GError *error;

	ZakConfiFilePlugin *plugin = ZAK_CONFI_FILE_PLUGIN (pluggable);
	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (plugin);

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
zak_confi_file_plugin_path_get_group_and_key (const gchar *path, gchar **group, gchar **key)
{
	gchar *last;

	if (path == NULL) return FALSE;
	if (path[strlen (path) - 1] == '/') return FALSE;

	last = g_strrstr (path, "/");
	if (last == NULL)
		{
			return FALSE;
		}

	*group = g_strndup (path + (path[0] == '/' ? 1 : 0), strlen (path + (path[0] == '/' ? 1 : 0)) - strlen (last));
	g_strstrip (*group);

	*key = g_strdup (last + 1);
	g_strstrip (*key);

	return TRUE;
}

static gchar
*zak_confi_file_plugin_path_get_value_from_file (ZakConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;

	gchar *group;
	gchar *key;

	GError *error;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	if (path == NULL) return NULL;

	group = NULL;
	key = NULL;
	if (!zak_confi_file_plugin_path_get_group_and_key (path, &group, &key))
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
zak_confi_file_plugin_get_children (ZakConfiPluggable *pluggable, GNode *parentNode)
{
	gchar **groups;
	gchar **keys;
	gsize lg;
	gsize lk;
	guint g;
	guint k;

	GError *error;

	GNode *gNode;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	groups = g_key_file_get_groups (priv->kfile, &lg);

	for (g = 0; g < lg; g++)
		{
			ZakConfiKey *ck = g_new0 (ZakConfiKey, 1);

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
							ZakConfiKey *ck = g_new0 (ZakConfiKey, 1);

							ck->key = g_strdup (keys[k]);
							ck->path = g_strdup (groups[g]);
							ck->value = zak_confi_file_plugin_path_get_value (pluggable, g_strdup_printf ("%s/%s", groups[g], keys[k]));
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
*zak_confi_file_plugin_get_configs_list (ZakConfiPluggable *pluggable,
                                     const gchar *filter)
{
	GList *lst;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	lst = NULL;

	ZakConfiConfi *confi;
	confi = g_new0 (ZakConfiConfi, 1);
	confi->name = g_strdup (priv->name);
	confi->description = g_strdup (priv->description);
	lst = g_list_append (lst, confi);

	return lst;
}

static gchar
*zak_confi_file_plugin_path_get_value (ZakConfiPluggable *pluggable, const gchar *path)
{
	gchar *ret;
	gchar *path_;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ret = NULL;

	path_ = zak_confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	ret = zak_confi_file_plugin_path_get_value_from_file (pluggable, path_);

	return ret;
}

static gboolean
zak_confi_file_plugin_path_set_value (ZakConfiPluggable *pluggable, const gchar *path, const gchar *value)
{
	gboolean ret;

	gchar *path_;
	gchar *group;
	gchar *key;

	GError *error;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	g_return_val_if_fail (value != NULL, FALSE);

	path_ = zak_confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return FALSE;
		}

	group = NULL;
	key = NULL;
	if (!zak_confi_file_plugin_path_get_group_and_key (path, &group, &key))
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
*zak_confi_file_plugin_get_tree (ZakConfiPluggable *pluggable)
{
	GNode *node;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ZakConfiKey *ck = g_new0 (ZakConfiKey, 1);

	ck->config = "Default";
	ck->path = "";
	ck->key = g_strdup ("/");
	ck->value = "";
	ck->description = "";

	node = g_node_new (ck);

	zak_confi_file_plugin_get_children (pluggable, node);

	return node;
}

static ZakConfiKey
*zak_confi_file_plugin_add_key (ZakConfiPluggable *pluggable, const gchar *parent, const gchar *key, const gchar *value)
{
	ZakConfiKey *ck;
	gchar *path;

	gchar *group;
	gchar *key_;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ck = NULL;
	path = g_strdup_printf ("%s/%s", parent, key);
	if (zak_confi_file_plugin_path_set_value (pluggable, path, value))
		{
			group = NULL;
			key_ = NULL;
			if (zak_confi_file_plugin_path_get_group_and_key (path, &group, &key_))
				{
					ck = g_new0 (ZakConfiKey, 1);
					ck->config = "Default";
					ck->path = g_strdup (path);
					ck->key = g_strdup (key);
					ck->value = zak_confi_file_plugin_path_get_value (pluggable, path);
					ck->description = g_key_file_get_comment (priv->kfile, group, key, NULL);;

					g_free (group);
					g_free (key_);
				}
		}
	g_free (path);

	return ck;
}

static gboolean
zak_confi_file_plugin_key_set_key (ZakConfiPluggable *pluggable,
                               ZakConfiKey *ck)
{
	gboolean ret;

	gchar *path;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	path = g_strdup_printf ("%s/%s", ck->path, ck->key);
	ret = zak_confi_file_plugin_path_set_value (pluggable, path, ck->value);
	g_free (path);

	return ret;
}

static ZakConfiKey
*zak_confi_file_plugin_path_get_confi_key (ZakConfiPluggable *pluggable, const gchar *path)
{
	gchar *path_;
	ZakConfiKey *ck;

	gchar *group;
	gchar *key;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	path_ = zak_confi_path_normalize (pluggable, path);
	if (path_ == NULL)
		{
			return NULL;
		}

	group = NULL;
	key = NULL;
	if (zak_confi_file_plugin_path_get_group_and_key (path_, &group, &key))
		{
			ck = g_new0 (ZakConfiKey, 1);
			ck->config = "Default";
			ck->path = g_strdup (group);
			ck->key = g_strdup (key);
			ck->value = zak_confi_file_plugin_path_get_value (pluggable, path_);
			ck->description = g_key_file_get_comment (priv->kfile, group, key, NULL);

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

static gboolean
zak_confi_file_plugin_remove_path (ZakConfiPluggable *pluggable, const gchar *path)
{
	gboolean ret;

	gchar *group;
	gchar *key;

	GError *error;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	group = NULL;
	key = NULL;
	if (zak_confi_file_plugin_path_get_group_and_key (path, &group, &key))
		{
			error = NULL;
			ret = g_key_file_remove_key (priv->kfile, group, key, &error);
			if (error != NULL)
				{
					g_warning ("Error on removing key from file: %s.",
					           error != NULL && error->message != NULL ? error->message : "no details");
				}
		}
	else
		{
			ret = FALSE;
		}

	return ret;
}

static gboolean
zak_confi_file_plugin_remove (ZakConfiPluggable *pluggable)
{
	gboolean ret;
	GFile *gfile;

	ZakConfiFilePluginPrivate *priv = ZAK_CONFI_FILE_PLUGIN_GET_PRIVATE (pluggable);

	ret = TRUE;

	gfile = g_file_new_for_path (priv->cnc_string);
	if (gfile != NULL)
		{
			g_file_delete (gfile, NULL, NULL);
		}
	g_key_file_unref (priv->kfile);

	return ret;
}

static void
zak_confi_file_plugin_class_init (ZakConfiFilePluginClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ZakConfiFilePluginPrivate));

	object_class->set_property = zak_confi_file_plugin_set_property;
	object_class->get_property = zak_confi_file_plugin_get_property;
	object_class->finalize = zak_confi_file_plugin_finalize;

	g_object_class_override_property (object_class, PROP_CNC_STRING, "cnc_string");
	g_object_class_override_property (object_class, PROP_NAME, "name");
	g_object_class_override_property (object_class, PROP_DESCRIPTION, "description");
	g_object_class_override_property (object_class, PROP_ROOT, "root");
}

static void
zak_confi_pluggable_iface_init (ZakConfiPluggableInterface *iface)
{
	iface->initialize = zak_confi_file_plugin_initialize;
	iface->get_configs_list = zak_confi_file_plugin_get_configs_list;
	iface->path_get_value = zak_confi_file_plugin_path_get_value;
	iface->path_set_value = zak_confi_file_plugin_path_set_value;
	iface->get_tree = zak_confi_file_plugin_get_tree;
	iface->add_key = zak_confi_file_plugin_add_key;
	iface->key_set_key = zak_confi_file_plugin_key_set_key;
	iface->path_get_confi_key = zak_confi_file_plugin_path_get_confi_key;
	iface->remove_path = zak_confi_file_plugin_remove_path;
	iface->remove = zak_confi_file_plugin_remove;
}

static void
zak_confi_file_plugin_class_finalize (ZakConfiFilePluginClass *klass)
{
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	zak_confi_file_plugin_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
	                                            ZAK_CONFI_TYPE_PLUGGABLE,
	                                            ZAK_CONFI_TYPE_FILE_PLUGIN);
}
