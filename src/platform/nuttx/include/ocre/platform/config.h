#ifndef OCRE_PLATFORM_NUTTX_H
#define OCRE_PLATFORM_NUTTX_H

/* The CONFIG_OCRE_* options come from the NuttX Kconfig (nuttx/Kconfig) */

#include <nuttx/config.h>

#ifndef CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY
#define CONFIG_OCRE_DEFAULT_WORKING_DIRECTORY "/mnt/sd0/ocre"
#endif

/* Kconfig leaves disabled options undefined, but some are tested with #if */

#ifndef CONFIG_OCRE_NETWORKING
#define CONFIG_OCRE_NETWORKING 0
#endif

#endif /* OCRE_PLATFORM_NUTTX_H */
