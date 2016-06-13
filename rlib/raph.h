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

#ifndef __RAPH_KERNEL_RAPH_H__
#define __RAPH_KERNEL_RAPH_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
template<class T>
static inline T align(T val, int base) {
  return (val / base) * base;
}

template<class T>
static inline T alignUp(T val, int base) {
  return align(val + base - 1, base);
}

inline void *operator new  (size_t, void *p)   throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __KERNEL__
#define kassert(flag) if (!(flag)) {throw #flag;}

#else
#undef kassert
  
  [[noreturn]] void _kassert(const char *file, int line, const char *func);
#define kassert(flag) if (!(flag)) { _kassert(__FILE__, __LINE__, __func__); }

#endif /* __KERNEL__ */

  
#define MASK(val, ebit, sbit) ((val) & (((1 << ((ebit) - (sbit) + 1)) - 1) << (sbit)))
  
  void kernel_panic(const char *class_name, const char *err_str);

  void checkpoint(int id, const char *str);
  void _checkpoint(const char *func, const int line);
#define CHECKPOINT _checkpoint(__func__, __LINE__)

  static inline void outb(uint16_t pin, uint8_t data) {
    asm volatile("outb %%al, %%dx;"::"d"(pin),"a"(data));
  }

  static inline void outw(uint16_t pin, uint16_t data) {
    asm volatile("outw %%ax, %%dx;"::"d"(pin),"a"(data));
  }

  static inline void outl(uint16_t pin, uint32_t data) {
    asm volatile("outl %%eax, %%dx;"::"d"(pin),"a"(data));
  }

  static inline uint8_t inb(uint16_t pin) {
    uint8_t data;
    asm volatile("inb %%dx, %%al;":"=a"(data):"d"(pin));
    return data;
  }

  static inline uint16_t inw(uint16_t pin) {
    uint16_t data;
    asm volatile("inw %%dx, %%ax;":"=a"(data):"d"(pin));
    return data;
  }

  static inline uint32_t inl(uint16_t pin) {
    uint32_t data;
    asm volatile("inl %%dx, %%eax;":"=a"(data):"d"(pin));
    return data;
  }

#ifdef __cplusplus
}
#endif

#endif /* __RAPH_KERNEL_RAPH_H__ */
