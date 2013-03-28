/*
 * dummy header fcntl.h
 *
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 */

#include <stdio.h>

#ifndef _DUMMY_FCNTL_H_
#define _DUMMY_FCNTL_H_

#ifndef _MSC_VER

#ifdef __cplusplus
extern "C" {
#endif

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_APPEND    0x0008
#define O_CREAT     0x0200
#define O_TRUNC     0x0400
#define O_NONBLOCK  0x4000

#define F_GETFL  3
#define F_SETFL  4

#ifdef __cplusplus
}
#endif

#endif /* _MSC_VER */

#endif /* _DUMMY_FCNTL_H_ */
