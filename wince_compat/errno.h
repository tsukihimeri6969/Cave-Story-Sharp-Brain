#ifndef WINCE_ERRNO_H
#define WINCE_ERRNO_H

static int wince_dummy_errno = 0;
#define errno wince_dummy_errno
#define ENOENT 2
#define EIO 5
#define EACCES 13
#define EEXIST 17
#define EINVAL 22

#endif
