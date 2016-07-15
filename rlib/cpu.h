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

#ifndef __RAPH_LIB_CPU_H__
#define __RAPH_LIB_CPU_H__

class CpuCtrlInterface {
public:
  virtual ~CpuCtrlInterface() {
  }
  virtual volatile int GetId() = 0;
  virtual int GetHowManyCpus() = 0;
  bool IsValidId(int cpuid) {
    return (cpuid >= 0 && cpuid < GetHowManyCpus());
  }
};

#ifdef __KERNEL__
#include <apic.h>
#include <global.h>

class CpuCtrl : public CpuCtrlInterface {
public:
  virtual volatile int GetId() override {
    return apic_ctrl->GetCpuId();
  }
  virtual int GetHowManyCpus() override {
    return apic_ctrl->GetHowManyCpus();
  }
};
#else
#include <thread.h>
#endif /* __KERNEL__ */

#endif /* __RAPH_LIB_CPU_H__ */
