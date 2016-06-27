#include <time.h>
#include <sys/time.h>
#include "etutils/common/time.hpp"

namespace etutils {

int64_t up_time() {
	struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC,&tv);
    return (int64_t) (tv.tv_sec * 1000L + tv.tv_nsec / 1000000L);
}

int64_t clock_time() {
	struct timeval tv;
	gettimeofday(&tv,nullptr);
	return (int64_t)(tv.tv_sec * 1000L + tv.tv_usec / 1000L);
}

}

