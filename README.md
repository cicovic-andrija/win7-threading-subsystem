# Kernel subsystem for thread management and synchronization

An implementation of a small, but completely functional thread management
subsystem for a preemptive, time-sharing operating system kernel. The solution
implements and allows concurrent execution of multiple user-written threads on
an Intel 8086 compatible CPU. The system provides a support for simple IPC using
UNIX-like signal sending/handling, as well as semaphore and event synchronization
primitives.

## Specification

- Read the full specification of the project, in Serbian: [ETF_OS1](./ETF_OS1.pdf)

## Notes

- Subsystem is implemented in C++ (non-standard, pre-C++98), and compiled by the
  Borland C++ Compiler compiler `bcc`, version 3.1 (with the `huge` memory model).
- Some low-level functionality in written in assembly language for Intel 8086 and
  compatible processors.
- The solution is target and tested on the 32-bit Windows 7 operating system.
- Source, header, and library file names must not be longer than 8 characters.
- `lib/APPLICAT.LIB` library implements the `Scheduler` class for thread scheduling.
