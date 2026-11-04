# NuttX builds Ocre with the make based build system in nuttx/Makefile.
# This file is provided for NuttX CMake builds and to mirror other platforms.

add_library(OcrePlatform)

target_sources(OcrePlatform
    PRIVATE
    memory.c
    workdir.c
    ../posix/log.c
    ../posix/file_alloc_read.c
    ../posix/lstat.c
)

add_subdirectory(.. OcrePlatformBase)

target_include_directories(OcrePlatform
    PUBLIC
    include
)

target_link_libraries(OcrePlatform
    PRIVATE
    OcrePlatformBase
)
