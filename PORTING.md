## Porting AliOS

This document describes the required functions to port AliOS to a new
architecture.

---
```c
void _taskInit(Task* task, void* stackBase, unsigned long stackSize)
```
This function is called by the kernel to perform low-level initialization of
the first task.

---
```c
void taskSetup(Task* task, void (*fx)())
```
This function does the architecture specific setup required for a task.  This
usually consists of setting up the initial stack.  The fx argument is a
pointer to a task function.

---
```c
void _taskSwitch(Task* current, Task* next)
```
This function is called by the kernel when it needs to switch task context.
The arguments current and next will never be NULL.

---
```c
void _taskEntry(Task* task)
```
This function is called by the kernel before a task is run for the first time.
Technically, the task is running but the kernel is just about call the task's
fx(arg) function.  Depending on the architecture, the kernel may start a task
with the kernel locked, and this callback can be used to unlock the kernel
before the task function is run.

---
```c
void _taskExit(Task* task)
```
This function is called by the kernel after a task has finished execution.
The task's resources (stack, etc.) are no longer in use.  This function can
be useful if your port has specific cleanup actions after a task has exited.

---
```c
#define _taskYield
```
This macro defines a function that is called whenever a task yields the
processor.  This is useful in implementing "fair timeslicing", where a task
always receives a "full" timeslice.  This would only be required when
preemption is used, which is rarely necessary in embedded.

---
```c
void taskIdle()
```
This function is called by the kernel when it has nothing to do.  It is called
within the context of the sleeping/blocked task and is called with the kernel
unlocked.  This function is useful for implementing a "wait for interrupt"
instruction or any other power savings features.

---
```c
unsigned long taskStackUsage(Task* task)
```
This function is required if TASK_STACK_USAGE is enabled.  This function must
return the number of bytes of stack that has been used by a task.  This
function is useful in properly configuring the stack sizes of tasks.

---
```c
void taskScheduleTick(bool adj, unsigned long ticks)
```
This function is called by the kernel to schedule a system timer/tick event.
The "ticks" argument is the maximum number of system ticks that can elapse.
It is completely acceptable to schedule a timer/tick event earlier than
requested by the kernel (for example, if the underlying hardware timer used
cannot be set for that long of a duration).  A "ticks" value of 0 denotes the
kernel requesting to disable system timer/tick events.  This function is
useful in implementing dynamic system ticks in order to obtain maximum power
savings.  If the adj parameter equals true the kernel is "adjusting" the
current tick, and the function should return the number of ticks that has
elapsed since the last call to this function.  Note: this function is always
called with the kernel locked.

---
```c
void kernelLocked()
```
This function is called to determine if the kernel is locked.

---
```c
void kernelLock()
```
This function is called to lock the kernel. This function is never called
recursively, but may be called from an interrupt context.

---
```c
void kernelUnlock()
```
This function is called to unlock the kernel.  This function may be called
from an interrupt context.

---
```c
void _smpLock()
```
This function is called to lock the kernel within an interrupt context.  This
function is never called recursively.

---
```c
void _smpUnlock()
```
This function is called to unlock the kernel within an interrupt context.

---
```c
#define cpuWake(cpu)
```
This macro is called by the kernel when SMP is defined.  It is used to wake
processors that may be in a low-power sleep state (possibly induced by
taskIdle).  The "cpu" argument is the cpu ID of the processor to wake.

---
```c
#define cpuID
```
This macro is called by the kernel when SMP is defined.  It is used to get
the ID of the processor.  The boot process must have ID 0, and all other
processor's IDs must increase contiguously.

---
```c
#define kmalloc
```
If this macro is defined, it defines the function that the kernel should use
to allocate dynamic memory.

---
```c
#define kfree
```
If this macro is defined, it defines the function that the kernel should use
to free dynamically allocated memory.
