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

#ifndef __RAPH_LIB_TASK_H__
#define __RAPH_LIB_TASK_H__

#include <global.h>
#include <function.h>
#include <spinlock.h>

class Task;

class TaskCtrl {
public:
  enum class TaskQueueState {
    kNotRunning,
    kRunning,
    kSleeped,
  };
  TaskCtrl() {}
  void Setup();
  void Register(int cpuid, Task *task);
  void Register(int cpuid, Task *task, int us);
  // void Remove(int cpuid, Task *task);
  void Run();
  TaskQueueState GetState(int cpuid) {
    if (_task_struct == nullptr) {
      return TaskQueueState::kNotRunning;
    }
    return _task_struct[cpuid].state;
  }
 private:
  void ForceWakeup(int cpuid);
  struct TaskStruct {
    // queue
    Task *top;
    Task *bottom;
    Task *top_sub;
    Task *bottom_sub;
    IntSpinLock lock;

    TaskQueueState state;
  } *_task_struct = nullptr;
};

class Task {
public:
  Task() {
  }
  virtual ~Task();
  void SetFunc(const GenericFunction &func) {
    _func.Copy(func);
  }
  enum class Status {
    kRunning,
    kWaitingInQueue,
    kOutOfQueue,
    kGuard,
  };
  Status GetStatus() {
    return _status;
  }
private:
  void Execute() {
    _func.Execute();
  }
  FunctionBase _func;
  Task *_next;
  Task *_prev;
  Status _status = Status::kOutOfQueue;
  friend TaskCtrl;
};
 
// Taskがキューに積まれている間にインクリメント可能
// 割り込み内からも呼び出せる
// ただし、一定時間後に立ち上げる事や割り当てcpuidを変える事はできない
class CountableTask {
public:
  CountableTask() {
    _cnt = 0;
    _cpuid = -1;
    ClassFunction<CountableTask> func;
    func.Init(this, &CountableTask::HandleSub, nullptr);
    _task.SetFunc(func);
  }
  virtual ~CountableTask() {
  }
  void SetFunc(int cpuid, const GenericFunction &func) {
    _cpuid = cpuid;
    _func.Copy(func);
  }
  Task::Status GetStatus() {
    return _task.GetStatus();
  }
  void Inc();
private:
  void HandleSub(void *);
  Task _task;
  IntSpinLock _lock;
  FunctionBase _func;
  int _cnt;
  int _cpuid;
};

#endif /* __RAPH_LIB_TASK_H__ */
