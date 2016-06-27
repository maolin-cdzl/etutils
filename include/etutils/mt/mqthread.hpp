#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include "etutils/mt/thread.hpp"

namespace etutils {

// NOTE,all method are NOT MT-SAFE unless itself say so.
// Requirements:
//	T must have a default constructor
//	T must be copyable
//	only one thread is allowed to put data
//	
//	using lockfree queue instead of shared queue,because in echat voip thread, any blocking is unaccepted.
template<typename T>
class MQThread : public Thread {
public:
	typedef T				value_type;
	typedef MQThread<T>		thread_type;
public:
	MQThread(const std::function<void(thread_type*,int)>& fn,size_t mqsize) :
		Thread(fn),
		m_queue(mqsize)
	{
	}

	virtual ~MQThread() {
		m_queue.reset();
	}

	virtual void stop() {
		Thread::stop();
		m_queue.reset();
	}

	bool putq(value_type const& obj) {
		bool ret = m_queue.push(obj);
		if( ret ) {
			sendSignal(char(1));
		}
		return ret;
	}

	// 只有线程函数 fn 允许访问
	bool getq(value_type& v) {
		return m_queue.pop(v);
	}
protected:
	boost::lockfree::spsc_queue<value_type> m_queue;
};

}

