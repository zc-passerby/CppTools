/*
 *timerThread.h
 *
 *	Created on: 2011-4-20
 *		Author: Joshua
 */

#ifndef TIMERTHREAD_H_
#define TIMERTHREAD_H_

#include "threadUtil.h"
#include <stdio.h>
#include <signal.h>

class TimerHandler
{
public:
	TimerHandler() {}
	virtual ~TimerHandler() {}
	virtual void HandleTimer() = 0;
};

class Timer : public Thread
{
protected:
	unsigned int m_Interval;
	bool m_Running;
	TimerHandler *m_TimerHandler;

protected:
	virtual int Run()
	{
#ifdef _DEBUG
		printf("=Timer= INFO: timer thread starts.\n");
#endif
		m_Running = true;
		while (m_Running)
		{
			for (unsigned int i = 0; i < m_Interval; i++)
			{
				sleep(1);
				if (!m_Running)
					return 0;
			}

			if (m_Running && m_TimerHandler)
			{
				m_TimerHandler->HandleTimer();
			}
		}

#ifdef _DEBUG
		printf("=Timer= INFO: timer thread exits.\n");
#endif
		return 0;
	}

public:
	Timer(unsigned int seconds = 1) : m_Interval(seconds), m_Running(false), m_TimerHandler(NULL) {}
	~Timer() {}

	virtual bool Stop()
	{
		m_Running = false;
		return true;
	}

	void SetTimerHandler(TimerHandler *pHandler)
	{
		m_TimerHandler = pHandler;
	}

	bool SetInterval(unsigned int interval)
	{
		if (0 == m_Interval)
			return false;

		m_Interval = interval;
		return true;
	}
};
#endif /* TIMERTHREAD_H_ */
