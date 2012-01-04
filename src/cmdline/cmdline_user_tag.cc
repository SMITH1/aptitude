// cmdline_user_tag.cc
//
//   Copyright (C) 2008-2010 Daniel Burrows
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


// Local includes:
#include "cmdline_user_tag.h"

#include "cmdline_util.h"
#include "terminal.h"
#include "text_progress.h"

#include <aptitude.h>

#include <generic/apt/apt.h>
#include <generic/apt/aptcache.h>
#include <generic/apt/config_signal.h>
#include <generic/apt/matching/match.h>
#include <generic/apt/matching/parse.h>
#include <generic/apt/matching/pattern.h>


// System includes:
#include <apt-pkg/error.h>
#include <apt-pkg/cmndline.h>

#include <stdio.h>
#include <string.h>

using aptitude::cmdline::create_terminal;
using aptitude::cmdline::make_text_progress;
using aptitude::cmdline::terminal_io;
using boost::shared_ptr;

namespace aptitude
{
  namespace cmdline
  {
    namespace
    {
      enum user_tag_action { action_add, action_remove };

      void do_user_tag(user_tag_action act,
		       const std::string &tag,
		       const pkgCache::PkgIterator &pkg,
		       int verbose)
      {
	switch(act)
	  {
	  case action_add:
	    if(verbose > 0)
				// Sometimes also "user-tag" is used!
	      printf(_("Adding user tag \"%s\" to the package \"%s\".\n"),
		     tag.c_str(), pkg.Name());

	    (*apt_cache_file)->attach_user_tag(pkg, tag, NULL);
	    break;
	  case action_remove:
	    if(verbose > 0)
	      printf(_("Removing user tag \"%s\" from the package \"%s\".\n"),
		     tag.c_str(), pkg.Name());

	    (*apt_cache_file)->detach_user_tag(pkg, tag, NULL);
	    break;
	  default:
	    fprintf(stderr, "Internal error: bad user tag action %d.", act);
	    break;
	  }
      }
    }

    bool cmdline_user_tag(CommandLine &cmdl)
    {
      const int argc = cmdl.FileSize();
      const shared_ptr<terminal_io> term = create_terminal();

      user_tag_action action = (user_tag_action)-1;

      if(strcmp(cmdl.FileList[0], "add-user-tag") == 0)
	action = action_add;
      else if(strcmp(cmdl.FileList[0], "remove-user-tag") == 0)
	action = action_remove;
      else
	{
	  fprintf(stderr, "Internal error: cmdline_user_tag encountered an unknown command name \"%s\"\n",
		  cmdl.FileList[0]);
	  abort();
	}

      if(argc < 3)
        return _error->Error(_("%s: too few arguments; expected at least a tag name and a package"),
                             cmdl.FileList[0]);

      _error->DumpErrors();

      OpProgress progress;

      apt_init(&progress, true);
      if(_error->PendingError())
        return false;

      std::string tag(cmdl.FileList[1]);

      const int quiet = aptcfg->FindI("quiet", 0);
      const int verbose = aptcfg->FindI(PACKAGE "::CmdLine::Verbose", 0);

      bool all_ok = true;
      for(int i = 2; i < argc; ++i)
	{
	  if(!aptitude::matching::is_pattern(cmdl.FileList[i]))
	    {
	      pkgCache::PkgIterator pkg = (*apt_cache_file)->FindPkg(cmdl.FileList[i]);
	      if(pkg.end())
		{
		  if(quiet == 0)
		    fprintf(stderr, _("No such package \"%s\".\n"), cmdl.FileList[i]);
		  all_ok = false;
		}
	      else
		do_user_tag(action, tag, pkg, verbose);
	    }
	  else
	    {
	      using namespace aptitude::matching;
	      using cwidget::util::ref_ptr;

	      ref_ptr<pattern> p(parse(cmdl.FileList[i]));

	      if(!p.valid())
		{
		  _error->DumpErrors();
		  all_ok = false;
		}
	      else
		{
		  std::vector<std::pair<pkgCache::PkgIterator, ref_ptr<structural_match> > > matches;
		  ref_ptr<search_cache> search_info(search_cache::create());
		  search(p, search_info,
			 matches,
			 *apt_cache_file,
			 *apt_package_records);

		  for(std::vector<std::pair<pkgCache::PkgIterator, ref_ptr<structural_match> > >::const_iterator
			it = matches.begin(); it != matches.end(); ++it)
		    do_user_tag(action, tag, it->first, verbose);
		}
	    }
	}

      shared_ptr<OpProgress> text_progress = make_text_progress(false, term, term, term);
      if(!(*apt_cache_file)->save_selection_list(*text_progress))
	exit(1);

      if(!all_ok)
	exit(2);

      return 0;
    }
  }
}
