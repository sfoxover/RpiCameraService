#include "VideoSource.h"
#include "defines.h"
#include "helpers.h"
#include <cassert>
#include "PublishMessage.h"
#include "DetectFaces.h"
#include <inttypes.h>
#include <MessageHelper.h>
#include "Settings.h"

CVideoSource::CVideoSource()
{
}

CVideoSource::~CVideoSource()
{
	std::wstring error;
	bool bOK = Stop(error);
	assert(bOK);
}

// Start streaming video from file
bool CVideoSource::Start(std::wstring filePath, std::wstring &error)
{
	_opencvCaputure.release();
	_opencvCaputure.open(Helpers::WideToUtf8(filePath));
	if (!_opencvCaputure.isOpened())
	{
		error = L"Error opening video file.";
		return false;
	}

	_stopThreadSignal = std::make_unique<std::promise<void>>();
	auto futureObj = _stopThreadSignal->get_future();
	_videoThread = std::thread(&CVideoSource::VideoStreamingThread, this, std::move(futureObj));
	return true;
}

// Start streaming video from hardware camera
bool CVideoSource::Start(std::wstring &error)
{
	_opencvCaputure.release();
	_opencvCaputure.open(0);
	if (!_opencvCaputure.isOpened())
	{
		error = L"Error opening video camera.";
		return false;
	}

	_stopThreadSignal = std::make_unique<std::promise<void>>();
	auto futureObj = _stopThreadSignal->get_future();
	_videoThread = std::thread(&CVideoSource::VideoStreamingThread, this, std::move(futureObj));
	return true;
}

// Stop video streaming
bool CVideoSource::Stop(std::wstring &error)
{
	if (_stopThreadSignal)
	{
		_stopThreadSignal->set_value();
		_stopThreadSignal.reset(nullptr);
	}

	if (_videoThread.joinable())
		_videoThread.join();
	return true;
}

// Video streaming thread
void CVideoSource::VideoStreamingThread(CVideoSource *pThis, std::future<void> futureObj)
{
	double fps = pThis->_opencvCaputure.get(cv::CAP_PROP_FPS);
	if (fps == 0)
		fps = 30;

	uint64_t frameCount = 0;
	bool bOK = true;
	std::wstring error;
	long delay = (long)(1000 / fps);
	std::future_status waitResult;
	do
	{
		cv::Mat image;
		pThis->_opencvCaputure.read(image);
		if (!image.empty())
		{
			frameCount++;
			
			// Do face detection
			if (CSettings::Instance().GetUseFaceDetect())
			{
				if (SKIP_FRAME_NUM == 0 || (frameCount % SKIP_FRAME_NUM) == 0)
				{
					CDetectFaces::Instance().DetectFaces(image, true);
				}
			}
			
			// Load message with video frame
			CMessage msg;
			if (CSettings::Instance().GetUseSampleVideo())
			{
				msg.CreateMessageFromMatFrame(CSettings::Instance().GetVideoSampleTopic(), image, fps);
			}
			else
			{
				msg.CreateMessageFromMatFrame(CSettings::Instance().GetVideoCamTopic(), image, fps);
			}
			bOK = CPublishMessage::Instance().SendMessageData(msg, error);
			assert(bOK);
			
			waitResult = futureObj.wait_for(std::chrono::milliseconds(10));
		}
		else
		{
			waitResult = futureObj.wait_for(std::chrono::milliseconds(delay));
		}
	} while (waitResult == std::future_status::timeout);

	pThis->_opencvCaputure.release();
}