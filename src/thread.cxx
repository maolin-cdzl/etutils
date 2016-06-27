#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <g3log/g3log.hpp>
#include "etutils/mt/thread.hpp"

namespace etutils {

Thread::Thread(const std::function<void(int)>& fn) :
	m_fn(fn)
{
	m_pipe[0] = m_pipe[1] = -1;
}

Thread::~Thread() {
	stop();
}

int Thread::start() {
	CHECK( m_fn != nullptr );
	if( 0 != socketpair(AF_UNIX,SOCK_STREAM,0,m_pipe) ) {
		return -1;
	}
	m_thread.reset(new std::thread(m_fn,m_pipe[1]));
	char sig = 0;
	if( 1 == read(m_pipe[0],&sig,1) ) {
		if( sig == 0 ) {
			return 0;
		}
	}
	// error
	if( m_thread->joinable() ) {
		m_thread->join();
	}
	close(m_pipe[0]);
	close(m_pipe[1]);
	m_pipe[0] = m_pipe[1] = -1;
	m_thread.reset();
	return sig;
}

void Thread::stop() {
	if( m_thread == nullptr ) {
		return;
	}
	char sig = 0;
	write(m_pipe[0],&sig,1);
	if( m_thread->joinable() ) {
		m_thread->join();
	}
	m_thread.reset();
	close(m_pipe[0]);
	close(m_pipe[1]);
	m_pipe[0] = m_pipe[1] = -1;
}

bool Thread::active() const {
	return m_thread != nullptr;
}

int Thread::sendSignal(char s) {
	if( m_pipe[0] != -1 ) {
		return write(m_pipe[0],&s,1);
	} else {
		return -1;
	}
}

}

