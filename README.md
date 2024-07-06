# SFL2 - Service Framework Library 2.0 for Windows

## Intro

This header-only library is designed for creating your own native Windows service applications in C++. This part has always been missing from MS materials for C++ programmers, and it looks like it will never appear in the foreseeable future of the world of Windows programming as we all knew it.

## Demo & Design stuff

Folder [examples](examples/) includes minimalistic demo application:

- [MinSvc](examples/MinSvc/) - a dummy Windows service application that does nothing but service stuff. Being properly registered it is able to start, pause, continue and stop as any other Windows service. In fact, the service source code may be used as a basic template for your service app.

- [SFL-Architecure](examples/SFL-Architecture.md) document gives you some design details about the library internals and the logic it implements.

## History

The library's version 2.0 is to hint you that it stems from my original work - the SFL v1.0 originally published on RSDN (Russian Software Developers Network). The main concepts were kept, but the internals were reworked noticeably.
