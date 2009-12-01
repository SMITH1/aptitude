/** \file area.h */    // *-c++-*-

// Copyright (C) 2009 Daniel Burrows
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.

#ifndef AREA_H
#define AREA_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <gtkmm/image.h>

#include <sigc++/signal.h>

// "areas" are the broad functional categories that tabs are grouped
// into.  Each area contains one or more "tabs" (views the user can
// select), along with one or more "notifications" (warnings for the
// user, or indications of the progress of an ongoing background
// operation).
//
// This file defines the abstractions used to describe the areas of
// all active tabs and ongoing notifications within each area.
//
// Areas aren't especially performance-critical, so I've hidden the
// implementations behind abstract interfaces in order to better
// insulate the rest of the code from the implementation details.
//
// Areas are not thread-safe: only the main GUI thread should read and
// write them.

namespace gui
{
  class notification_info;
  class tab_info;

  /** \brief Utility class to allow generic iteration over a list.
   *
   *  Unlike an iterator, instantiations of this class can be used
   *  without the definition of the concrete object being visible to
   *  the user.  Like Java and .NET iterators, these iterators start
   *  "before" the list being iterated over and must be advanced to
   *  the first entry.
   */
  template<typename T>
  class enumerator
  {
  public:
    /** \brief Get the value at the current position. */
    T get_current() = 0;

    /** \brief Advance to the next entry in the list.
     *
     *  \return \b true if there was another entry, or \b false if we
     *  reached the end of the list.
     */
    bool advance() = 0;
  };

  /** \brief The abstract description of an area. */
  class area_info
  {
  public:
    /** \brief Get the name of this area (e.g., "Upgrade" or
     *	"Search").
     */
    virtual std::string get_name() = 0;

    /** \brief Get a description of this area. */
    virtual std::string get_description() = 0;

    /** \brief Get the icon of this area. */
    virtual Glib::RefPtr<Gdk::Pixbuf> get_icon() = 0;


    typedef enumerator<boost::shared_ptr<tab_info> > tab_enumerator;

    /** \brief Enumerate the tabs in this area.
     *
     *  To get a consistent picture of the tabs, the caller should
     *  enumerate them before any other process adds or removes a tab.
     *  Typically this means enumerating them in a tight loop.
     */
    virtual boost::shared_ptr<tab_enumerator> get_tabs() = 0;

    /** \brief Append a tab to this area's tab list. */
    virtual void append_tab(const boost::shared_ptr<tab_info> &tab) = 0;

    /** \brief Remove a tab from this area's tab list. */
    virtual void remove_tab(const boost::shared_ptr<tab_info> &tab) = 0;


    /** \brief Make the given tab the currently active tab in this area. */
    virtual void set_active_tab(const boost::shared_ptr<tab_info> &tab) = 0;

    /** \brief Retrieve the currently active tab in this area. */
    virtual boost::shared_ptr<tab_info> get_active_tab() = 0;



    typedef enumerator<boost::shared_ptr<notification_info> > notification_enumerator;

    /** \brief Enumerate the notifications in this area.
     *
     *  To get a consistent picture of the notifications, the caller
     *  should enumerate them before any other process adds or removes
     *  a notification.  Typically this means enumerating them in a
     *  tight loop.
     */
    virtual boost::shared_ptr<notification_enumerator> get_notifications() = 0;

    /** \brief Append a notification to this area's notification list. */
    virtual void append_notification(const boost::shared_ptr<notification_info> &notification) = 0;

    /** \brief Remove a notification from this area's notification list. */
    virtual void remove_notification(const boost::shared_ptr<notification_info> &notification) = 0;


    /** \brief Signals */
    // @{

    /** \brief Emitted when a tab is appended to the area's tab list. */
    sigc::signal<void, boost::shared_ptr<tab_info> > signal_tab_appended;

    /** \brief Emitted when a tab is removed from the area's tab list. */
    sigc::signal<void, boost::shared_ptr<tab_info> > signal_tab_removed;


    /** \brief Emitted when the currently active tab changes.
     *
     *  signal_active_tab_changed(from, to)
     *
     *  The first parameter is the previously active tab; the second
     *  is the new active tab.
     */
    sigc::signal<void, boost::shared_ptr<tab_info>, boost::shared_ptr<tab_info> > signal_active_tab_changed;


    /** \brief Emitted when a notification is appended to the area's notification list. */
    sigc::signal<void, boost::shared_ptr<notification_info> > signal_notification_appended;

    /** \brief Emitted when a notification is removed from the area's notification list. */
    sigc::signal<void, boost::shared_ptr<notification_info> > signal_notification_removed;

    // @}
  };

  boost::shared_ptr<area_info> create_area_info(const std::string &name,
						const Gtk::Image *icon);

  /** \brief The abstract description of a tab. */
  class tab_info
  {
  public:
    /** \brief Get the name of this tab. */
    virtual std::string get_name() = 0;

    /** \brief Get tooltip information for this tab.
     *
     *  \param tooltip_text A location in which to store the text of the
     *                      tooltip.
     *  \param tooltip_window A location in which to store a pointer to
     *                        a window that should be used to display
     *                        the tooltip.
     *
     *  tooltip_text is empty if there is no text; tooltip_window is
     *  NULL if there is no window.
     */
    virtual void get_tooltip(std::string &tooltip_text,
			     Gtk::Window * &tooltip_window) = 0;

    /** \brief Set the tooltip text for this tab.
     *
     *  Automatically deletes and clears the tooltip window.
     */

    /** \brief Get the icon of this tab. */
    virtual Glib::RefPtr<Gdk::Pixbuf> get_icon();

    /** \brief Get the main widget of this tab. */
    virtual Gtk::Widget *get_widget();


  };
}

#endif // AREA_H

