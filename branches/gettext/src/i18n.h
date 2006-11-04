/* **************************************************************** -*- C -*- */
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      i18n.h
  Version:   $Rev$
  Author(s): Gilles Caulier (gc) <caulier.gilles@kdemail.net>
  History:   01-Nov-06, gc: created
 */
#ifndef I18N_H_
#define I18N_H_

#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "gettext.h"

#ifdef EXV_ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (EXV_PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  if ! defined(textdomain)
#    define textdomain(String) (String)
#  endif
#  if ! defined(gettext)
#    define gettext(String) (String)
#  endif
#  if ! defined(dgettext)
#    define dgettext(Domain,Message) (Message)
#  endif
#  if ! defined(dcgettext)
#    define dcgettext(Domain,Message,Type) (Message)
#  endif
#  if ! defined(bindtextdomain)
#    define bindtextdomain(Domain,Directory) (Domain)
#  endif
#  if ! defined(_)
#    define _(String) (String)
#  endif
#  if ! defined(N_)
#    define N_(String) (String)
#  endif
#endif

#endif /* I18N_H_ */
