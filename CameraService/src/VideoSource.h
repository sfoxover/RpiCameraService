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
	std::condition_variable _stopEvent;
	std::mutex _stopEventLock;

	// Stop video flag
	bool _stoppingFlag;
	std::mutex _stoppingFlagLock;

	// Open cv video capture
	cv::VideoCapture _opencvCaputure;

public:
// Methods
	static CVideoSource& Instance()
	{
		static CVideoSource instance;
		return instance;
	}

	// Get set for _stoppingFlag
	void GetStoppingFlag(bool &value);
	void SetStoppingFlag(bool value);

	// Start streaming video from hardware camera
	bool Start(std::wstring& error);

	// Stop video streaming
	bool Stop(std::wstring& error);

	// Video streaming thread
	static void VideoStreamingThread(CVideoSource* pThis);	

	// Publish detected face images to zeroMq subscribers
	static void PublishDetectedFaces(cv::Mat image);
};