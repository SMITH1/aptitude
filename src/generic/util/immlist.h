/** \file immlist.h */   // -*-c++-*-


//   Copyright (C) 2009 Daniel Burrows
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

#include <cwidget/generic/util/ref_ptr.h>
#include "refcounted_base.h"

namespace imm
{
  /** \brief Immutable list, using the standard head/tail breakdown.
   *
   *  The empty list is represented by a default-constructed ref_ptr.
   *
   *  Like imm::set, this is not threadsafe because it exists for use
   *  in some moderately intensive pieces of code that are
   *  single-threaded.
   */
  template<typename T>
  class list
  {
  public:
    typedef unsigned int size_type;

  private:
    class node : public aptitude::util::refcounted_base_not_threadsafe
    {
      T head;

      cwidget::util::ref_ptr<list> tail;

      size_type size;

    public:
      node(const T &_head, const cwidget::util::ref_ptr<list> &_tail)
	: head(_head), tail(_tail), size(_tail->size() + 1)
      {
      }

      const T &get_head() const { return head; }
      const cwidget::util::ref_ptr<list> &get_tail() const { return tail; }
      size_type get_size() const { return size; }
    };

    cwidget::util::ref_ptr<node> lst;

    list()
      : lst()
    {
    }

    list(const T &head, const list &tail)
      : lst(new node(head, tail))
    {
    }

  public:
    static list make_empty() { return list(); }
    static list make_cons(const T &head, const list &tail)
    {
      return list(head, tail);
    }

    /** \brief Check whether the list is empty. */
    bool empty() const { return !lst.valid(); }
    /** \brief Retrieve the size of this list.
     *
     *  We store sizes in list nodes, so this is O(1).  We can do this
     *  only because the list is immutable.
     */
    size_type size() const
    {
      if(lst.valid())
	return lst->get_size();
      else
	return 0;
    }

    /** \brief Add a new value to the front of this list. */
    void push_front(const T &t)
    {
      lst = cwidget::util::ref_ptr<node>(new node(t, lst));
    }

    /** \brief Remove the first value from this list.
     *
     *  Undefined behavior if the list is empty.
     */
    void pop_front()
    {
      eassert(lst.valid());

      lst = lst->get_tail();
    }

    /** \brief Retrieve the first value from this list.
     *
     *  Undefined behavior if the list is empty.
     */
    const T &front() const
    {
      eassert(lst.valid());

      return lst->get_head();
    }

    /** \brief Iterates down a single imm::list.
     *
     *  Unlike the imm::set iterator, this one should be quite
     *  efficient, albeit still slightly less efficient than an
     *  intrinsic for_each().
     */
    class const_iterator
    {
      cwidget::util::ref_ptr<node> lst;

    public:
      /** \brief Construct a const_iterator that iterates down the
       *  given list.
       */
      const_iterator(const cwidget::util::ref_ptr<node> &_lst)
	: lst(_lst)
      {
      }

      /** \brief Construct a const_iterator pointing at the empty
       *  list.
       */
      const_iterator()
      {
      }

      /** \brief Return \b true if the iterator points at a valid
       *  member of the list.
       */
      bool valid() const { return lst.valid(); }

      const_iterator &operator++()
      {
	lst = lst->get_tail();
      }

      const T &operator*() const { return lst->get_head(); }
      const T *operator->() const { return lst->get_tail().operator->(); }
    };

    /** \brief Retrieve an iterator pointing to the front of the list.
     */
    const_iterator begin() const
    {
      return const_iterator(lst);
    }

    /** \brief Retrieve an iterator pointing past the end of the list.
     */
    const_iterator end() const
    {
      return const_iterator();
    }
  };
}