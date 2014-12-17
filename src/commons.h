/*
 *  Copyright (C) 2014 Andrea Zagli <azagli@libero.it>
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

#ifndef __LIBCONFI_COMMONS_H__
#define __LIBCONFI_COMMONS_H__

#include <glib-object.h>


G_BEGIN_DECLS


#define CONFI_TYPE_CONFI (confi_confi_get_type ())

GType confi_confi_get_type ();

typedef struct _ConfiConfi ConfiConfi;
struct _ConfiConfi
	{
		gchar *name;
		gchar *description;
	};

ConfiConfi *confi_confi_copy (ConfiConfi *confi);
void confi_confi_free (ConfiConfi *confi);

#define CONFI_TYPE_KEY (confi_key_get_type ())

GType confi_key_get_type ();

typedef struct _ConfiKey ConfiKey;
struct _ConfiKey
	{
		gint id_config;
		gint id;
		gint id_parent;
		gchar *key;
		gchar *value;
		gchar *description;
		gchar *path;
	};

ConfiKey *confi_key_copy (ConfiKey *key);
void confi_key_free (ConfiKey *key);


G_END_DECLS

#endif /* __LIBCONFI_COMMONS_H__ */
