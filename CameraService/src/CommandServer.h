#pragma once
/*
	CommandServer - listen for command messages on well known port
*/

#include <zmq.hpp>
#include "Message.h"
#include <thread>
#include <future>

class CommandServer
{
private:
	CommandServer();
	~CommandServer();

private:
// Properties

	// Exit thread signal
	std::unique_ptr<std::promise<void>> _stopReadThreadSignal;

	// Streaming thread
	std::thread _readSubThread;

public:
	// Methods
	static CommandServer& Instance()
	{
		static CommandServer instance;
		return instance;
	}

	// Start streaming video from hardware camera
	bool Start(std::wstring& error);

	// Stop reading video frames
	bool Stop(std::wstring& error);

	// Server read messages thread
	static void ReadThread(CommandServer* pThis, std::future<void> futureObj, std::unique_ptr<zmq::socket_t> subscriber, std::unique_ptr<zmq::context_t> context);

	// Execute command and return result
	bool RunCommand(CMessage& command, std::wstring& error);
};