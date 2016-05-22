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

#include <task.h>
#include <cpu.h>

#ifdef __KERNEL__
#include <apic.h>
#else
#include <unistd.h>
#endif // __KERNEL__

void TaskCtrl::Setup() {
  int cpus = cpu_ctrl->GetHowManyCpus();
  _task_struct = reinterpret_cast<TaskStruct *>(virtmem_ctrl->Alloc(sizeof(TaskStruct) * cpus));
  Task *t;
  for (int i = 0; i < cpus; i++) {
    new(&_task_struct[i]) TaskStruct;

    t = virtmem_ctrl->New<Task>();
    t->_status = Task::Status::kGuard;
    t->_next = nullptr;
    t->_prev = nullptr;

    _task_struct[i].top = t;
    _task_struct[i].bottom = t;

    t = virtmem_ctrl->New<Task>();
    t->_status = Task::Status::kGuard;
    t->_next = nullptr;
    t->_prev = nullptr;

    _task_struct[i].top_sub = t;
    _task_struct[i].bottom_sub = t;

    _task_struct[i].state = TaskQueueState::kNotRunning;

    DefferedTask *dt = virtmem_ctrl->New<DefferedTask>();
    t->_next = nullptr;
    _task_struct[i].dtop = dt;
  }
}

// void TaskCtrl::Remove(int cpuid, Task *task) {
//   //TODO taskにcpuidを突っ込むべき
//   kassert(task->_status != Task::Status::kGuard);
//   Locker locker(_task_struct[cpuid].lock);
//   switch(task->_status) {
//   case Task::Status::kWaitingInQueue: {
//     Task *next = task->_next;
//     Task *prev = task->_prev;

//     task->_next = nullptr;
//     task->_prev = nullptr;

//     kassert(prev != nullptr);
//     prev->_next = next;

//     if (next == nullptr) {
//       if (task == _task_struct[cpuid].bottom) {
//         _task_struct[cpuid].bottom = prev;
//       } else if (task == _task_struct[cpuid].bottom_sub) {
//         _task_struct[cpuid].bottom_sub = prev;
//       } else {
//         kassert(false);
//       }
//     }

//     prev->_next = next;
//     next->_prev = prev;
//     break;
//   }
//   case Task::Status::kRunning:
//   case Task::Status::kOutOfQueue: {
//     break;
//   }
//   default:{
//     kassert(false);
//   }
//   }
//   task->_status = Task::Status::kOutOfQueue;  
// }

void TaskCtrl::Run() {
  int cpuid = cpu_ctrl->GetId();
#ifdef __KERNEL__
  apic_ctrl->SetupTimer();
#endif // __KERNEL__
  while(true) {
    {
      Locker locker(_task_struct[cpuid].lock);
#ifdef __KERNEL__
      apic_ctrl->StopTimer();
#endif // __KERNEL__
      kassert(_task_struct[cpuid].state == TaskQueueState::kNotRunning
              || _task_struct[cpuid].state == TaskQueueState::kSleeped);
      _task_struct[cpuid].state = TaskQueueState::kRunning;
    }
    {
      uint64_t time = timer->GetCntAfterPeriod(timer->ReadMainCnt(), kTaskExecutionInterval);
      
      DefferedTask *dt = _task_struct[cpuid].dtop;
      while(dt->_next != nullptr) {
	DefferedTask *dtt;
	{
	  Locker locker(_task_struct[cpuid].dlock);
	  dtt = dt->_next;
	  if (timer->IsGreater(dtt->_time, time)) {
	    break;
	  }
	  dt->_next = dtt->_next;
	}
	Register(cpuid, &dtt->_task);
      }
    }
    while (true){
      Task *t;
      {
        Locker locker(_task_struct[cpuid].lock);
        Task *tt = _task_struct[cpuid].top;
        t = tt->_next;
        if (t == nullptr) {
          kassert(tt == _task_struct[cpuid].bottom);
          break;
        }
        tt->_next = t->_next;
        if (t->_next == nullptr) {
          kassert(_task_struct[cpuid].bottom == t);
          _task_struct[cpuid].bottom = tt;
        } else {
          t->_next->_prev = tt;
        }
        kassert(t->_status == Task::Status::kWaitingInQueue);
        t->_status = Task::Status::kRunning;
        t->_next = nullptr;
        t->_prev = nullptr;
      }
      
      t->Execute();

      {
        Locker locker(_task_struct[cpuid].lock);
        if (t->_status == Task::Status::kRunning) {
          t->_status = Task::Status::kOutOfQueue;
        }
      }
    }
    {
      Locker locker(_task_struct[cpuid].lock);
      Task *tmp;
      tmp = _task_struct[cpuid].top;
      _task_struct[cpuid].top = _task_struct[cpuid].top_sub;
      _task_struct[cpuid].top_sub = tmp;

      tmp = _task_struct[cpuid].bottom;
      _task_struct[cpuid].bottom = _task_struct[cpuid].bottom_sub;
      _task_struct[cpuid].bottom_sub = tmp;

      if (_task_struct[cpuid].top->_next != nullptr || _task_struct[cpuid].top_sub->_next != nullptr) {
        _task_struct[cpuid].state = TaskQueueState::kNotRunning;
      } else {
        _task_struct[cpuid].state = TaskQueueState::kSleeped;
      }
    }
#ifdef __KERNEL__
    if (_task_struct[cpuid].state == TaskQueueState::kNotRunning) {
      apic_ctrl->StartTimer();
    }
    asm volatile("hlt");
#else
    usleep(10);
#endif // __KERNEL__
  }
}

void TaskCtrl::Register(int cpuid, Task *task) {
  if (!cpu_ctrl->IsValidId(cpuid)) {
    return;
  }
  Locker locker(_task_struct[cpuid].lock);
  if (task->_status == Task::Status::kWaitingInQueue) {
    return;
  }
  task->_next = nullptr;
  task->_status = Task::Status::kWaitingInQueue;
  _task_struct[cpuid].bottom_sub->_next = task;
  task->_prev = _task_struct[cpuid].bottom_sub;
  _task_struct[cpuid].bottom_sub = task;
  
  ForceWakeup(cpuid);
}

void TaskCtrl::RegisterDefferedTask(int cpuid, DefferedTask *task) {
  if (!cpu_ctrl->IsValidId(cpuid)) {
    return;
  }
  Locker locker(_task_struct[cpuid].dlock);
  
  DefferedTask *dt = _task_struct[cpuid].dtop;
  while(dt->_next != nullptr) {
    DefferedTask *dtt = dt->_next;
    if (timer->IsGreater(dtt->_time, task->_time)) {
      task->_next = dtt;
      dt->_next = task;
      break;
    }
    dt = dtt;
  }

  ForceWakeup(cpuid);
}

void TaskCtrl::ForceWakeup(int cpuid) {
#ifdef __KERNEL__
  if (_task_struct[cpuid].state == TaskQueueState::kSleeped) {
    if (cpu_ctrl->GetId() != cpuid) {
      apic_ctrl->SendIpi(apic_ctrl->GetApicIdFromCpuId(cpuid));
    }
  }
#endif // __KERNEL__
}

Task::~Task() {
  kassert(_status == Status::kOutOfQueue);
}

void CountableTask::Inc() {
  if (!cpu_ctrl->IsValidId(_cpuid)) {
    return;
  }
  //TODO CASを使って高速化
  Locker locker(_lock);
  _cnt++;
  if (_cnt == 1) {
    task_ctrl->Register(_cpuid, &_task);
  }
}

void CountableTask::HandleSub(void *) {
  _func.Execute();
  {
    Locker locker(_lock);
    _cnt--;
    if (_cnt != 0) {
      task_ctrl->Register(_cpuid, &_task);
    }
  }
}

void DefferedTask::Register(int cpuid, int us) {
  Locker locker(_lock);
  kassert(!_is_registered);
  _is_registered = true;
  _time = timer->GetCntAfterPeriod(timer->ReadMainCnt(), us);
  task_ctrl->RegisterDefferedTask(cpuid, this);
}

void DefferedTask::HandleSub(void *) {
  if (timer->IsTimePassed(_time)) {
    _is_registered = false;
    _func.Execute();
  } else {
    task_ctrl->Register(cpu_ctrl->GetId(), &_task);
  }
}
