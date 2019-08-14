/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/

#pragma once
#ifndef APIEXCEPTION_H
#define APIEXCEPTION_H

#include <exception>
#include <stdexcept>

class APIException : public std::runtime_error
{
public:
	APIException(char const* const message) throw();
	virtual char const* what() const throw();
private:
	std::string message;
};

#endif

