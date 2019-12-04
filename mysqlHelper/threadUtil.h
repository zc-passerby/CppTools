#ifndef THREADUTIL_H_
#define THREADUTIL_H_

#include <queue>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

using namespace std;

class autoRwLock
{
private:
	pthread_rwlock_t *mRwlock;

public:
	autoRwLock(pthread_rwlock_t &lock, bool read = true)
	{
		mRwlock = &lock;
		if (read)
		{
			pthread_rwlock_rdlock(mRwlock);
		}
		else
		{
			pthread_rwlock_wrlock(mRwlock);
		}
	}
	~autoRwLock()
	{
		pthread_rwlock_unlock(mRwlock);
	}
};

class autoLock
{
private:
	pthread_mutex_t *mlock;

public:
	autoLock(pthread_mutex_t &lock)
	{
		mlock = &lock;
		pthread_mutex_lock(mlock);
	}
	~autoLock()
	{
		pthread_mutex_unlock(mlock);
	}
};
class atomic_Int
{
private:
	pthread_mutex_t m_lock;
	int m_num;

public:
	atomic_Int() : m_num(0)
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	atomic_Int(int num) : m_num(num)
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~atomic_Int()
	{
		pthread_mutex_destroy(&m_lock);
	}
	int atomic_set(int num)
	{
		autoLock al(m_lock);
		m_num = num;
		return m_num;
	}
	int atomic_inc()
	{
		autoLock al(m_lock);
		return ++m_num;
	}
	int atomic_dec()
	{
		autoLock al(m_lock);
		return --m_num;
	}
};

template <class T>
class SafeQueue
{
private:
	pthread_mutex_t m_lock;
	queue<T> m_queue;

public:
	SafeQueue()
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~SafeQueue()
	{
		pthread_mutex_destroy(&m_lock);
	}
	bool PutMsg(T msg)
	{
		autoLock al(m_lock);
		m_queue.push(msg);
		return true;
	}
	bool GetMsg(T &msg)
	{
		autoLock al(m_lock);
		if (m_queue.empty())
		{
			memset(&msg, 0, sizeof(msg));
			return false;
		}

		msg = m_queue.front();
		m_queue.pop();
		return true;
	}
};
/*
class Thread {
protected:
	static void * start_routine(void* arg) {
		return (void *) ((Thread *) arg)->Run();
	}
	pthread_t m_id;
	//to be overrided by subclass
	virtual int Run() = 0;
public:
	Thread() {}
	virtual ~Thread() {}

	virtual bool Stop() = 0;

	virtual bool Start(){
		int err = pthread_create(&m_id, NULL, Thread::start_routine, this);
		if (0 != err) {
			printf("Thread::Start Error: failed to create thread.\n");
			return false;
		}
		return true;
	}

	virtual bool Join() {
		int err = pthread_join(m_id, NULL);
		if (0 != err) {
			printf("Thread::Join Error: failed to join thread[%u].\n", (unsigned int)m_id);
			return false;
		}
		return true;
	}
};
*/
class ThreadPool
{
protected:
	static void *start_routine(void *arg)
	{
		return (void *)(long)((ThreadPool *)arg)->Run();
	}
	pthread_t *m_id;
	int m_num;
	//to be overrided by subclass
	virtual int Run() = 0;

public:
	virtual bool Stop() = 0;

	ThreadPool() : m_id(NULL), m_num(0) {}
	virtual ~ThreadPool()
	{
		delete[] m_id;
	}

	virtual bool Start(int num = 3)
	{
		if (num < 1)
		{
			//log err
			return false;
		}

		m_num = num;
		m_id = new pthread_t[m_num];

		for (int i = 0; i < num; i++)
		{
			int err = pthread_create(&m_id[i], NULL, ThreadPool::start_routine, this);
			if (0 != err)
			{
				printf("ThreadPool::Start Error: failed to create thread.\n");
				return false;
			}
		}
		return true;
	}

	virtual bool Join()
	{
		for (int i = 0; i < m_num; i++)
		{
			int err = pthread_join(m_id[i], NULL);
			if (0 != err)
			{
				printf("Thread::Join Error: failed to join thread[%u].\n", (unsigned int)m_id[i]);
				return false;
			}
		}
		return true;
	}
};

class Thread_Message
{
public:
	bool isQuitMsg;
	Thread_Message() : isQuitMsg(false) {}
	virtual ~Thread_Message() {}
};

class MessageHandler : public ThreadPool
{
private:
	bool m_initialized;
	SafeQueue<Thread_Message *> m_queue;
	sem_t m_sem;
	atomic_Int running_num;

public:
	MessageHandler() : m_initialized(false), running_num(0) {}
	~MessageHandler()
	{
		sem_destroy(&m_sem);
	}

	bool PutMsg(Thread_Message *msg)
	{
		if (!m_initialized)
			return false;

		//put message into the queue, and signal semaphore
		m_queue.PutMsg(msg);
		int err = sem_post(&m_sem);
		if (0 != err)
		{
			printf("MessageHandler::PutMsg Error: failed to do sem_post.\n");
			return false;
		}
		return true;
	}

	bool Init()
	{
		if (m_initialized)
			return false;

		int err = sem_init(&m_sem, 0, 0);
		if (0 != err)
		{
			printf("MessageHandler::Init Error: failed to do sem_init.\n");
			return false;
		}

		m_initialized = true;
		return true;
	}

	virtual bool Start(int num = 3)
	{
		if (!ThreadPool::Start(num))
			return false;
		running_num.atomic_set(m_num);
		return true;
	}

	bool Stop()
	{
		if (!m_initialized)
			return false;

		for (int i = 0; i < m_num; i++)
		{
			Thread_Message *msg = new Thread_Message;
			msg->isQuitMsg = true;
			PutMsg(msg);
		}
		m_initialized = false;
		return true;
	}

	bool isInitialized()
	{
		return m_initialized;
	}

protected:
	virtual int Run()
	{
		if (!m_initialized)
			return -1;

		while (1)
		{
			int err = sem_wait(&m_sem);
			if (0 != err)
			{
				if (EINTR == errno)
					continue;

				printf("MessageHandler::Run Error: failed to do sem_wait.\n");
				return -1;
			}

			Thread_Message *msg = NULL;
			if (m_queue.GetMsg(msg))
			{
				if (msg->isQuitMsg)
				{
					PreQuit();
					return 0;
				}
				else
				{
					Handle(msg);
				}
				delete msg;
			}
		}

		return 0;
	}

	virtual void PreQuit()
	{
		if (0 == running_num.atomic_dec())
		{
			//delete all messages in the queue
			Thread_Message *msg = NULL;
			while (m_queue.GetMsg(msg))
			{
				delete msg;
			}
		}
	}

	//to be overrided by subclass
	virtual void Handle(Thread_Message *msg) = 0;
};

#endif /* THREADUTIL_H_ */
