#pragma once

#include "etutils/mt/mqthread.hpp"

namespace etutils {

template<typename T>
class MQThreadPool {
public:
	typedef T						value_type;
	typedef MQThread<T>				thread_type;

public:
	MQThreadPool(const std::function<size_t(const value_type&,size_t)>& disfn) :
		m_dis_fn(disfn)
	{
	}

	~MQThreadPool() {
	}

	int start(const std::function<void(thread_type*,int)>& fn,size_t mqsize,size_t number) {
		if( ! m_threads.empty() ) {
			return -1;
		}
		do {
			for(size_t i=0; i < number; ++i) {
				std::unique_ptr<thread_type> pt(new thread_type(fn,mqsize));
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

	void stop() {
		for(size_t i=0; i < m_threads.size(); ++i) {
			m_threads[i]->stop();
		}
		m_threads.clear();
	}

	bool putq(value_type const& obj) {
		size_t idx = m_dis_fn(obj,m_threads.size());
		if( idx >= 0 && idx < m_threads.size() ) {
			return m_threads[idx]->putq(obj);
		} else {
			return false;
		}
	}

private:
	std::function<size_t(const value_type&,size_t)>			m_dis_fn;
	std::vector<std::unique_ptr<thread_type>>				m_threads;
};

}

