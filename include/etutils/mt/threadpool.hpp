#pragma once

#include <vector>
#include "etutils/mt/thread.hpp"

namespace etutils {

class ThreadPool {
public:
	ThreadPool();
	~ThreadPool();

	int start(const std::function<void(int)>& fn,size_t number);
	void stop();

protected:
	std::vector<std::unique_ptr<Thread>>		m_threads;
};

}

