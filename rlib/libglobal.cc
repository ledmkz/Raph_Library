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
 * Author: Liva
 * 
 */

#include <libglobal.h>
#include <thread.h>
#include <task.h>
#include <tty.h>
#include <mem/uvirtmem.h>
#include <dev/posixtimer.h>

CpuCtrlInterface *cpu_ctrl;
TaskCtrl *task_ctrl;
Timer *timer;
Tty *gtty;
Tty *gerr;

#ifndef __KERNEL__
void AllocateLibGlobalObjects() {
  virtmem_ctrl = new UVirtmemCtrl;
  timer = new PosixTimer;
  cpu_ctrl = new PthreadCtrl(8);
  task_ctrl = new TaskCtrl;
  gtty = new StdOut;
  gerr = new StdErr;
}

void InitializeLibGlobalObjects() {
  task_ctrl->Setup();
  gtty->Init();
  gerr->Init();
}

#endif // __KERNEL__

