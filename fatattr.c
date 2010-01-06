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
#include <stdlib.h>
#include <stdio.h>

int getattrs(char *file, __u32 *attrs)
{
  return _ioctl_attrs(file, attrs, FAT_IOCTL_GET_ATTRIBUTES, "reading");
}

int setattrs(char *file, __u32 *attrs)
{
  return _ioctl_attrs(file, attrs, FAT_IOCTL_SET_ATTRIBUTES, "writing");
}

int _ioctl_attrs(char *file, __u32 *attrs, int ioctlnum, char *verb)
{
  int fd;

  // Interesting, we don't need a read-write handle to call the SET ioctl.
  fd = open(file, O_RDONLY | O_NOATIME);
  if (fd < 0) {
    fprintf(stderr, "Error opening '%s': %s\n", file, strerror(errno));
    goto err;
  }

  if (ioctl(fd, ioctlnum, attrs) != 0) {
    fprintf(stderr, "Error %s attributes: %s\n", verb, strerror(errno));
    goto err;
  }

  close (fd);
  return 0;

  err:
    close (fd);
    return -1;
}

void printattrs(char *file, __u32 attrs)
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

int makeaddattrs(__u32 *attrs, char arg)
{
  switch (arg) {
  case 'n': *attrs |= ATTR_NONE;   break;
  case 'r': *attrs |= ATTR_RO;     break;
  case 'h': *attrs |= ATTR_HIDDEN; break;
  case 's': *attrs |= ATTR_SYS;    break;
  case 'v': *attrs |= ATTR_VOLUME; break;
  case 'd': *attrs |= ATTR_DIR;    break;
  case 'a': *attrs |= ATTR_ARCH;   break;
  default:
    return -1;
  }
  return 0;
}

int makedelattrs(__u32 *attrs, char arg)
{
  switch (arg) {
  case 'n': *attrs |= ATTR_NONE;   break;
  case 'r': *attrs |= ATTR_RO;     break;
  case 'h': *attrs |= ATTR_HIDDEN; break;
  case 's': *attrs |= ATTR_SYS;    break;
  case 'v': *attrs |= ATTR_VOLUME; break;
  case 'd': *attrs |= ATTR_DIR;    break;
  case 'a': *attrs |= ATTR_ARCH;   break;
  default:
    return -1;
  }
  return 0;
}

void invalidarg(char *arg)
{
  fprintf(stderr, "Invalid argument: %s\n", arg);
  exit(2);
}

void exitusage(void)
{
  fprintf(stderr, "Usage: fatattr [+-nrhsvda] <filenames...>\n");
  exit(1);
}

int main(int argc, char **argv)
{
  __u32 attrs = 0, addattrs = 0, delattrs = 0;
  char *file = NULL;
  int argpos, dosetattrs = 0;

  if (argc < 2) {
    exitusage();
  }

  // Our own little getopt that does +/- options.
  for (argpos = 1; argpos < argc; argpos++) {
    if (argv[argpos][0] == '-' && argv[argpos][1] == '-') {
      // Everything after -- is a filename so you can run this on files
      // starting with + or -.
      argpos++;
      break;
    }

    if (argv[argpos][0] == '-') {
      if (makedelattrs(&delattrs, argv[argpos][1]) < 0)
        invalidarg(argv[argpos]);
      dosetattrs = 1;
    } else if (argv[argpos][0] == '+') {
      if (makeaddattrs(&addattrs, argv[argpos][1]) < 0)
        invalidarg(argv[argpos]);
      dosetattrs = 1;
    } else {
      // The first arg without a +/- in front begins the filenames.
      break;
    }
  }

  for (; argpos < argc; argpos++) {
    file = argv[argpos];
    if (getattrs(file, &attrs) == 0) {
      if (dosetattrs) {
        // Add has precedence over delete.
	attrs &= ~delattrs;
	attrs |= addattrs;
        setattrs(file, &attrs);
	// Get the attributes again to make sure it worked.
        getattrs(file, &attrs);
      }
      printattrs(file, attrs);
    }
  }

  return 0;
}
