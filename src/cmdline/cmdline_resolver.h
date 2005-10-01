// cmdline_resolver.h                              -*-c++-*-
//
//   Copyright (C) 2005 Daniel Burrows
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.

#ifndef CMDLINE_RESOLVER_H
#define CMDLINE_RESOLVER_H

#include "cmdline_common.h"

/** Run the resolver once, possibly prompting the user in the process.
 *
 *  \param to_install a list of packages which the user explicitly
 *  asked to install
 *
 *  \param to_hold a list of packages which the user explicitly asked
 *  to hold back
 *
 *  \param to_remove a list of packages which the user explicitly
 *  asked to remove
 *
 *  \param to_purge a list of packages which the user explicitly asked
 *  to purge
 *
 *  \param assume_yes if \b true, try to find a single solution
 *  (regardless of how long it takes) and accept it immediately.
 *
 *  \param force_no_change if \b true, assign extra version scores to
 *  heavily bias the resolver against changing any packages in the
 *  supplied sets.
 *  \param verbose the verbosity level set by the user
 */
bool cmdline_resolve_deps(pkgset &to_install,
			  pkgset &to_hold,
			  pkgset &to_remove,
			  pkgset &to_purge,
			  bool assume_yes,
			  bool force_no_change,
			  int verbose);

#endif // CMDLINE_RESOLVER_H
