#ifndef MONITOR_UTILS_H_
#define MONITOR_UTILS_H_
#include "structs.h"
#include "typedef.h"

class Utils
{
public:
    Utils(){};
    ~Utils(){};
    static const char *tcp_flag_to_str(unsigned char flag);
    static int64 ToMword(unsigned long x);
    static unsigned int UINT24to32(unsigned short low,unsigned char high);
	static const char * DCTypeToString(int dc_type);
	static const char * DCGeneral_IntypeToString(int dc_general_intype);
	static void WriteIntoFile(const char *file_name, const char *mode, const void* data , size_t length);
	//pthread_t tid;
	//tid =pthread_self();
	//Utils::Print_Thread_ID(tid);
	static void Print_Thread_ID(pthread_t tid);
	static void SleepUsec(const int usec);
	static unsigned long GetCurrentTime();
};

#endif // MONITOR_UTILS_H_
