#include "VideoSource.h"
#include "defines.h"
#include "helpers.h"
#include <cassert>
#include "PublishMessage.h"
#include "DetectFaces.h"
#include <inttypes.h>
#include <MessageHelper.h>
#include "Settings.h"
#include "MessageFactory.h"

CVideoSource::CVideoSource()
{
	SetStoppingFlag(false);
}

CVideoSource::~CVideoSource()
{
	std::wstring error;
	bool bOK = Stop(error);
	assert(bOK);
}

// Get set for _stoppingFlag
void CVideoSource::GetStoppingFlag(bool &value)
{
	_stoppingFlagLock.lock();
	value = _stoppingFlag;
	_stoppingFlagLock.unlock();
}

void CVideoSource::SetStoppingFlag(bool value)
{
	_stoppingFlagLock.lock();
	_stoppingFlag = value;
	_stoppingFlagLock.unlock();
}

// Start streaming video from hardware camera
bool CVideoSource::Start(std::wstring &error)
{
	bool bOK = Stop(error);
	if (!bOK)
	{
		return false;
	}
	_opencvCaputure.release();
	
	// Check settings to use sample video
	if (CSettings::Instance().GetUseSampleVideo())
	{
		auto filePath = Helpers::AppendToRunPath(Helpers::AppendPath("assets", CSettings::Instance().GetSampleVideoName()));
		bool exists = Helpers::FileExists(filePath);
		assert(exists);
		_opencvCaputure.open(filePath);
		if (!_opencvCaputure.isOpened())
		{
			error = L"Error opening video file.";
			return false;
		}
	}
	else
	{
		// Open default video cam
		_opencvCaputure.open(0);
		if (!_opencvCaputure.isOpened())
		{
			error = L"Error opening video camera.";
			return false;
		}
	}
	SetStoppingFlag(false);
	_videoThread = std::thread(&CVideoSource::VideoStreamingThread, this);
	return true;
}

// Stop video streaming
bool CVideoSource::Stop(std::wstring &error)
{
	SetStoppingFlag(true);
	_stopEvent.notify_all();

	if (_videoThread.joinable())
		_videoThread.join();
	return true;
}

// Video streaming thread
void CVideoSource::VideoStreamingThread(CVideoSource *pThis)
{
	double sourceFps = pThis->_opencvCaputure.get(cv::CAP_PROP_FPS);
	if (sourceFps == 0)
		sourceFps = 30;

	bool useSampleVideo = CSettings::Instance().GetUseSampleVideo();
	uint64_t frameCount = 0;
	bool bOK = true;
	std::wstring error;
	double fpsDelay = (double)(1000 / sourceFps);
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
		pThis->GetStoppingFlag(exiting);
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

				// Encode image
				std::vector<uchar> jpgBuffer;
				cv::imencode(".jpg", image, jpgBuffer);
				
				// Load message with video frame
				CMessage msg = MessageFactory::Create(topic, CMessage::MessageType::JpegFrame | CMessage::MessageType::Video, jpgBuffer);
				msg.SetHeaderMapValue("fps", sourceFps);
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
			std::unique_lock<std::mutex> lock(pThis->_stopEventLock);
			pThis->_stopEvent.wait_for(lock, std::chrono::milliseconds((uint64_t)waitDelay));
		}
	} 
	while (!exiting);

	pThis->_opencvCaputure.release();
}

// Publish detected face images to zeroMq subscribers
void CVideoSource::PublishDetectedFaces(cv::Mat image)
{
	// Encode image
	std::vector<uchar> jpgBuffer;
	cv::imencode(".jpg", image, jpgBuffer);

	std::wstring error;
	CMessage msg = MessageFactory::Create(CSettings::Instance().GetFaceDetectTopic(), CMessage::MessageType::JpegFrame | CMessage::MessageType::Video | CMessage::MessageType::FaceDetection, jpgBuffer);
	bool bOK = CPublishMessage::Instance().SendMessageData(msg, error);
	assert(bOK);
}