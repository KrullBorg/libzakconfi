/*
 * commons.c
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

#include "commons.h"

ZakConfiConfi
*zak_confi_confi_copy (ZakConfiConfi *confi)
{
	ZakConfiConfi *b;

	b = g_slice_new (ZakConfiConfi);
	b->name = g_strdup (confi->name);
	b->description = g_strdup (confi->description);

	return b;
}

void
zak_confi_confi_free (ZakConfiConfi *confi)
{
	g_free (confi->name);
	g_free (confi->description);
	g_slice_free (ZakConfiConfi, confi);
}

G_DEFINE_BOXED_TYPE (ZakConfiConfi, zak_confi_confi, zak_confi_confi_copy, zak_confi_confi_free)

ZakConfiKey
*zak_confi_key_copy (ZakConfiKey *key)
{
	ZakConfiKey *b;

	b = g_slice_new (ZakConfiKey);
	b->config = g_strdup (key->config);
	b->path = g_strdup (key->path);
	b->key = g_strdup (key->key);
	b->value = g_strdup (key->value);
	b->description = g_strdup (key->description);

	return b;
}

void
zak_confi_key_free (ZakConfiKey *key)
{
	g_free (key->config);
	g_free (key->path);
	g_free (key->key);
	g_free (key->value);
	g_free (key->description);
	g_slice_free (ZakConfiKey, key);
}

G_DEFINE_BOXED_TYPE (ZakConfiKey, zak_confi_key, zak_confi_key_copy, zak_confi_key_free)
