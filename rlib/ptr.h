/*
 *
 * Copyright (c) 2016 Raphine Project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Author: Liva
 * 
 */

#ifndef __RAPH_LIBRARY_PTR_H__
#define __RAPH_LIBRARY_PTR_H__

#include <raph.h>

template<class T>
class uptr {
public:
  uptr(uptr &p) {
    _obj = p._obj;
    p._obj = nullptr;
  }
  template <class ...Args>
  uptr(Args ...args) {
    _obj = new T(args...);
  }
  ~uptr() {
    delete _obj;
  }
  T *operator&();
  T *operator*();
  T *operator->() {
    if (_obj == nullptr) {
      kassert(false);
    }
    return _obj;
  }
private:
  T *_obj;
};

template<class T>
class sptr {
public:
  sptr(const sptr &p) {
    _obj = p._obj;
    _ref_cnt = p._ref_cnt;
    while(!__sync_bool_compare_and_swap(_ref_cnt, *_ref_cnt, *_ref_cnt + 1)) {
    }
  }
  template <class ...Args>
  sptr(Args ...args) {
    _obj = new T(args...);
    _ref_cnt = new int(1);
  }
  ~sptr() {
    while(!__sync_bool_compare_and_swap(_ref_cnt, *_ref_cnt, *_ref_cnt - 1)) {
    }
    if (*_ref_cnt == 0) {
      delete _obj;
    }
  }
  T *operator&();
  T *operator*();
  T *operator->() {
    return _obj;
  }
private:
  T *_obj;
  int *_ref_cnt;
};

#endif // __RAPH_LIBRARY_PTR_H__
