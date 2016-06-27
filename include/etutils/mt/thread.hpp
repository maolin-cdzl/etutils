#pragma once

#include <memory>
#include <thread>

namespace etutils {

// NOTE,all method are NOT MT-SAFE unless itself say so.

//主要实现思路参考 czmq 中的 zactor_t. 线程函数 fn　必须在完成初始化后通过参数传递的pipe发送一个byte 0通知调用者初始化完成．非零值意味着初始化失败，线程函数即将中止自身．
//同样，当需要中止线程时，Thread 会通过管道发送一个byte 0，线程函数 fn 必须检测管道符并根据要求中止自身运行．
class Thread {
public:
	Thread(const std::function<void(int)>& fn);
	virtual ~Thread();

	Thread(const Thread&) = delete;
	Thread& operator = (const Thread&) = delete;
	Thread& operator = (Thread&&) = delete;

	// 要求已经通过构造函数提供线程函数
	// start 会阻塞直到线程函数通过管道发送初始化完成消息．
	// 如果完成消息不为０，启动失败 start 返回线程函数返回的错误码
	// 当 start 返回时，线程函数已经初始化完成处于运行中，或者已失败，线程已经销毁.
	virtual int start();

	virtual void stop();

	bool active() const;
protected:
	int sendSignal(char s);
protected:
	std::function<void(int)>		m_fn;
	std::unique_ptr<std::thread>	m_thread;
	int								m_pipe[2];
};

}

