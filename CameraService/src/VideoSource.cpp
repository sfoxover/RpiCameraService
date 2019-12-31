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
	SetExitingFlag(false);
}

CVideoSource::~CVideoSource()
{
	std::wstring error;
	bool bOK = Stop(error);
	assert(bOK);
}

// Get set for _exitingFlag
void CVideoSource::GetExitingFlag(bool &value)
{
	_exitingFlagLock.lock();
	value = _exitingFlag;
	_exitingFlagLock.unlock();
}

void CVideoSource::SetExitingFlag(bool value)
{
	_exitingFlagLock.lock();
	_exitingFlag = value;
	_exitingFlagLock.unlock();
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

	_videoThread = std::thread(&CVideoSource::VideoStreamingThread, this);
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

	_videoThread = std::thread(&CVideoSource::VideoStreamingThread, this);
	return true;
}

// Stop video streaming
bool CVideoSource::Stop(std::wstring &error)
{
	SetExitingFlag(true);
	_stopWaitEvent.notify_all();

	if (_videoThread.joinable())
		_videoThread.join();
	return true;
}

// Video streaming thread
void CVideoSource::VideoStreamingThread(CVideoSource *pThis)
{
	double fps = pThis->_opencvCaputure.get(cv::CAP_PROP_FPS);
	if (fps == 0)
		fps = 30;

	bool useSampleVideo = CSettings::Instance().GetUseSampleVideo();
	uint64_t frameCount = 0;
	bool bOK = true;
	std::wstring error;
	double fpsDelay = (double)(1000 / fps);
	std::string topic;
	if (useSampleVideo)
	{
		topic = CSettings::Instance().GetVideoSampleTopic();
	}
	else
	{
		topic = CSettings::Instance().GetVideoCamTopic();
	}
	bool exiting = false;
	do
	{
		pThis->GetExitingFlag(exiting);
		if (!exiting)
		{
			auto tmStart = std::chrono::high_resolution_clock::now();
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
			}
			else
			{
				// Restart sample video from beginning
				if (CSettings::Instance().GetUseSampleVideo())
				{
					pThis->_opencvCaputure.set(cv::CAP_PROP_POS_FRAMES, 0);
				}
			}

			// Subtract time taken to read frame from fps delay
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tmStart);
			auto waitDelay = fpsDelay - elapsed.count();
			if (waitDelay <= 0)
				waitDelay = 1;

			// Pause for correct fps
			std::unique_lock<std::mutex> lock(pThis->_stopWaitEventLock);
			pThis->_stopWaitEvent.wait_for(lock, std::chrono::milliseconds((uint64_t)waitDelay));
		}
	} 
	while (!exiting);

	pThis->_opencvCaputure.release();
}

// Publish detected face images to zeroMq subscribers
void CVideoSource::PublishDetectedFaces(cv::Mat image)
{
	std::wstring error;
	int imagesPerSecond = 0;
	CDetectFaces::Instance().GetImagesPerSecond(imagesPerSecond);
	CMessage msg;
	msg.CreateMessageFromFaceDetectedMatFrame(CSettings::Instance().GetFaceDetectTopic(), image, imagesPerSecond);
	bool bOK = CPublishMessage::Instance().SendMessageData(msg, error);
	assert(bOK);
}