#ifndef OCRE_PLATFORM_NUTTX_WORKDIR_H
#define OCRE_PLATFORM_NUTTX_WORKDIR_H

/**
 * Check that a working directory is on a mounted file system.
 *
 * On NuttX, directories can be created in the pseudo file system (i.e. under
 * '/mnt/sd0' when the SD card is not mounted), but files cannot, so Ocre would
 * fail later with ENOENT.
 *
 * @param path The working directory. It does not need to exist.
 * @return 0 if it is on a mounted file system, -1 otherwise (an error is printed)
 */
int ocre_check_working_directory(const char *path);

#endif /* OCRE_PLATFORM_NUTTX_WORKDIR_H */
