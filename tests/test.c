/*
 * Copyright (C) 2005-2006 Andrea Zagli <azagli@inwind.it>
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

#include <libgda/libgda.h>
#include <libconfi.h>

gboolean
traverse_func (GNode *node,
               gpointer data)
{
	ConfiKey *ck = (ConfiKey *)node->data;
	if (ck->id != 0)
		{
			g_fprintf (stderr, "%s%s%s\n", ck->path, strcmp (ck->path, "") == 0 ? "" : "/", ck->key);
		}

	return FALSE;
}

int
main (int argc, char **argv)
{
	GdaClient *gda_client;
	Confi *confi;
	GNode *tree;

	gda_init ("test", "0.0.1", argc, argv);
	gda_client = gda_client_new ();
	if (gda_client == NULL)
		{
			g_fprintf (stderr, "Errore nell'inizializzazione del gda_client\n");
			return 0;
		}

	confi = confi_new (gda_client, "PostgreSQL", "HOSTADDR=127.0.0.1;PORT=5432;DATABASE=confi;HOST=localhost;USER=postgres", "Default", NULL, FALSE);
	if (confi == NULL)
		{
			g_fprintf (stderr, "Errore nell'inizializzazione della configurazione\n");
			return 0;
		}

	g_fprintf (stderr, "Value from key \"folder/key1/key1_2\"\n%s\n\n", confi_path_get_value (confi, "folder/key1/key1_2"));

	g_fprintf (stderr, "Traversing the entire tree\n");
	tree = confi_get_tree (confi);
	g_node_traverse (tree, G_PRE_ORDER, G_TRAVERSE_ALL, -1, traverse_func, NULL);
	g_fprintf (stderr, "\n");

	g_fprintf (stderr, "Setting root \"key2\"\n");
	confi_set_root (confi, "key2");
	g_fprintf (stderr, "Value from key \"key2-1\" %s\n", confi_path_get_value (confi, "key2-1"));

	confi_destroy (confi);

	return 0;
}
