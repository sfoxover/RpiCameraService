#include "defines.h"
#include "CommandServer.h"
#include <future>
#include <Settings.h>
#include <MessageHelper.h>
#include <helpers.h>
#include <DetectFaces.h>

CommandServer::CommandServer()
{
}

CommandServer::~CommandServer()
{
	std::wstring error;
	bool bOK = Stop(error);
	assert(bOK);
}

// Start reading client requests on new thread
bool CommandServer::Start(std::wstring& error)
{
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

	_stopReadThreadSignal = std::make_unique<std::promise<void>>();
	auto futureObj = _stopReadThreadSignal->get_future();
	_readSubThread = std::thread(&CommandServer::ReadThread, this, std::move(futureObj), std::move(server), std::move(context));
	return true;
}

// Stop video streaming
bool CommandServer::Stop(std::wstring& error)
{
	if (_stopReadThreadSignal)
	{
		_stopReadThreadSignal->set_value();
		_stopReadThreadSignal.reset(nullptr);
	}
	if (_readSubThread.joinable())
		_readSubThread.join();

	return true;
}

// Message topics read thread
void CommandServer::ReadThread(CommandServer* pThis, std::future<void> futureObj, std::unique_ptr<zmq::socket_t> server, std::unique_ptr<zmq::context_t> context)
{
	long delay = 10;
	std::future_status waitResult;
	do
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
			
			waitResult = futureObj.wait_for(std::chrono::milliseconds(1));
		}
		else
		{
			waitResult = futureObj.wait_for(std::chrono::milliseconds(delay));
		}
	} while (waitResult == std::future_status::timeout);
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
		// Change Face detect AI method
		if (runCommand == "SetAIMethod")
		{
			bool bOK = CDetectFaces::Instance().Initialize(commandArgs, error);
			if (bOK)
			{
				CSettings::Instance().SetUseFaceDetect(commandArgs != "Off");
			}
			return bOK;
		}
		else if (runCommand == "GetAIMethod")
		{
			auto result = CDetectFaces::Instance().GetDetectMethod();
			command.SetHeaderMapValue("AIMethod", result);
			return true;
		}
	}

	std::wstringstream szErr;
	szErr << L"Error, unsupported command:" << Helpers::Utf8ToWide(runCommand) << L", args:" << Helpers::Utf8ToWide(commandArgs) << ".";
	error = szErr.str();
	return false;
}