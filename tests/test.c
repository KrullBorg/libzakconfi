/*
 * Copyright (C) 2005-2014 Andrea Zagli <azagli@libero.it>
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

#include <libpeas/peas.h>
#include <libconfi.h>

gboolean
traverse_func (GNode *node,
               gpointer data)
{
	ConfiKey *ck = (ConfiKey *)node->data;
	if (ck->id != 0)
		{
			g_printf ("%s%s%s\n", ck->path, strcmp (ck->path, "") == 0 ? "" : "/", ck->key);
		}

	return FALSE;
}

int
main (int argc, char **argv)
{
	PeasEngine *engine;
	Confi *confi;
	GList *confis;
	GNode *tree;

	gda_init ();

	if (argc < 2)
		{
			g_error ("Usage: test <connection string>");
			return 0;
		}

	engine = peas_engine_get_default ();
	peas_engine_add_search_path (engine, "./plugins", NULL);

	confis = confi_get_configs_list (argv[1], NULL);
	while (confis)
		{
			ConfiConfi *confi = (ConfiConfi *)confis->data;

			if (confi == NULL) break;

			g_printf ("NAME: %s\nDESCRIPTION: %s\n\n",
			          confi->name,
			          confi->description);
			confis = g_list_next (confis);
		}

	confi = confi_new (argv[1]);
	if (confi == NULL)
		{
			g_error ("Error on configuration initialization.");
			return 0;
		}

	gchar *val = confi_path_get_value (confi, "folder/key1/key1_2");
	g_printf ("Value from key \"folder/key1/key1_2\"\n%s\n\n", val);
	confi_path_set_value (confi, "folder/key1/key1_2", "new value programmatically setted");
	g_printf ("Value from key \"folder/key1/key1_2\"\n%s\n\n", confi_path_get_value (confi, "folder/key1/key1_2"));
	confi_path_set_value (confi, "folder/key1/key1_2", val);

	g_printf ("Traversing the entire tree\n");
	tree = confi_get_tree (confi);
	g_node_traverse (tree, G_PRE_ORDER, G_TRAVERSE_ALL, -1, traverse_func, NULL);
	g_printf ("\n");

	/*g_printf ("Setting root \"key2\"\n");
	confi_set_root (confi, "key2");
	g_printf ("Value from key \"key2-1\" %s\n", confi_path_get_value (confi, "key2-1"));*/

	confi_destroy (confi);

	return 0;
}
