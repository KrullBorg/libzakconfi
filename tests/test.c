/*
 * Copyright (C) 2005-2016 Andrea Zagli <azagli@libero.it>
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

#include <glib/gprintf.h>
#include <libpeas/peas.h>

#include "libzakconfi.h"

gboolean
traverse_func (GNode *node,
               gpointer data)
{
	ZakConfiKey *ck = (ZakConfiKey *)node->data;
	g_printf ("%s%s%s => %s\n", ck->path, g_strcmp0 (ck->path, "") == 0 ? "" : "/", ck->key, ck->value);

	return FALSE;
}

int
main (int argc, char **argv)
{
	PeasEngine *engine;
	ZakConfi *confi;
	PeasPluginInfo *ppinfo;
	GNode *tree;

	if (argc < 2)
		{
			g_error ("Usage: test <connection string>");
			return 0;
		}

	engine = peas_engine_get_default ();
	peas_engine_add_search_path (engine, "./plugins", NULL);

	confi = zak_confi_new (argv[1]);
	if (confi == NULL)
		{
			g_error ("Error on configuration initialization.");
			return 0;
		}

	ppinfo = zak_confi_get_plugin_info (confi);
	g_printf ("Plugin info\n");
	g_printf ("Name: %s\n", peas_plugin_info_get_name (ppinfo));
	g_printf ("Module dir: %s\n", peas_plugin_info_get_module_dir (ppinfo));
	g_printf ("Module name: %s\n", peas_plugin_info_get_module_name (ppinfo));
	g_printf ("\n");

	g_printf ("Traversing the entire tree\n");
	tree = zak_confi_get_tree (confi);
	g_node_traverse (tree, G_PRE_ORDER, G_TRAVERSE_ALL, -1, traverse_func, NULL);
	g_printf ("\n");

	gchar *val = zak_confi_path_get_value (confi, "folder/key1/key1_2");
	g_printf ("Value from key \"folder/key1/key1_2\"\n%s\n\n", val);
	zak_confi_path_set_value (confi, "folder/key1/key1_2", "new value programmatically setted");
	g_printf ("Value from key \"folder/key1/key1_2\"\n%s\n\n", zak_confi_path_get_value (confi, "folder/key1/key1_2"));
	zak_confi_path_set_value (confi, "folder/key1/key1_2", val);
	g_printf ("Value from key \"folder/key1/key1_2\"\n%s\n\n", zak_confi_path_get_value (confi, "folder/key1/key1_2"));

	zak_confi_add_key (confi, "folder/key2", "key2-2", NULL);
	zak_confi_path_set_value (confi, "folder/key2/key2-2", "value for key2-2, programmatically setted");

	ZakConfiKey *ck;
	ck = zak_confi_path_get_confi_key (confi, "folder/key2/key2-2");
	g_printf ("ConfiKey for folder/key2/key2-2\n");
	g_printf ("Path: %s\n", ck->path);
	g_printf ("Key: %s\n", ck->key);
	g_printf ("Description: %s\n", ck->description);
	g_printf ("Value: %s\n", ck->value);
	g_printf ("\n");

	g_printf ("Setting root \"folder/key2\"\n");
	zak_confi_set_root (confi, "folder/key2");
	g_printf ("Value from key \"key2-1\" %s\n", zak_confi_path_get_value (confi, "key2-1"));
	g_printf ("Value from key \"folder/key1/key1_2\" (expected null) %s\n", zak_confi_path_get_value (confi, "folder/key1/key1_2"));

	zak_confi_destroy (confi);

	return 0;
}
