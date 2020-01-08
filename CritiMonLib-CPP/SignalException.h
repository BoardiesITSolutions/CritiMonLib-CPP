#pragma once
#ifndef SIGNALEXCEPTION_H
#define SIGNALEXCEPTION_H

#include <exception>
#include <stdexcept>
#include <signal.h>

class SignalException : public std::runtime_error
{
public:
	SignalException() : std::runtime_error("") { };
	SignalException(int signum, char const * const message) throw();
	virtual char const* what() const throw();
	int getSignnum();
	std::string getExceptionType();
private:
	std::string message;
	int signum;
};
#endif

