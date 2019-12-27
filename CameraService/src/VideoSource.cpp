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

	bool useSampleVideo = CSettings::Instance().GetUseSampleVideo();
	uint64_t frameCount = 0;
	bool bOK = true;
	std::wstring error;
	double delay = (double)(1000 / fps);
	std::string topic;
	if (useSampleVideo)
	{
		topic = CSettings::Instance().GetVideoSampleTopic();
	}
	else
	{
		topic = CSettings::Instance().GetVideoCamTopic();
	}
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
					CDetectFaces::Instance().AddImageToQueue(image);
				}
			}
			
			// Load message with video frame
			CMessage msg;
			msg.CreateMessageFromMatFrame(topic, image, fps);
			bOK = CPublishMessage::Instance().SendMessageData(msg, error);
			assert(bOK);
			
			waitResult = futureObj.wait_for(std::chrono::milliseconds((int64_t)delay));
		}
		else
		{
			waitResult = futureObj.wait_for(std::chrono::milliseconds(10));

			// Restart sample video from beginning
			if (CSettings::Instance().GetUseSampleVideo())
			{
				pThis->_opencvCaputure.set(cv::CAP_PROP_POS_FRAMES, 0);
			}
		}
	} while (waitResult == std::future_status::timeout);

	pThis->_opencvCaputure.release();
}

// Publish detected face images to zeroMq subscribers
void CVideoSource::PublishDetectedFaces(cv::Mat image)
{
	std::wstring error;
	CMessage msg;
	msg.CreateMessageFromFaceDetectedMatFrame(CSettings::Instance().GetFaceDetectTopic(), image);
	bool bOK = CPublishMessage::Instance().SendMessageData(msg, error);
	assert(bOK);
}