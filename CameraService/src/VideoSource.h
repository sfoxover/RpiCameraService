#pragma once
/*
	Read video data and send to pubisher
*/
#include <thread>
#include <future>
#include <chrono>
#include <opencv2/opencv.hpp>

class CVideoSource
{
private:
	CVideoSource();
	~CVideoSource();

private:
// Properties
	// Exit thread signal
	std::unique_ptr<std::promise<void>> _stopThreadSignal;
	
	// Streaming thread
	std::thread _videoThread;

	// Open cv video capture
	cv::VideoCapture _opencvCaputure;

public:
// Methods
	static CVideoSource& Instance()
	{
		static CVideoSource instance;
		return instance;
	}

	// Start streaming video from file
	bool Start(std::wstring filePath, std::wstring& error);
	
	// Start streaming video from hardware camera
	bool Start(std::wstring& error);

	// Stop video streaming
	bool Stop(std::wstring& error);

	// Video streaming thread
	static void VideoStreamingThread(CVideoSource* pThis, std::future<void> futureObj);	
};