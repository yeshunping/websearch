/**
 * @file utils/u.h
 * @brief 通用工具.
 *  
 **/

#ifndef  __UTILS_U_H_
#define  __UTILS_U_H_
#include <sys/time.h>

#define TIMER_BEGIN	\
	static int _cnt = 0;\
	static long _delt = 0;\
	struct timeval t1, t2;\
	gettimeofday(&t1, NULL);

#define TIMER_END(where)	\
	gettimeofday(&t2, NULL);\
	_cnt++;\
	_delt += (t2.tv_usec - t1.tv_usec) + 1000000 * (t2.tv_sec - t1.tv_sec);\
	fprintf(stderr, "%s time : %ld/%d=%ld(usecs)\n", where, _delt, _cnt, _delt/_cnt);

#endif  //__UTILS/U_H_
