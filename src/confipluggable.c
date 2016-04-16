/*
 * confipluggable.c
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

#include "confipluggable.h"

/**
 * SECTION:confipluggable
 * @short_description: Interface for pluggable plugins.
 * @see_also: #PeasExtensionSet
 *
 **/

G_DEFINE_INTERFACE(ZakConfiPluggable, zak_confi_pluggable, G_TYPE_OBJECT)

void
zak_confi_pluggable_default_init (ZakConfiPluggableInterface *iface)
{
	static gboolean initialized = FALSE;

	if (!initialized)
		{
			/**
			* ZakConfiPluggable:cnc_string:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("cnc_string",
			                                                          "Connection string",
			                                                          "Connection string",
			                                                          "",
			                                                          G_PARAM_READWRITE));

			/**
			* ZakConfiPluggable:name:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("name",
			                                                          "Configuraton Name",
			                                                          "The configuration name",
			                                                          "",
			                                                          G_PARAM_READWRITE));

			/**
			* ZakConfiPluggable:description:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("description",
			                                                          "Configuraton Description",
			                                                          "The configuration description",
			                                                          "",
			                                                          G_PARAM_READWRITE));

			/**
			* ZakConfiPluggable:root:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("root",
			                                                          "Configuraton Root",
			                                                          "The configuration root",
			                                                          "/",
			                                                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

			initialized = TRUE;
		}
}

/**
 * zak_confi_pluggable_initialize:
 * @pluggable: A #ZakConfiPluggable.
 * @cnc_string: The connection string.
 *
 * Initialize the backend.
 *
 * Returns: #TRUE if success.
 */
gboolean
zak_confi_pluggable_initialize (ZakConfiPluggable *pluggable, const gchar *cnc_string)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->initialize != NULL, FALSE);

	return iface->initialize (pluggable, cnc_string);
}

/**
 * zak_confi_pluggable_get_configs_list:
 * @pluggable: A #ZakConfiPluggable.
 * @filter: (nullable):
 *
 * Returns: (element-type ZakConfi) (transfer container): a #GList of #ZakConfi. If there's no configurations, returns a valid
 * #GList but with a unique NULL element.
*/
GList
*zak_confi_pluggable_get_configs_list (ZakConfiPluggable *pluggable, const gchar *filter)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->get_configs_list != NULL, FALSE);

	return iface->get_configs_list (pluggable, filter);
}

/**
 * zak_confi_pluggable_path_get_value:
 * @pluggable: A #ZakConfiPluggable.
 * @path:
 *
 * Returns: the value of the @path.
*/
gchar
*zak_confi_pluggable_path_get_value (ZakConfiPluggable *pluggable, const gchar *path)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->path_get_value != NULL, FALSE);

	return iface->path_get_value (pluggable, path);
}

/**
 * zak_confi_pluggable_path_set_value:
 * @pluggable: a #ZakConfiPluggable object.
 * @path: the key's path.
 * @value: the value to set.
 *
 */
gboolean
zak_confi_pluggable_path_set_value (ZakConfiPluggable *pluggable, const gchar *path, const gchar *value)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->path_set_value != NULL, FALSE);

	return iface->path_set_value (pluggable, path, value);
}

/**
 * zak_confi_pluggable_get_tree:
 * @pluggable: a #ZakConfiPluggable object.
 *
 * Returns: a #GNode with the entire tree of configurations.
 */
GNode
*zak_confi_pluggable_get_tree (ZakConfiPluggable *pluggable)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->get_tree != NULL, FALSE);

	return iface->get_tree (pluggable);
}

/**
 * zak_confi_pluggable_add_key:
 * @pluggable: a #ZakConfiPluggable object.
 * @parent: the path where add the key.
 * @key: the key's name.
 * @value: the key's value.
 *
 * Returns: a #ZakConfigKey struct filled with data from the key just added.
 */
ZakConfiKey
*zak_confi_pluggable_add_key (ZakConfiPluggable *pluggable, const gchar *parent, const gchar *key, const gchar *value)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->add_key != NULL, FALSE);

	return iface->add_key (pluggable, parent, key, value);
}

/**
 * zak_confi_pluggable_key_set_key:
 * @pluggable: a #ZakConfiPluggable object.
 * @ck: a #ZakConfiKey struct.
 *
 */
gboolean
zak_confi_pluggable_key_set_key (ZakConfiPluggable *pluggable,
                   ZakConfiKey *ck)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->key_set_key != NULL, FALSE);

	return iface->key_set_key (pluggable, ck);
}

/**
 * zak_confi_pluggable_path_get_confi_key:
 * @pluggable: a #ZakConfiPluggable object.
 * @path: the key's path to get.
 *
 * Returns: (transfer full): a #ZakConfiKey from @path
 */
ZakConfiKey
*zak_confi_pluggable_path_get_confi_key (ZakConfiPluggable *pluggable, const gchar *path)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->path_get_confi_key != NULL, FALSE);

	return iface->path_get_confi_key (pluggable, path);
}

/**
 * zak_confi_pluggable_remove_path:
 * @pluggable: a #ZakConfiPluggable object.
 * @path: the path to remove.
 *
 * Removes @path and every child key.
 */
gboolean
zak_confi_pluggable_remove_path (ZakConfiPluggable *pluggable, const gchar *path)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->remove_path != NULL, FALSE);

	return iface->remove_path (pluggable, path);
}

/**
 * zak_confi_pluggable_remove:
 * @pluggable: a #ZakConfiPluggable object.
 *
 * Remove a configuration from databases and destroy the relative object.
 */
gboolean
zak_confi_pluggable_remove (ZakConfiPluggable *pluggable)
{
	ZakConfiPluggableInterface *iface;

	g_return_val_if_fail (ZAK_CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = ZAK_CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->remove != NULL, FALSE);

	return iface->remove (pluggable);
}
