#include "defines.h"
#include "CommandServer.h"
#include <Settings.h>
#include <MessageHelper.h>
#include <helpers.h>
#include <DetectFaces.h>
#include "VideoSource.h"

using namespace std::chrono_literals;

CommandServer::CommandServer()
{
	SetStoppingFlag(false);
}

CommandServer::~CommandServer()
{
	std::wstring error;
	bool bOK = Stop(error);
	assert(bOK);
}

// Get set for _stoppingFlag
void CommandServer::GetStoppingFlag(bool& value)
{
	_stoppingFlagLock.lock();
	value = _stoppingFlag;
	_stoppingFlagLock.unlock();
}

void CommandServer::SetStoppingFlag(bool value)
{
	_stoppingFlagLock.lock();
	_stoppingFlag = value;
	_stoppingFlagLock.unlock();
}

// Start reading client requests on new thread
bool CommandServer::Start(std::wstring& error)
{
	bool bOK = Stop(error);
	if (!bOK)
	{
		return false;
	}

	// zeromq subscriber object
	auto uri = CSettings::Instance().GetCmdServerUri();
	if (uri.empty())
	{
		error = L"Error CmdServerEndpoint is missing from the json config file";
		return false;
	}
	auto context = std::make_unique<zmq::context_t>(1);
	auto server = std::make_unique<zmq::socket_t>(*context, ZMQ_REP);
	server->bind(uri);
	std::cout << "Command server listening on " << uri << std::endl;

	SetStoppingFlag(false);
	_readSubThread = std::thread(&CommandServer::ReadThread, this, std::move(server), std::move(context));
	return true;
}

// Stop video streaming
bool CommandServer::Stop(std::wstring& error)
{
	SetStoppingFlag(true);
	_stopEvent.notify_all();

	if (_readSubThread.joinable())
		_readSubThread.join();

	return true;
}

// Message topics read thread
void CommandServer::ReadThread(CommandServer* pThis, std::unique_ptr<zmq::socket_t> server, std::unique_ptr<zmq::context_t> context)
{
	bool exiting = false;
	do
	{
		pThis->GetStoppingFlag(exiting);
		if (!exiting)
		{
			zmq::message_t frame;
			auto result = server->recv(frame, zmq::recv_flags::dontwait);
			if (result.has_value() && result.value() > 0)
			{
				// Read message command
				std::vector<unsigned char> buffer((const char*)frame.data(), (const char*)frame.data() + frame.size());
				CMessage message;
				message.DeserializeBufferToMessage(buffer);

				// Run command and return results
				std::wstring error;
				bool bOK = pThis->RunCommand(message, error);
				message.SetHeaderMapValue("result", bOK);
				message.SetHeaderMapValue("error", error);

				// Send reply
				buffer.clear();
				message.SerializeMessageToBuffer(buffer);
				size_t count = server->send(&buffer[0], buffer.size());
				assert(count == buffer.size());
			}
			std::unique_lock<std::mutex> lock(pThis->_stopEventLock);
			pThis->_stopEvent.wait_for(lock, 100ms);
		}
	} while (!exiting);
}

// Execute command and return result
bool CommandServer::RunCommand(CMessage& command, std::wstring& error)
{
	assert(command.GetType() == CMessage::ServerCommand);
	if (command.GetType() != CMessage::ServerCommand)
	{
		std::wstringstream szErr;
		szErr << L"Error unexpected message type " << command.GetType() << L".";
		error = szErr.str();
		return false;
	}

	// Get command type and value
	std::string runCommand;
	std::string commandArgs;
	std::any value;
	if (command.GetHeaderMapValue("Command", value))
	{
		CASTANY(value, runCommand);
	}
	if (command.GetHeaderMapValue("Value", value))
	{
		CASTANY(value, commandArgs);
	}
	assert(!runCommand.empty());
	if (!runCommand.empty())
	{
		if (runCommand == "SetAIMethod")
		{
			// Change Face detect AI method
			bool bOK = CDetectFaces::Instance().Start(commandArgs, &CVideoSource::PublishDetectedFaces, error);
			if (bOK)
			{
				CSettings::Instance().SetUseFaceDetect(commandArgs != "Off");
			}
			return bOK;
		}
		else if (runCommand == "GetAIMethod")
		{
			// Return current face detection method
			std::string method;
			CDetectFaces::Instance().GetDetectMethod(method);
			command.SetHeaderMapValue("AIMethod", method);
			return true;
		}
		else if (runCommand == "StopVideo")
		{
			// Stop video stream
			bool bOK = CVideoSource::Instance().Stop(error);
			assert(bOK);
			return bOK;
		}
		else if (runCommand == "StartVideo")
		{
			// Start video stream
			bool bOK = CVideoSource::Instance().Start(error);
			assert(bOK);
			return bOK;
		}
		else if (runCommand == "GetVideoPlaying")
		{
			// Return current face detection method
			bool stopped = false;
			CVideoSource::Instance().GetStoppingFlag(stopped);
			command.SetHeaderMapValue("VideoPlaying", !stopped);
			return true;
		}
	}

	std::wstringstream szErr;
	szErr << L"Error, unsupported command:" << Helpers::Utf8ToWide(runCommand) << L", args:" << Helpers::Utf8ToWide(commandArgs) << ".";
	error = szErr.str();
	return false;
}