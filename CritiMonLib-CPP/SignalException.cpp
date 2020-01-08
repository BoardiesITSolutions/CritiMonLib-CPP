#include "SignalException.h"
#include <iostream>
#include <string>
#include <sstream>

SignalException::SignalException(int signum, char const* const message) throw() : std::runtime_error(message)
{
	this->message = message;
	this->signum = signum;
}

char const* SignalException::what() const throw()
{
	return this->message.c_str();
}

int SignalException::getSignnum()
{
	return this->signum;
}

std::string SignalException::getExceptionType()
{
	switch (this->getSignnum())
	{
		case SIGABRT:
			return "Abort signal was received";
		case SIGFPE:
			return "Erroneous arithmeitic operation detected";
		case SIGILL:
			return "A processor command error was detected";
		case SIGSEGV:
			return "A memory access violation was detected";
		default:
			std::stringstream typeStream;
			typeStream << "Got unexpected signal. Signal number: " << this->getSignnum();
			return typeStream.str();
			break;
	}
}