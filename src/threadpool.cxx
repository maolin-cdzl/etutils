#include "etutils/mt/threadpool.hpp"

namespace etutils {

ThreadPool::ThreadPool()
{
}

ThreadPool::~ThreadPool() {
	stop();
}

int ThreadPool::start(const std::function<void(int)>& fn,size_t number) {
	if( ! m_threads.empty() ) {
		return -1;
	}
	do {
		for(size_t i=0; i < number; ++i) {
			std::unique_ptr<Thread> pt(new Thread(fn));
			if( -1 == pt->start() ) {
				break;
			}
			m_threads.push_back(std::move(pt));
		}
		return 0;
	} while(0);

	for(size_t i=0; i < m_threads.size(); ++i) {
		m_threads[i]->stop();
	}
	m_threads.clear();
	return -1;
}

void ThreadPool::stop() {
	for(size_t i=0; i < m_threads.size(); ++i) {
		m_threads[i]->stop();
	}
	m_threads.clear();
}


}

