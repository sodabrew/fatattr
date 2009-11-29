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

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int getattr(char *file, __u32 *attrs)
{
  int fd;

  fd = open(file, O_RDONLY | O_NOATIME);
  if (fd < 0) {
    fprintf(stderr, "Error opening file %s\n", strerror(errno));
    goto err;
  }

  if (ioctl(fd, FAT_IOCTL_GET_ATTRIBUTES, attrs) != 0) {
    fprintf(stderr, "Error reading attributes: %s\n", strerror(errno));
    goto err;
  }

  close (fd);
  return 0;

  err:
    close (fd);
    return -1;
}

void printattr(char *file, __u32 attrs)
{
  char out[8];
  out[0] = attrs & ATTR_NONE   ? 'n' : '-';
  out[1] = attrs & ATTR_RO     ? 'r' : '-';
  out[2] = attrs & ATTR_HIDDEN ? 'h' : '-';
  out[3] = attrs & ATTR_SYS    ? 's' : '-';
  out[4] = attrs & ATTR_VOLUME ? 'v' : '-';
  out[5] = attrs & ATTR_DIR    ? 'd' : '-';
  out[6] = attrs & ATTR_ARCH   ? 'a' : '-';
  out[7] = '\0';
  printf("%s %s\n", out, file);
}

int addattr(__u32 *attrs, char arg)
{
  switch (arg) {
  case 'n':
    *attrs |= ATTR_NONE;
    break;
  case 'r':
    *attrs |= ATTR_RO;
    break;
  case 'h':
    *attrs |= ATTR_HIDDEN;
    break;
  case 's':
    *attrs |= ATTR_SYS;
    break;
  case 'v':
    *attrs |= ATTR_VOLUME;
    break;
  case 'd':
    *attrs |= ATTR_DIR;
    break;
  case 'a':
    *attrs |= ATTR_ARCH;
    break;
  default:
    return -1;
  }
  return 0;
}

int delattr(__u32 *attrs, char arg)
{
  switch (arg) {
  case 'n':
    *attrs &= ~ATTR_NONE;
    break;
  case 'r':
    *attrs &= ~ATTR_RO;
    break;
  case 'h':
    *attrs &= ~ATTR_HIDDEN;
    break;
  case 's':
    *attrs &= ~ATTR_SYS;
    break;
  case 'v':
    *attrs &= ~ATTR_VOLUME;
    break;
  case 'd':
    *attrs &= ~ATTR_DIR;
    break;
  case 'a':
    *attrs &= ~ATTR_ARCH;
    break;
  default:
    return -1;
  }
  return 0;
}

int main(int argc, char **argv)
{
  __u32 attrs = 0, setattrs = 0;
  char *file = NULL;
  int argpos;

  if (argc < 2) {
    printf("Usage: fatattr [+-nrhsvda] <filenames...>\n");
    return 1;
  }

  // Our own little getopt that does +/- options.
  for (argpos = 1; argpos < argc; argpos++) {
    if (argv[argpos][0] == '-') {
      delattr(&setattrs, argv[argpos][1]);
    } else if (argv[argpos][0] == '+') {
      addattr(&setattrs, argv[argpos][1]);
    } else {
      break;
    }
  }

  // The first arg without a +/- in front begins the filenames.
  for (; argpos < argc; argpos++) {
    file = argv[argpos];
    if (getattr(file, &attrs) < 0) {
      // Error
      continue;
    } else {
      printattr(file, attrs);
    }
  }

  return 0;
}
