/*
 *  Copyright (C) 2005-2016 Andrea Zagli <azagli@libero.it>
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

#include <glib.h>

#ifdef G_OS_WIN32
#include <windows.h>
#endif

#include "libzakconfi.h"


static gchar *pluginsdir;

enum
{
	PROP_0,
	PROP_ID_CONFIG,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_ROOT
};

static void zak_confi_class_init (ZakConfiClass *klass);
static void zak_confi_init (ZakConfi *confi);

static void zak_confi_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec);
static void zak_confi_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec);

static ZakConfiPluggable *zak_confi_get_confi_pluggable_from_cnc_string (const gchar *cnc_string);

#define ZAK_CONFI_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), ZAK_TYPE_CONFI, ZakConfiPrivate))

typedef struct _ZakConfiPrivate ZakConfiPrivate;
struct _ZakConfiPrivate
	{
		gint id_config;
		gchar *name;
		gchar *description;
		gchar *root;
		GHashTable *values;

		ZakConfiPluggable *pluggable;
	};

G_DEFINE_TYPE (ZakConfi, zak_confi, G_TYPE_OBJECT)

#ifdef G_OS_WIN32
static HMODULE backend_dll = NULL;

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
         DWORD     fdwReason,
         LPVOID    lpReserved)
{
	switch (fdwReason)
		{
			case DLL_PROCESS_ATTACH:
				backend_dll = (HMODULE) hinstDLL;
				break;
			case DLL_THREAD_ATTACH:
			case DLL_THREAD_DETACH:
			case DLL_PROCESS_DETACH:
				break;
		}
	return TRUE;
}
#endif

static void
zak_confi_class_init (ZakConfiClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (object_class, sizeof (ZakConfiPrivate));

	object_class->set_property = zak_confi_set_property;
	object_class->get_property = zak_confi_get_property;
}

static void
zak_confi_init (ZakConfi *confi)
{
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	priv->pluggable = NULL;
}

static ZakConfiPluggable
*zak_confi_get_confi_pluggable_from_cnc_string (const gchar *cnc_string)
{
	ZakConfiPluggable *pluggable;

	const GList *lst_plugins;

	pluggable = NULL;

	PeasEngine *peas_engine;

	peas_engine = peas_engine_get_default ();
	if (peas_engine == NULL)
		{
			return NULL;
		}

	peas_engine_add_search_path (peas_engine, pluginsdir, NULL);

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
							ext = peas_engine_create_extension (peas_engine, ppinfo, ZAK_CONFI_TYPE_PLUGGABLE,
							                                    "cnc_string", cnc_string + strlen (uri),
							                                    NULL);
							pluggable = (ZakConfiPluggable *)ext;
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
 * zak_confi_new:
 * @cnc_string: the connection string.
 *
 * Returns: (transfer none): the newly created #ZakConfi object, or NULL if it fails.
 */
ZakConfi
*zak_confi_new (const gchar *cnc_string)
{
	ZakConfi *confi;
	ZakConfiPrivate *priv;
	ZakConfiPluggable *pluggable;

	g_return_val_if_fail (cnc_string != NULL, NULL);

#ifdef G_OS_WIN32

	gchar *moddir;
	gchar *p;

	moddir = g_win32_get_package_installation_directory_of_module (backend_dll);

	p = g_strrstr (moddir, g_strdup_printf ("%c", G_DIR_SEPARATOR));
	if (p != NULL
	    && (g_ascii_strcasecmp (p + 1, "src") == 0
	    || g_ascii_strcasecmp (p + 1, ".libs") == 0))
		{
			pluginsdir = g_strdup (PLUGINSDIR);
		}
	else
		{
			pluginsdir = g_build_filename (moddir, "lib", PACKAGE, "plugins", NULL);
		}

#undef PLUGINSDIR

#else

	pluginsdir = g_strdup (PLUGINSDIR);

#endif

	confi = NULL;

	pluggable = zak_confi_get_confi_pluggable_from_cnc_string (cnc_string);
	if (pluggable != NULL)
		{
			confi = ZAK_CONFI (g_object_new (zak_confi_get_type (), NULL));
			priv = ZAK_CONFI_GET_PRIVATE (confi);
			priv->pluggable = pluggable;
		}

	return confi;
}

PeasPluginInfo
*zak_confi_get_plugin_info (ZakConfi *confi)
{
	PeasPluginInfo *ppinfo;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ppinfo = NULL;
		}
	else
		{
			ppinfo = peas_extension_base_get_plugin_info ((PeasExtensionBase *)priv->pluggable);
		}

	return ppinfo;
}

/**
 * zak_confi_get_configs_list:
 * @cnc_string: the connection string to use to connect to database that
 * contains configuration.
 * @filter: (nullable):
 *
 * Returns: (element-type ZakConfiConfi) (transfer container):  a #GList of #ZakConfiConfi. If there's no configurations, returns a valid
 * #GList but with a unique NULL element.
 */
GList
*zak_confi_get_configs_list (const gchar *cnc_string,
							 const gchar *filter)
{
	ZakConfiPluggable *pluggable;
	GList *lst;

	pluggable = zak_confi_get_confi_pluggable_from_cnc_string (cnc_string);

	if (pluggable == NULL)
		{
			g_warning ("Not initialized.");
			lst = NULL;
		}
	else
		{
			lst = zak_confi_pluggable_get_configs_list (pluggable, filter);
		}

	return lst;
}

/**
 * zak_confi_get_confi_confi:
 * @confi:
 *
 * Returns: a #ZakConfiConfi struct.
 */
ZakConfiConfi
*zak_confi_get_confi_confi (ZakConfi *confi)
{
	ZakConfiConfi *cc;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	cc = g_new0 (ZakConfiConfi, 1);
	cc->name = g_strdup (priv->name);
	cc->description = g_strdup (priv->description);

	return cc;
}

/**
 * zak_confi_add_config:
 * @cnc_string:
 * @name:
 * @description:
 *
 * Returns: a #ZakConfiConfi struct.
 */
ZakConfiConfi
*zak_confi_add_config (const gchar *cnc_string,
					   const gchar *name,
					   const gchar *description)
{
	ZakConfiPluggable *pluggable;
	ZakConfiConfi *cc;

	pluggable = zak_confi_get_confi_pluggable_from_cnc_string (cnc_string);

	if (pluggable == NULL)
		{
			g_warning ("Not initialized.");
			cc = NULL;
		}
	else
		{
			cc = zak_confi_pluggable_add_config (pluggable, name, description);
		}

	return cc;
}

/**
 * zak_confi_get_tree:
 * @confi: a #ZakConfi object.
 *
 * Returns: a #GNode.
 */
GNode
*zak_confi_get_tree (ZakConfi *confi)
{
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable != NULL)
		{
			return zak_confi_pluggable_get_tree (priv->pluggable);
		}
	else
		{
			return NULL;
		}
}

/**
 * zak_confi_normalize_set_root:
 * @confi: a #ZakConfi object.
 * @root: the root.
 *
 * Returns: a correct value for root property.
 */
gchar
*zak_confi_normalize_root (const gchar *root)
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
 * zak_confi_set_root:
 * @confi: a #ZakConfi object.
 * @root: the root.
 *
 */
gboolean
zak_confi_set_root (ZakConfi *confi, const gchar *root)
{
	gboolean ret;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

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
 * zak_confi_set_config:
 * @confi: a #ZakConfi object.
 * @name:
 * @description:
 *
 * Returns:
 */
gboolean
zak_confi_set_config (ZakConfi *confi,
					  const gchar *name,
					  const gchar *description)
{
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			return FALSE;
		}
	else
		{
			return zak_confi_pluggable_set_config (priv->pluggable,
												   name,
												   description);
		}
}

/**
 * zak_confi_add_key:
 * @confi: a #ZakConfi object.
 * @parent: the path where add the key.
 * @key: the key's name.
 * @value: the key's value.
 *
 * Returns: a #ZakConfigKey struct filled with data from the key just added.
 */
ZakConfiKey
*zak_confi_add_key (ZakConfi *confi, const gchar *parent, const gchar *key, const gchar *value)
{
	ZakConfiKey *ck;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ck = NULL;
		}
	else
		{
			ck = zak_confi_pluggable_add_key (priv->pluggable, parent, key, value);
		}

	return ck;
}

/**
 * zak_confi_key_set_key:
 * @confi: a #ZakConfi object.
 * @ck: a #ZakConfiKey struct.
 *
 */
gboolean
zak_confi_key_set_key (ZakConfi *confi,
                   ZakConfiKey *ck)
{
	gboolean ret;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = FALSE;
		}
	else
		{
			ret = zak_confi_pluggable_key_set_key (priv->pluggable, ck);
		}

	return ret;
}

/**
 * zak_confi_remove_path:
 * @confi: a #ZakConfi object.
 * @path: the path to remove.
 *
 * Removes @path and every child key.
 */
gboolean
zak_confi_remove_path (ZakConfi *confi, const gchar *path)
{
	gboolean ret;
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = FALSE;
		}
	else
		{
			ret = zak_confi_pluggable_remove_path (priv->pluggable, path);
		}

	return ret;
}

/**
 * zak_confi_path_get_value:
 * @confi: a #ZakConfi object.
 * @path: the path from which retrieving the value.
 *
 * Returns: the configuration's value as a string.
 */
gchar
*zak_confi_path_get_value (ZakConfi *confi, const gchar *path)
{
	gchar *ret;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = NULL;
		}
	else
		{
			ret = zak_confi_pluggable_path_get_value (priv->pluggable, path);
		}

	return ret;
}

/**
 * zak_confi_path_set_value:
 * @confi: a #ZakConfi object.
 * @path: the key's path.
 * @value: the value to set.
 *
 */
gboolean
zak_confi_path_set_value (ZakConfi *confi, const gchar *path, const gchar *value)
{
	gboolean ret;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = FALSE;
		}
	else
		{
			ret = zak_confi_pluggable_path_set_value (priv->pluggable, path, value);
		}

	return ret;
}

/**
 * zak_confi_path_get_confi_key:
 * @confi: a #ZakConfi object.
 * @path: the key's path to get.
 *
 * Returns: (transfer full): a #ZakConfiKey from @path
 */
ZakConfiKey
*zak_confi_path_get_confi_key (ZakConfi *confi, const gchar *path)
{
	ZakConfiKey *ck;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ck = NULL;
		}
	else
		{
			ck = zak_confi_pluggable_path_get_confi_key (priv->pluggable, path);
		}

	return ck;
}

/**
 * zak_confi_remove:
 * @confi: a #ZakConfi object.
 *
 * Remove a configuration from databases and destroy the relative object.
 */
gboolean
zak_confi_remove (ZakConfi *confi)
{
	gboolean ret;

	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	if (priv->pluggable == NULL)
		{
			g_warning ("Not initialized.");
			ret = FALSE;
		}
	else
		{
			ret = zak_confi_pluggable_remove (priv->pluggable);
		}

	if (ret)
		{
			zak_confi_destroy (confi);
		}

	return ret;
}

/**
 * zak_confi_destroy:
 * @confi: a #ZakConfi object.
 *
 * Destroy the #ZakConfi object, freeing memory.
 */
void
zak_confi_destroy (ZakConfi *confi)
{
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	g_free (priv->name);
	g_free (priv->description);
	g_free (priv->root);
	g_object_unref (priv->pluggable);
}

/**
 * zak_confi_path_normalize:
 * @pluggable: a #ZakConfiPluggable object.
 *
 * Returns: a normalize path (with root).
 */
gchar
*zak_confi_path_normalize (ZakConfiPluggable *pluggable, const gchar *path)
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
zak_confi_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
	gchar *sql;

	ZakConfi *confi = ZAK_CONFI (object);
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}

static void
zak_confi_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
	ZakConfi *confi = ZAK_CONFI (object);
	ZakConfiPrivate *priv = ZAK_CONFI_GET_PRIVATE (confi);

	switch (property_id)
		{
			default:
				G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
				break;
		}
}
