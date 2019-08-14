/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/

#include "APIException.h"

APIException::APIException(char const* const message) throw() : std::runtime_error(message)
{
	this->message = message;
}

char const* APIException::what() const throw()
{
	return this->message.c_str();
}