## Porting AliOS

This document describes the required functions to port AliOS to a new
architecture.

```c
##### void _kernelLock()
```
This function is called to lock the kernel from within an interrupt context.

```c
##### void _kernelUnlock()
```
This function is called to unlock the kernel from within an interrupt context.
This function must be able to unlock a kernelLock().

```c
##### void kernelLock()
```
This function is called to lock the kernel. This function must be able to be
called from within an interrupt context.

```c
##### void kernelUnlock()
```
This function is called to unlock the kernel. This function must be able to
unlock a _kernelLock().

```c
##### void taskSetup(Task* task, void (*fx)(void*, void*), void* arg1, void* arg2)
```
This function does the architecture specific setup required for a task.  This
usually consists of settting up the initial stack.

```c
##### unsigned long taskStackUsage(Task* task)
```
This function is required if TASK_STACK_USAGE is enabled.  This function must
return the number of bytes of stack that has been used by a task.  This
function is useful in properly setting the stack sizes of tasks.

```c
##### void _taskEntry(Task* task)
```
This function is called by the kernel before a task is run for the first time.
Technically, the task is running but the kernel is just about call the task's
fx(arg) function.  Depending on the architecture, the kernel will start a
task with the kernel locked, and this callback can be used to unlock the
kernel.
