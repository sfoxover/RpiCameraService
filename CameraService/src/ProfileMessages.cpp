#include "ProfileMessages.h"
#include "helpers.h"
#include "Settings.h"
#include <assert.h>
#include <Message.h>
#include <MessageFactory.h>
#include <PublishMessage.h>
#include <DetectFaces.h>
using namespace std::chrono_literals;

CProfileMessages::CProfileMessages()
{
	SetExitingFlag(false);
}

CProfileMessages::~CProfileMessages()
{
	std::wstring error;
	bool bOK = Stop(error);
	assert(bOK);
}

// Get set for _exitingFlag
void CProfileMessages::GetExitingFlag(bool& value)
{
	_exitingFlagLock.lock();
	value = _exitingFlag;
	_exitingFlagLock.unlock();
}

void CProfileMessages::SetExitingFlag(bool value)
{
	_exitingFlagLock.lock();
	_exitingFlag = value;
	_exitingFlagLock.unlock();
}

// Start profiling data thread
bool CProfileMessages::Start(std::wstring& error)
{
	_profileThread = std::thread(&CProfileMessages::ProfilingThread, this);
	return true;
}

// Stop profiling data thread
bool CProfileMessages::Stop(std::wstring& error)
{
	SetExitingFlag(true);
	_stopEvent.notify_all();

	if (_profileThread.joinable())
		_profileThread.join();
	return true;
}

// Profiling data thread
void CProfileMessages::ProfilingThread(CProfileMessages* pThis)
{
	bool exiting = false;
	do
	{
		pThis->GetExitingFlag(exiting);
		if (!exiting)
		{
			std::map<std::string, std::any> items;

			// Get AI detection images per second
			int imagesPerSecond = 0;
			CDetectFaces::Instance().GetImagesPerSecond(imagesPerSecond);
			items["AiImagesPerSec"] = imagesPerSecond;

			// Get CPU usage for current process
			double cpuUsage = Helpers::GetProcessCpuUsage();
			items["CpuUsage"] = cpuUsage;

			CMessage msg = MessageFactory::Create(CSettings::Instance().GetProfilingTopic(), CMessage::MessageType::ProfilingData, items);
			std::wstring error;
			bool bOK = CPublishMessage::Instance().SendMessageData(msg, error);
			assert(bOK);

			// Sample every 1 second
			std::unique_lock<std::mutex> lock(pThis->_stopEventLock);
			pThis->_stopEvent.wait_for(lock, 1s);
		}
	} 
	while (!exiting);
}
