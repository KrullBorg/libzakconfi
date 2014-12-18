/*
 * confipluggable.c
 * This file is part of libconfi
 *
 * Copyright (C) 2014 Andrea Zagli <azagli@libero.it>
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

G_DEFINE_INTERFACE(ConfiPluggable, confi_pluggable, G_TYPE_OBJECT)

void
confi_pluggable_default_init (ConfiPluggableInterface *iface)
{
	static gboolean initialized = FALSE;

	if (!initialized)
		{
			/**
			* ConfiPluggable:cnc_string:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("cnc_string",
			                                                          "Connection string",
			                                                          "Connection string",
			                                                          "",
			                                                          G_PARAM_READWRITE));

			/**
			* ConfiPluggable:name:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("name",
			                                                          "Configuraton Name",
			                                                          "The configuration name",
			                                                          "",
			                                                          G_PARAM_READWRITE));

			/**
			* ConfiPluggable:description:
			*
			*/
			g_object_interface_install_property (iface,
			                                     g_param_spec_string ("description",
			                                                          "Configuraton Description",
			                                                          "The configuration description",
			                                                          "",
			                                                          G_PARAM_READWRITE));

			/**
			* ConfiPluggable:root:
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
 * confi_pluggable_initialize:
 * @pluggable: A #ConfiPluggable.
 * @cnc_string: The connection string.
 *
 * Initialize the backend.
 *
 * Returns: #TRUE if success.
 */
gboolean
confi_pluggable_initialize (ConfiPluggable *pluggable, const gchar *cnc_string)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->initialize != NULL, FALSE);

	return iface->initialize (pluggable, cnc_string);
}

/**
 * confi_pluggable_get_configs_list:
 * @pluggable: A #ConfiPluggable.
 * @filter: (nullable):
 *
 * Returns: (element-type ConfiConfi) (transfer container): a #GList of #ConfiConfi. If there's no configurations, returns a valid
 * #GList but with a unique NULL element.
*/
GList
*confi_pluggable_get_configs_list (ConfiPluggable *pluggable, const gchar *filter)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->get_configs_list != NULL, FALSE);

	return iface->get_configs_list (pluggable, filter);
}

/**
 * confi_pluggable_path_get_value:
 * @pluggable: A #ConfiPluggable.
 * @path:
 *
 * Returns: the value of the @path.
*/
gchar
*confi_pluggable_path_get_value (ConfiPluggable *pluggable, const gchar *path)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->path_get_value != NULL, FALSE);

	return iface->path_get_value (pluggable, path);
}

/**
 * confi_pluggable_path_set_value:
 * @pluggable: a #ConfiPluggable object.
 * @path: the key's path.
 * @value: the value to set.
 *
 */
gboolean
confi_pluggable_path_set_value (ConfiPluggable *pluggable, const gchar *path, const gchar *value)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->path_set_value != NULL, FALSE);

	return iface->path_set_value (pluggable, path, value);
}

/**
 * confi_pluggable_get_tree:
 * @pluggable: a #ConfiPluggable object.
 *
 * Returns: a #GNode with the entire tree of configurations.
 */
GNode
*confi_pluggable_get_tree (ConfiPluggable *pluggable)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->get_tree != NULL, FALSE);

	return iface->get_tree (pluggable);
}

/**
 * confi_pluggable_add_key:
 * @pluggable: a #ConfiPluggable object.
 * @parent: the path where add the key.
 * @key: the key's name.
 * @value: the key's value.
 *
 * Returns: a #ConfigKey struct filled with data from the key just added.
 */
ConfiKey
*confi_pluggable_add_key (ConfiPluggable *pluggable, const gchar *parent, const gchar *key, const gchar *value)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->add_key != NULL, FALSE);

	return iface->add_key (pluggable, parent, key, value);
}

/**
 * confi_pluggable_key_set_key:
 * @pluggable: a #ConfiPluggable object.
 * @ck: a #ConfiKey struct.
 *
 */
gboolean
confi_pluggable_key_set_key (ConfiPluggable *pluggable,
                   ConfiKey *ck)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->key_set_key != NULL, FALSE);

	return iface->key_set_key (pluggable, ck);
}

/**
 * confi_pluggable_path_get_confi_key:
 * @pluggable: a #ConfiPluggable object.
 * @path: the key's path to get.
 *
 * Returns: (transfer full): a #ConfiKey from @path
 */
ConfiKey
*confi_pluggable_path_get_confi_key (ConfiPluggable *pluggable, const gchar *path)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->path_get_confi_key != NULL, FALSE);

	return iface->path_get_confi_key (pluggable, path);
}

/**
 * confi_pluggable_remove_path:
 * @pluggable: a #ConfiPluggable object.
 * @path: the path to remove.
 *
 * Removes @path and every child key.
 */
gboolean
confi_pluggable_remove_path (ConfiPluggable *pluggable, const gchar *path)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->remove_path != NULL, FALSE);

	return iface->remove_path (pluggable, path);
}

/**
 * confi_pluggable_remove:
 * @pluggable: a #ConfiPluggable object.
 *
 * Remove a configuration from databases and destroy the relative object.
 */
gboolean
confi_pluggable_remove (ConfiPluggable *pluggable)
{
	ConfiPluggableInterface *iface;

	g_return_val_if_fail (CONFI_IS_PLUGGABLE (pluggable), FALSE);

	iface = CONFI_PLUGGABLE_GET_IFACE (pluggable);
	g_return_val_if_fail (iface->remove != NULL, FALSE);

	return iface->remove (pluggable);
}
