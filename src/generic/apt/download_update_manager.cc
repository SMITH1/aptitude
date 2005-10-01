// download_update_manager.cc
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

#include "download_update_manager.h"

#include "apt.h"
#include "config_signal.h"
#include "download_signal_log.h"

#include <apt-pkg/clean.h>
#include <apt-pkg/error.h>
#include <apt-pkg/sourcelist.h>

class my_cleaner:public pkgArchiveCleaner
{
protected:
  virtual void Erase(const char *file,
		     string pkg,
		     string ver,
		     struct stat &stat)
  {
    unlink(file);
  }
};

download_update_manager::download_update_manager()
  : log(NULL)
{
}

download_update_manager::~download_update_manager()
{
}

bool download_update_manager::prepare(OpProgress &progress,
				      pkgAcquireStatus &acqlog,
				      download_signal_log *signallog)
{
  log = signallog;

  pkgSourceList src_list;

  if(apt_cache_file != NULL &&
     !(*apt_cache_file)->save_selection_list(progress))
    return false;

  // FIXME: should save_selection_list do this?
  progress.Done();

  if(src_list.ReadMainList() == false)
    {
      _error->Error(_("Couldn't read list of package sources"));
      return false;
    }

  // Lock the list directory
  FileFd lock;
  if(aptcfg->FindB("Debug::NoLocking", false) == false)
    {
      lock.Fd(GetLock(aptcfg->FindDir("Dir::State::Lists")+"lock"));
      if(_error->PendingError() == true)
	{
	  _error->Error(_("Couldn't lock list directory..are you root?"));
	  return false;
	}
    }

  fetcher = new pkgAcquire(&acqlog);

  if(!src_list.GetIndexes(fetcher))
    {
      delete fetcher;
      fetcher = NULL;
      return false;
    }
  else
    return true;
}

download_manager::result
download_update_manager::finish(pkgAcquire::RunResult res,
				OpProgress &progress)
{
  if(log != NULL)
    log->Complete();

  if(res != pkgAcquire::Continue)
    {
      apt_reload_cache(&progress, true);
      return failure;
    }

  // Clean old stuff out
  if(fetcher->Clean(aptcfg->FindDir("Dir::State::lists")) == false ||
     fetcher->Clean(aptcfg->FindDir("Dir::State::lists")+"partial/") == false)
    {
      _error->Error(_("Couldn't clean out list directories"));
      return failure;
    }

  apt_reload_cache(&progress, true);

  if(apt_cache_file != NULL &&
     aptcfg->FindB(PACKAGE "::Forget-New-On-Update", false))
    {
      (*apt_cache_file)->forget_new(NULL);
      post_forget_new_hook();
    }

  if(apt_cache_file != NULL &&
     aptcfg->FindB(PACKAGE "::AutoClean-After-Update", false))
    {
      pre_autoclean_hook();

      my_cleaner cleaner;
      cleaner.Go(aptcfg->FindDir("Dir::Cache::archives"), *apt_cache_file);
      cleaner.Go(aptcfg->FindDir("Dir::Cache::archives")+"partial/",
		 *apt_cache_file);

      post_autoclean_hook();
    }

  return success;
}

