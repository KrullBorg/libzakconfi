/*
 *  Copyright (C) 2014-2016 Andrea Zagli <azagli@libero.it>
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

#ifndef __LIBZAKCONFI_COMMONS_H__
#define __LIBZAKCONFI_COMMONS_H__

#include <glib-object.h>


G_BEGIN_DECLS


#define ZAK_CONFI_TYPE_CONFI (zak_confi_confi_get_type ())

GType zak_confi_confi_get_type ();

typedef struct _ZakConfiConfi ZakConfiConfi;
struct _ZakConfiConfi
	{
		gchar *name;
		gchar *description;
	};

ZakConfiConfi *zak_confi_confi_copy (ZakConfiConfi *confi);
void zak_confi_confi_free (ZakConfiConfi *confi);

#define ZAK_CONFI_TYPE_KEY (zak_confi_key_get_type ())

GType zak_confi_key_get_type ();

typedef struct _ZakConfiKey ZakConfiKey;
struct _ZakConfiKey
	{
		gchar *config;
		gchar *path;
		gchar *key;
		gchar *value;
		gchar *description;
	};

ZakConfiKey *zak_confi_key_copy (ZakConfiKey *key);
void zak_confi_key_free (ZakConfiKey *key);


G_END_DECLS

#endif /* __LIBZAKCONFI_COMMONS_H__ */
