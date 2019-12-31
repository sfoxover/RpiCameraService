#pragma once
/*
	CVideoSource - send server metrics such as CPU usage to message subscribers.
*/
#include <thread>
#include <condition_variable>

class CProfileMessages
{
private:
	CProfileMessages();
	~CProfileMessages();

private:
// Properties

	// Send sample profiling data thread
	std::thread _profileThread;

	// Wait event will signal if exiting
	std::condition_variable _stopEvent;
	std::mutex _stopEventLock;

	// Exit thread flag
	bool _exitingFlag;
	std::mutex _exitingFlagLock;

public:
	// Methods
	static CProfileMessages& Instance()
	{
		static CProfileMessages instance;
		return instance;
	}

	// Get set for _exitingFlag
	void GetExitingFlag(bool& value);
	void SetExitingFlag(bool value);

	// Start streaming video from hardware camera
	bool Start(std::wstring& error);

	// Stop video streaming
	bool Stop(std::wstring& error);

	// Profiling data thread
	static void ProfilingThread(CProfileMessages* pThis);
};