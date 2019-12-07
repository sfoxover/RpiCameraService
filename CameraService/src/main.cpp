#include "defines.h"
#include "VideoSource.h"
#include "PublishMessage.h"
#include "defines.h"
#include "helpers.h"
#include "MessageHelper.h"
#include <cassert>
#include <filesystem>
#ifdef _WIN32
#include "windows.h"
#endif
#include "Settings.h"
#include "DetectFaces.h"

int main()
{
	bool bOK = true;
	std::wstring error;
    std::string path = Helpers::AppendToRunPath("message_developer.json");
	bOK = CSettings::Instance().Initialize(path, error);
	if(!bOK)
	{
		std::wcout << L"Camera Service settings initialize failed. Error, " << error << std::endl;
	}
	assert(bOK);

	// Initialize facial detection
	if(CSettings::Instance().GetUseFaceDetect())
	{
		CDetectFaces::Instance().Initialize(CSettings::Instance().GetFaceDetectMethod());
	}

	// Set up publisher endpoints
	bOK = CPublishMessage::Instance().Initialize(CSettings::Instance().GetPublishUris(), error);
	assert(bOK);
	if (bOK)
	{
		std::wcout << L"Camera Service initialized OK" << std::endl;
	}
	else
	{
		std::wcout << L"Camera Service initialize failed. Error, " << error << std::endl;
	}

	// Load sample video
	if (CSettings::Instance().GetUseSampleVideo())
	{
		auto filePath = Helpers::AppendToRunPath(CSettings::Instance().GetSampleVideoName());
		bool exists = Helpers::FileExists(filePath);
		assert(exists);

		// Publish video stream
		bOK = CVideoSource::Instance().Start(Helpers::Utf8ToWide(filePath), error);
		assert(bOK);
	}
	else
	{
		// Publish camera stream
		bOK = CVideoSource::Instance().Start(error);
		assert(bOK);
	}

	// Start motion sensor on GPIO 7 pin 26
//	CMotionSensor::Instance().Start(7, error);

	// Wait for enter
	std::cin.get();

	return 0;
}
