/**
 * Simple tool to read and write FAT attributes in Linux.
 *
 * Copyright 2009 Aaron Stone <aaron@serendipity.cx>
 *
 * This file is placed in the public domain.
 * It is provided with NO WARRANTY of any kind.
 **/

// Gives us O_NOATIME
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <inttypes.h>
#include <linux/msdos_fs.h>

#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  int fd;
  __u32 attrs;
  char *file;

  if (argc < 2) {
    printf("Usage: fatattr <filename>\n");
    return 1;
  }

  file = argv[1];
  printf("Opening file %s\n", file);

  fd = open(file, O_RDONLY | O_NOATIME);
  if (fd < 0) {
    printf("Error opening file.\n");
    return 2;
  }

  if (ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, &attrs) != 0) {
    printf("Error reading attributes.\n");
    return 2;
  }
  close (fd);

  if (attrs & ATTR_NONE   ) printf("No attributes\n");
  if (attrs & ATTR_RO     ) printf("Read only\n");
  if (attrs & ATTR_HIDDEN ) printf("Hidden\n");
  if (attrs & ATTR_SYS    ) printf("System\n");
  if (attrs & ATTR_VOLUME ) printf("Volume label\n");
  if (attrs & ATTR_DIR    ) printf("Directory\n");
  if (attrs & ATTR_ARCH   ) printf("Archived\n");

  return 0;
}
