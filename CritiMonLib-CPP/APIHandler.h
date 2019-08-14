/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions 
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/

#pragma once
#ifndef APIHANDLER_H
#define APIHANDLER_H
#endif

#include "CritiMon.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include "CookieObj.h"

#include "APIException.h"


class APIHandler
{
public:
	APIHandler(std::map<std::string, std::string>& postData);
	enum API_METHOD {Initialise, SendCrash};
	void execute(API_METHOD apiMethod, void(*eventcallback)(int statusCode, std::string message) = nullptr);
	std::string curlResponse = "";
	std::string curlResponseHeaders = "";
	size_t curlResponseWriteCallback(void* ptr, size_t size, size_t nmemb, void* stream);
	size_t curlResponseHeaderCallback(void* ptr, size_t size, size_t nmemb, void* stream);
private:
	std::map<std::string, std::string> postData;
	std::string getStringFromAPIMethod(APIHandler::API_METHOD& apiMethod);
	std::string returnPostDataString(APIHandler::API_METHOD& apiMethod);
	CookieObj storeCookie(std::string& cookieString);
	void sendCallbackResult(int result, std::string message = "", void(*eventcallback)(int statusCode, std::string message) = nullptr);
};
extern "C"
{
	size_t curlStaticAPIHandler(void* ptr, size_t size, size_t nmemb, void* stream);
	size_t curlStaticHeader(void* ptr, size_t size, size_t nmemb, void* stream);
}

