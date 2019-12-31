#ifdef _WIN32
	#include "windows.h"
#endif
#include "defines.h"
#include "VideoSource.h"
#include "PublishMessage.h"
#include "defines.h"
#include "helpers.h"
#include "MessageHelper.h"
#include <cassert>
#include <filesystem>
#include "Settings.h"
#include "DetectFaces.h"
#include "CommandServer.h"
#include <ProfileMessages.h>

int main()
{
	std::ios_base::Init init;

	// Load settings from json file
	bool bOK = true;
	std::wstring error;
	std::string path = Helpers::AppendToRunPath("message_developer.json");
	bOK = CSettings::Instance().Initialize(path, error);
	assert(bOK);
	if (!bOK)
	{
		std::wcout << L"Camera Service settings initialize failed. Error, " << error << std::endl;
	}
	else
	{
		// Initialize facial detection
		if (CSettings::Instance().GetUseFaceDetect())
		{
			bOK = CDetectFaces::Instance().Initialize(CSettings::Instance().GetFaceDetectMethod(), &CVideoSource::PublishDetectedFaces, error);
			assert(bOK);
		}

		// Set up publisher endpoints
		if (bOK)
		{
			bOK = CPublishMessage::Instance().Initialize(CSettings::Instance().GetPublishUri(), error);
			assert(bOK);

			// Set up command server endpoints
			if (bOK)
			{
				bOK = CommandServer::Instance().Start(error);
				assert(bOK);

				// Load sample video
				if (bOK)
				{
					if (CSettings::Instance().GetUseSampleVideo())
					{
						auto filePath = Helpers::AppendToRunPath(Helpers::AppendPath("assets", CSettings::Instance().GetSampleVideoName()));
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
				}			
			}
		}

		// Start profiling messages
		if (bOK)
		{
			bOK = CProfileMessages::Instance().Start(error);
			assert(bOK);
		}
	}
	if (bOK)
	{
		std::wcout << L"Camera Service initialized OK" << std::endl;
	}
	else
	{
		std::wcout << L"Camera Service initialize failed. Error, " << error << std::endl;
	}

	// Wait for enter
	std::cin.get();

	return 0;
}
