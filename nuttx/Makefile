include $(APPDIR)/Make.defs

OCRE_SRC = ../src

# WAMR, built with its NuttX makefile. wamr.mk expects the sources at ./wamr
# and is configured with CONFIG_INTERPRETERS_WAMR_* variables, which are set
# here to match the Zephyr build (zephyr/wamr.cmake).

CONFIG_INTERPRETERS_WAMR_FAST = $(CONFIG_OCRE_WAMR_INTERPRETER)
CONFIG_INTERPRETERS_WAMR_AOT = $(CONFIG_OCRE_WAMR_AOT)
CONFIG_INTERPRETERS_WAMR_LIBC_WASI = y
CONFIG_INTERPRETERS_WAMR_THREAD_MGR = y
CONFIG_INTERPRETERS_WAMR_LIB_WASI_THREADS = y
CONFIG_INTERPRETERS_WAMR_SHARED_MEMORY = y
CONFIG_INTERPRETERS_WAMR_BULK_MEMORY = y
CONFIG_INTERPRETERS_WAMR_REF_TYPES = y
CONFIG_INTERPRETERS_WAMR_LOG = y

include wamr/product-mini/platforms/nuttx/wamr.mk

CFLAGS += -DWASM_ENABLE_BULK_MEMORY_OPT=1
CFLAGS += -DWASM_ENABLE_CALL_INDIRECT_OVERLONG=1

# The runtime always uses the shared heap API. The heap itself is only created
# with CONFIG_OCRE_SHARED_HEAP.

CFLAGS += -DWASM_ENABLE_SHARED_HEAP=1
CSRCS += shared_heap_wrapper.c
VPATH += $(IWASM_ROOT)/libraries/shared-heap

# Ocre

CFLAGS += ${INCDIR_PREFIX}generated
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/platform/nuttx/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/platform/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/common/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/ocre/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/runtime/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/runtime/wamr-wasip1/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/uthash/include
CFLAGS += ${INCDIR_PREFIX}$(OCRE_SRC)/shell/include

# Sources are found through VPATH, so objects are created in this directory.
# Sources sharing a basename are compiled through the wrappers in shell/.
# The mini sample is compiled through samples/ocre_mini_main.c.

VPATH += $(OCRE_SRC)/common
VPATH += $(OCRE_SRC)/ocre
VPATH += $(OCRE_SRC)/ocre/util
VPATH += $(OCRE_SRC)/runtime/wamr-wasip1
VPATH += $(OCRE_SRC)/runtime/wamr-wasip1/ocre_api
VPATH += $(OCRE_SRC)/runtime/wamr-wasip1/ocre_api/core
VPATH += $(OCRE_SRC)/runtime/wamr-wasip1/ocre_api/ocre_timers
VPATH += $(OCRE_SRC)/runtime/wamr-wasip1/ocre_api/ocre_messaging
VPATH += $(OCRE_SRC)/runtime/wamr-wasip1/ocre_api/utils
VPATH += $(OCRE_SRC)/platform/nuttx
VPATH += $(OCRE_SRC)/platform/posix

# core
CSRCS += common.c
CSRCS += container.c context.c ocre.c
CSRCS += rm_rf.c string_array.c unique_random_id.c

# runtime
CSRCS += wamr.c
CSRCS += ocre_api.c ocre_common.c ocre_timer.c ocre_messaging.c strlcat.c
CSRCS += core_eventq.c core_misc.c core_timer.c core_mutex.c core_memory.c

# platform
CSRCS += memory.c log.c file_alloc_read.c lstat.c workdir.c

# shell, daemon and client

ifeq ($(CONFIG_OCRE_SHELL),y)
VPATH += shell
VPATH += $(OCRE_SRC)/shell
VPATH += $(OCRE_SRC)/shell/image
VPATH += $(OCRE_SRC)/shell/container
VPATH += $(OCRE_SRC)/shell/sha256
VPATH += $(OCRE_SRC)/samples/supervisor/nuttx

CSRCS += shell.c image.c shell_container.c
CSRCS += ls.c pull.c sha256_file.c shell_image_rm.c
CSRCS += create.c kill.c pause.c ps.c shell_container_rm.c start.c stop.c unpause.c wait.c
CSRCS += sha256.c
CSRCS += download_file.c

PROGNAME  += ocred ocre
PRIORITY  += SCHED_PRIORITY_DEFAULT SCHED_PRIORITY_DEFAULT
STACKSIZE += $(CONFIG_OCRE_DAEMON_STACKSIZE) $(CONFIG_OCRE_CLIENT_STACKSIZE)
MAINSRC   += ocred_main.c ocre_client_main.c
endif

# mini sample

ifeq ($(CONFIG_OCRE_MINI),y)
VPATH += samples

OCRE_INPUT_FILE ?= $(OCRE_SRC)/samples/mini/hello-world.wasm

CSRCS += input_file.c

PROGNAME  += ocre_mini
PRIORITY  += SCHED_PRIORITY_DEFAULT
STACKSIZE += $(CONFIG_OCRE_MINI_STACKSIZE)
MAINSRC   += ocre_mini_main.c

input_file.c: $(OCRE_INPUT_FILE) ../scripts/c_array.awk
	$(Q) od -v -An -tx1 $< | awk -f ../scripts/c_array.awk > $@
endif

MODULE = $(CONFIG_OCRE)

# Generated build information (src/common/CMakeLists.txt)

generated/build_info.h:
	$(Q) mkdir -p generated
	$(Q) echo "#define OCRE_BUILD_HOST_INFO \"$(USER) @ $$(uname -a)\"" > $@
	$(Q) echo "#define OCRE_BUILD_DATE \"$$(date +'%Y-%m-%d %H:%M:%S %Z')\"" >> $@

generated/commit_id.h:
	$(Q) mkdir -p generated
	$(Q) echo "/* Auto-generated file. DO NOT EDIT */" > $@
	$(Q) echo "#define GIT_COMMIT_ID \"$$(git -C .. describe --always --abbrev=0 --dirty 2>/dev/null || echo unknown)\"" >> $@

context:: generated/build_info.h generated/commit_id.h $(filter input_file.c,$(CSRCS))

distclean::
	$(call DELDIR, generated)
	$(call DELFILE, input_file.c)

include $(APPDIR)/Application.mk
