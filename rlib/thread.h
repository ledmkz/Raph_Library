/*
 *
 * Copyright (c) 2016 Project Raphine
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
 * Author: Levelfour
 * 
 */

#ifndef __RAPH_LIB_THREAD_H__
#define __RAPH_LIB_THREAD_H__

#ifndef __KERNEL__

#include <vector>
#include <memory>
#include <thread>
#include <stdint.h>
#include <apic.h>

class PthreadCtrl : public ApicCtrlInterface {
public:
  PthreadCtrl() : _thread_pool(0) {}
  PthreadCtrl(int num_threads) : _cpu_nums(num_threads), _thread_pool(num_threads-1) {}
  ~PthreadCtrl() {
    for(auto th: _thread_pool) {
      th->detach();
    }
  }
  virtual void Setup() override {
    for(int i = 0; i < _thread_pool.size(); i++) {
      _thread_pool[i] = new std::thread ([]{
          task_ctrl->Run();
      });
    }
  }
  virtual volatile int GetCpuId() override {
    return 0;
  }
  virtual int GetHowManyCpus() override {
    return _cpu_nums;
  }
  virtual void SetupTimer() override {
  }
  virtual void StartTimer() override {
  }
  virtual void StopTimer() override {
  }

private:
  int _cpu_nums = 1;
  std::vector<std::unique_ptr<std::thread>> _thread_pool;
};

#endif // !__KERNEL__

#endif /* __RAPH_LIB_THREAD_H__ */
