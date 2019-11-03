/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/


#pragma once

#ifndef CRITIMON_H
#define CRITIMON_H
#endif

#ifndef RAPIDJSON_HAS_STDSTRING
#define RAPIDJSON_HAS_STDSTRING 1
#endif
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <vector>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <map>
#include "APIHandler.h"
#include <exception>
#include <stdexcept>
#include <boost/locale/info.hpp>
#include <iostream>
#include <locale>
#include <boost/stacktrace.hpp>
#include <typeinfo>
#include <boost/filesystem.hpp>
#include <fstream>

#include <cstdlib>


#ifdef _WIN32
#include "wtypes.h"
#else
#include <sys/utsname.h>
#endif

class CritiMon
{
public:
	CritiMon() {};
	enum CrashSeverity { Low, Medium, Major, Critical };
	void Initialise(std::string& api_key, std::string& app_id, std::string& version, void(*eventcallback)(int statusCode, std::string message) = nullptr);
	void SendCrash(std::exception& exception, CritiMon::CrashSeverity crashSeverity, void(*eventcallback)(int statusCode, std::string message) = nullptr);
	void SendCrash(std::exception& exception, CritiMon::CrashSeverity crashSeverity, std::map<std::string, std::string> customProperties, void(*eventcallback)(int statusCode, std::string message) = nullptr);
	void SendCrash(std::exception& exception, CritiMon::CrashSeverity crashSeverity, std::string customPropertyKey, std::string customPropertyValue, void(*eventcallback)(int statusCode, std::string message) = nullptr);
	void OverrideAPIURL(std::string apiUrl);
	void shouldDisableSSLPeerVerification(bool disable);
	static std::string SessionID;
	static std::string DOLB;
	static bool terminateHandlerInstalled;
	static bool disableSSLPeerVerification;
	static std::string api_url;
	void sendUnhandledCrash();
	static std::string APIKey;
	static std::string DeviceID;
	static std::string AppID;
	static std::string Version;
	static std::vector<std::map<std::string, std::string>> retryCrashQueue;
	static bool CritiMonInitialised;

private:
	std::map<std::string, std::string> getPostData(std::exception& exception, CritiMon::CrashSeverity crashSeverity, bool handledCrash);
	std::string returnBacktrace();
	void SendCrash(std::map<std::string, std::string> postData);
	void parseStacktrace(std::string& backtrace, std::string* className, int* lineNo, bool handledCrash);
	void GetDesktopResolution(int& horizontal, int& vertical);
	std::string getSeverityString(CritiMon::CrashSeverity& severity);
	std::string generateDeviceID();
	void getOSDetails(std::string *osName, std::string *build);

	std::string readCurrentVersionRegKey(std::string key);
	std::string getArchitecture();
	
	

	
	static void unhandledTerminateHandler();

};

