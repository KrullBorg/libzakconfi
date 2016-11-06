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

int
main (int argc, char **argv)
{
	PeasEngine *engine;
	PeasPluginInfo *ppinfo;

	if (argc < 4)
		{
			g_error ("Usage: test_add_config <connection string> <config name> <config description>");
			return 0;
		}

	engine = peas_engine_get_default ();
	peas_engine_add_search_path (engine, "./plugins", NULL);

	if (zak_confi_add_config (argv[1], argv[2], argv[3]) == NULL)
		{
			g_warning ("Config %s not created.", argv[1]);
		}

	return 0;
}
