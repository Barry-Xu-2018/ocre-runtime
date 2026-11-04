# NuttX Platform

Ocre runs on NuttX as a regular NuttX application, located in `nuttx/`. It is built with the NuttX make based build system and configured with `nuttx/Kconfig`. It has been tested on Sony Spresense (Spresense SDK 3.4.7, NuttX 12.3.0).

Check [Get Started with NuttX](GetStartedNuttX.md) for building and running it.

## WAMR

WAMR is built from the `wasm-micro-runtime` submodule with WAMR's own NuttX makefile (`product-mini/platforms/nuttx/wamr.mk`), through the `nuttx/wamr` symbolic link. `nuttx/Makefile` selects the same WAMR features as the Zephyr build (fast interpreter, WASI, wasi-threads, reference types, bulk memory, shared heap). `CONFIG_INTERPRETERS_WAMR` (`apps/interpreters/wamr`) must not be enabled at the same time.

## container loading

`open(2)`, `malloc(3)` and `read(2)` are used (`src/platform/posix/file_alloc_read.c`), as `mmap(2)` is not available on most NuttX file systems.

## user memory

Implemented with `memalign(3)`, `realloc(3)` and `free(3)`.

## logging

Same as the [POSIX platform](PlatformPosix.md): the `LOG_*(fmt, ...)` macros use `fprintf(3)` to `stderr`.

## threads

NuttX has a small default pthread stack size (`CONFIG_PTHREAD_STACK_DEFAULT`), so containers threads are created with `CONFIG_OCRE_CONTAINER_STACK_SIZE` (16 KiB by default).  

The Ocre timer API uses `SIGEV_THREAD`, so `CONFIG_OCRE_TIMER` selects `CONFIG_SIG_EVTHREAD`.

## commands

A container runs in a thread of the task that starts it, and NuttX releases the threads of a task when it exits. As NSH builtin commands are short lived tasks, the Ocre context is owned by a daemon:

- `ocred`: starts the daemon, which creates the Ocre context in `CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY`. `ocred stop` stops it.
- `ocre`: the [Ocre CLI](OcreCli.md). It forwards its command line to `ocred` through a message queue and returns when the command completes. The output is printed by the daemon on its console.
- `ocre_mini`: the [mini sample](samples/mini.md). Runs the embedded `hello-world.wasm` once, without the daemon.

`ocred` and `ocre_mini` first check that the working directory is on a mounted file system (`src/platform/nuttx/workdir.c`). Without it, a missing SD card fails later with `ENOENT`, as NuttX creates the directories in the pseudo file system but not the files.

`ocre image pull` uses `netutils/webclient` (HTTP only), enabled with `CONFIG_OCRE_IMAGE_PULL`.

## config.h

Includes `nuttx/config.h`, which defines the `CONFIG_OCRE_*` options from `nuttx/Kconfig`.  
`CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY` defaults to `/mnt/sd0/ocre` (SD card).

## Current limitations

- `CONFIG_OCRE_GPIO` and `CONFIG_OCRE_SENSORS` are Zephyr only.
- `ocre image pull` supports HTTP only.
- AOT (`CONFIG_OCRE_WAMR_AOT`) is not tested.
