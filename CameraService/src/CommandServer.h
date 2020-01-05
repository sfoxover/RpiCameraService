#pragma once
/*
	CommandServer - listen for command messages on well known port
*/

#include <zmq.hpp>
#include "Message.h"
#include <thread>

class CommandServer
{
private:
	CommandServer();
	~CommandServer();

private:
// Properties

	// Wait event will signal if exiting
	std::condition_variable _stopEvent;
	std::mutex _stopEventLock;

	// Stop video flag
	bool _stoppingFlag;
	std::mutex _stoppingFlagLock;

	// Streaming thread
	std::thread _readSubThread;

public:
	// Methods
	static CommandServer& Instance()
	{
		static CommandServer instance;
		return instance;
	}

	// Get set for _stoppingFlag
	void GetStoppingFlag(bool& value);
	void SetStoppingFlag(bool value);

	// Start streaming video from hardware camera
	bool Start(std::wstring& error);

	// Stop reading video frames
	bool Stop(std::wstring& error);

	// Server read messages thread
	static void ReadThread(CommandServer* pThis, std::unique_ptr<zmq::socket_t> subscriber, std::unique_ptr<zmq::context_t> context);

	// Execute command and return result
	bool RunCommand(CMessage& command, std::wstring& error);
};