// desc_parse.cc
//
//  Copyright 2004-2008 Daniel Burrows
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
//
//  Parses a description into a cw::fragment.

#include "desc_render.h"

#include "aptitude.h"
#include "ui.h"

#include <generic/apt/apt.h>
#include <generic/apt/config_signal.h>
#include <generic/apt/tags.h>

#include <cwidget/fragment.h>
#include <cwidget/generic/util/transcode.h>
#include <cwidget/config/colors.h>

using namespace std;

namespace cw = cwidget;

namespace aptitude
{
  namespace
  {
    cw::fragment *make_desc_fragment(const std::vector<description_element_ref> &elements,
				     int level)
    {
      std::vector<cw::fragment *> fragments;

      for(std::vector<description_element_ref>::const_iterator it = elements.begin();
	  it != elements.end(); ++it)
	{
	  const description_element_ref &elt(*it);

	  switch(elt->get_type())
	    {
	    case description_element::blank_line:
	      fragments.push_back(cw::newline_fragment());
	      break;
	    case description_element::paragraph:
	      fragments.push_back(wrapbox(cw::text_fragment(elt->get_string())));
	      break;
	    case description_element::literal:
	      fragments.push_back(cw::hardwrapbox(cw::text_fragment(elt->get_string())));
	      break;
	    case description_element::bullet_list:
	      {
		wstring bullet;
		bullet.push_back(L"*+-"[level%3]);

		cw::fragment *item_contents(make_desc_fragment(elt->get_elements(),
							       level + 1));

		fragments.push_back(cw::style_fragment(cw::text_fragment(bullet),
						       cw::get_style("Bullet")));
		fragments.push_back(cw::indentbox(1,
						  (level + 1) * 2,
						  item_contents));
	      }
	      break;
	    }
	}

      return cw::sequence_fragment(fragments);
    }
  }

  cw::fragment *make_desc_fragment(const std::vector<description_element_ref> &elements)
  {
    return make_desc_fragment(elements, 0);
  }
}

cw::fragment *make_desc_fragment(const wstring &desc)
{
  std::vector<aptitude::description_element_ref> elements;
  aptitude::parse_desc(desc, elements);

  return aptitude::make_desc_fragment(elements);
}


cw::fragment *make_tags_fragment(const pkgCache::PkgIterator &pkg)
{
  if(pkg.end())
    return NULL;

#ifdef HAVE_EPT
  typedef ept::debtags::Tag tag;
  using aptitude::apt::get_tags;
#endif

#ifdef HAVE_EPT
  const set<tag> realS(get_tags(pkg));
  const set<tag> * const s(&realS);
#else
  const set<tag> * const s(get_tags(pkg));
#endif

  vector<cw::fragment *> rval;
  if(s != NULL && !s->empty())
    {
      vector<cw::fragment *> tags;
      for(set<tag>::const_iterator i = s->begin(); i != s->end(); ++i)
	{
#ifdef HAVE_EPT
	  std::string name(i->fullname());
#else
	  const std::string name(i->str());
#endif

	  tags.push_back(cw::text_fragment(name, cw::style_attrs_on(A_BOLD)));
	}

      wstring tagstitle = W_("Tags");

      rval.push_back(cw::fragf("%ls: %F",
			       tagstitle.c_str(),
			       indentbox(0, wcswidth(tagstitle.c_str(), tagstitle.size())+2,
					 wrapbox(cw::join_fragments(tags, L", ")))));
    }

  typedef aptitudeDepCache::user_tag user_tag;
  const set<user_tag> &user_tags((*apt_cache_file)->get_ext_state(pkg).user_tags);
  if(!user_tags.empty())
    {
      vector<cw::fragment *> tags;
      for(set<user_tag>::const_iterator it = user_tags.begin();
	  it != user_tags.end(); ++it)
	{
	  tags.push_back(cw::text_fragment((*apt_cache_file)->deref_user_tag(*it),
					   cw::style_attrs_on(A_BOLD)));
	}

      wstring title = W_("User Tags");
      rval.push_back(dropbox(cw::fragf("%ls: ", title.c_str()),
			     wrapbox(cw::join_fragments(tags, L", "))));
    }

  if(!rval.empty())
    return cw::join_fragments(rval, L"\n");
  else
    return NULL;
}
