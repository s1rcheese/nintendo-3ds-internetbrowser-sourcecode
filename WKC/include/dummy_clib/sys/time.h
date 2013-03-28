/*
 * dummy header sys/time.h
 *
 * Copyright (c) 2010 ACCESS CO., LTD. All rights reserved.
 */

#ifndef _DUMMY_SYS_TIME_H_
#define _DUMMY_SYS_TIME_H_

#if defined(__ARMCC_VERSION)
#ifndef WKC_DECLARED_TIMEVAL
struct timeval {
    long tv_sec;
    long tv_usec;
};
#define WKC_DECLARED_TIMEVAL
#endif
#endif //__ARMCC_VERSION


#endif /* _DUMMY_SYS_TIME_H_ */
