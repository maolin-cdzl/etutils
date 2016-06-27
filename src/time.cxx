#include <time.h>
#include <sys/time.h>
#include "etutils/common/time.hpp"

namespace etutils {

uint64_t up_time() {
	struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC,&tv);
    return (uint64_t) (tv.tv_sec * 1000UL + tv.tv_nsec / 1000000UL);
}

uint64_t clock_time() {
	struct timeval tv;
	gettimeofday(&tv,nullptr);
	return (uint64_t)(tv.tv_sec * 1000UL + tv.tv_usec / 1000UL);
}

}

