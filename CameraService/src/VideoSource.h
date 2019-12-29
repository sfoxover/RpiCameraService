#pragma once
/*
	Read video data and send to pubisher
*/
#include <thread>
#include <future>
#include <chrono>
#include <condition_variable>
#include <opencv2/opencv.hpp>

class CVideoSource
{
private:
	CVideoSource();
	~CVideoSource();

private:
// Properties
	
	// Streaming thread
	std::thread _videoThread;

	// Wait event will signal if exiting
	std::condition_variable _stopWaitEvent;
	std::mutex _stopWaitEventLock;

	// Exit thread flag
	bool _exitingFlag;
	std::mutex _exitingFlagLock;

	// Open cv video capture
	cv::VideoCapture _opencvCaputure;

public:
// Methods
	static CVideoSource& Instance()
	{
		static CVideoSource instance;
		return instance;
	}

	// Get set for _exitingFlag
	void GetExitingFlag(bool &value);
	void SetExitingFlag(bool value);

	// Start streaming video from file
	bool Start(std::wstring filePath, std::wstring& error);
	
	// Start streaming video from hardware camera
	bool Start(std::wstring& error);

	// Stop video streaming
	bool Stop(std::wstring& error);

	// Video streaming thread
	static void VideoStreamingThread(CVideoSource* pThis);	

	// Publish detected face images to zeroMq subscribers
	static void PublishDetectedFaces(cv::Mat image);
};