# Kernel thread management subsystem

## Operating Systems 1

### School of Electrical Engineering, University of Belgrade, 2017.

An implementation of a small, but completely functional thread management
subsystem for a preemptive, time-sharing operating system kernel.
The solution implements and allows concurrent execution of multiple
user-written threads on an Intel 8086 compatible CPU.
The system provides a support for simple IPC using UNIX-like
signal sending/handling, and semaphore and event synchronization primitives.

Full project description in Serbian: `OS1_UpravljanjeNitima.pdf`

### Notes:
* System is implemented in C++ (90s version, before C++98 standard), supported
  by `bcc 3.1` compiler. Some parts were written in assembly for Intel 8086
  and compatible processors.
* Targeted (host) operating system: Windows 7 (32-bit)
* Required compiler: BCC (Borland C++ Compiler), version 3.1
* Memory model: huge
* Source/header/library file names should not be longer than 8 characters
* `APPLICAT.LIB` lib: Implementation of Scheduler class
