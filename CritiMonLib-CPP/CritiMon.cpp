/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/


#include "CritiMon.h"
#include <csignal>
#include <exception>
#include "SignalException.h"
using namespace std;

//void CritiMon::Initialise(std::string& api_key, std::string& app_id, std::string& version);

string CritiMon::DOLB;
string CritiMon::SessionID;
string CritiMon::APIKey;
string CritiMon::DeviceID;
string CritiMon::AppID;
string CritiMon::Version;
//string CritiMon::api_url = "https://engine.critimon.com";
string CritiMon::api_url = "http://192.168.1.118:500";
bool  CritiMon::disableSSLPeerVerification = false;

bool CritiMon::CritiMonInitialised = false;
//std::string SessionID = "";
std::string APIKey = "";
std::string AppID = "";
bool CritiMon::terminateHandlerInstalled = false;
std::vector<std::map<string, string>> CritiMon::retryCrashQueue = std::vector<std::map<string, string>>();
static SignalException* signalException = NULL;
CritiMon *CritiMon::critimon = NULL;

/**
* You should not use this method. This is used only for Boardies IT Solutions for testing purposes. 
* If you change this URL your crashes will not be sent to the CritiMon service
*/
void CritiMon::OverrideAPIURL(string overrideUrl)
{
	CritiMon::api_url = overrideUrl;
}

void CritiMon::shouldDisableSSLPeerVerification(bool disable)
{
	CritiMon::disableSSLPeerVerification = disable;
}

void CritiMon::signalHandler(int signum)
{
	
	switch (signum)
	{
		case SIGABRT:
			signalException = new SignalException(signum, "An abort signal was received");
			break;
		case SIGFPE:
			signalException = new SignalException(signum, "Erroneous Arithmetic Operation Signal Received");
			break;
		case SIGILL:
			signalException = new SignalException(signum, "Processor Command Error Signal Received");
			break;
		case SIGSEGV:
			signalException = new SignalException(signum, "Memory Access Violation Signal Received");
			break;
		default:
			signalException = new SignalException(signum, "An unexpected signal was detected");
			break;
	}


	if (CritiMon::critimon != NULL)
	{
		CritiMon::critimon->SendCrash(*signalException, CritiMon::CrashSeverity::Critical);
	}
	exit(EXIT_FAILURE);
	

	//CritiMon::SendCrash((SignalException&)signalException, CritiMon::CrashSeverity::Critical);
}

void CritiMon::Initialise(std::string& api_key, std::string& app_id, std::string& version, void(*eventcallback)(int statusCode, std::string message))
{
	APIKey = api_key;
	AppID = app_id;
	Version = version;

	if (!boost::filesystem::exists("critimon.tmp"))
	{
		DeviceID = this->generateDeviceID();
		std::ofstream outfile("critimon.tmp");
		outfile << DeviceID;
		outfile.close();
	}
	else
	{
		fstream file("critimon.tmp", fstream::in);
		string s;
		getline(file, s, '\0'); //Read the entire contents of the file (there won't be a \0 in the file)
		file.close();
		if (!s.empty())
		{
			DeviceID = s;
		}
		else
		{
			DeviceID = this->generateDeviceID();
			std::ofstream outfile("critimon.tmp");
			outfile << DeviceID;
			outfile.close();
		}
	}

	//Set up the termination handler
	if (!CritiMon::terminateHandlerInstalled)
	{
		std::set_terminate(&CritiMon::unhandledTerminateHandler);
		CritiMon::terminateHandlerInstalled = true;
	}
	
	//register the signals
	//We don't register SIGINT as this is a usually a user initiatted graceful shutdown so not a failure/crash
	signal(SIGABRT, CritiMon::signalHandler); //Critical error detected such as a double free or memory allocation failure
	signal(SIGFPE, CritiMon::signalHandler); //Erroneous arithimetic operation such as divide by zero
	signal(SIGILL, CritiMon::signalHandler); //Processor command error
	signal(SIGSEGV, CritiMon::signalHandler); //Memory access violation


	std::map<std::string, std::string> postData;
	postData["APIKey"] = APIKey;
	postData["AppID"] = AppID;
	postData["AppVersion"] = Version;
	
	postData["DeviceID"] = DeviceID;

	APIHandler apiHandler(postData);
	apiHandler.execute(APIHandler::API_METHOD::Initialise, eventcallback);
	this->critimon = this;
}

void CritiMon::SendCrash(std::exception& exception, CritiMon::CrashSeverity crashSeverity, std::string customPropertyKey, std::string customPropertyValue, void(*eventcallback)(int statusCode, std::string message))
{
	std::map<string, string> postData = this->getPostData(exception, crashSeverity, true);

	stringstream customProperty;
	customProperty << "{\"" << customPropertyKey << "\":\"" << customPropertyValue << "\"}";
	postData["CustomProperty"] = customProperty.str();

	APIHandler apiHandler(postData);
	apiHandler.execute(APIHandler::API_METHOD::SendCrash, eventcallback);
}


void CritiMon::SendCrash(std::exception& exception, CritiMon::CrashSeverity crashSeverity, std::map<std::string, std::string> customProperties, void(*eventcallback)(int statusCode, std::string message))
{
	std::map<string, string> postData = this->getPostData(exception, crashSeverity, true);

	rapidjson::Document document;
	document.SetObject();

	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

	for (std::map<string, string>::iterator it = customProperties.begin(); it != customProperties.end(); it++)
	{
		rapidjson::Value key(it->first.c_str(), allocator);
		rapidjson::Value value(it->second.c_str(), allocator);
		document.AddMember(key, value, allocator);
	}

	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer>writer(buffer);
	document.Accept(writer);

	string propertiesJSON = buffer.GetString();
	postData["CustomProperty"] = propertiesJSON;

	APIHandler apiHandler(postData);
	apiHandler.execute(APIHandler::API_METHOD::SendCrash, eventcallback);

}

std::map<std::string, std::string> CritiMon::getPostData(std::exception& exception, CritiMon::CrashSeverity crashSeverity, bool handledCrash)
{

	string stacktrace = this->returnBacktrace();
	string className = "";
	int lineNo = 0;
	this->parseStacktrace(stacktrace, &className, &lineNo, handledCrash);
	int x = 0, y = 0;
#ifdef _WIN32
	GetDesktopResolution(x, y);
#endif
	stringstream screenResolutionString;
	if (x != 0 || y != 0)
	{
		screenResolutionString << x << " x " << y;
	}
	else
	{
		screenResolutionString << "Unknown";
	}
	string exceptionType = typeid(exception).name();

	if (exceptionType.compare("SignalException") != 0)
	{
		SignalException signalException = (SignalException&)exception;
		exceptionType = signalException.getExceptionType();
	}
	//exceptionType = exceptionType.replace(exceptionType.find("class "), exceptionType.length(), "");

	string osName = "";
	string osBuild = "";
	this->getOSDetails(&osName, &osBuild);

	std::map<std::string, std::string> postData;
	postData["APIKey"] = APIKey;
	postData["AppID"] = AppID;
	postData["DeviceID"] = DeviceID;
	postData["Stacktrace"] = stacktrace;
	postData["ScreenResolution"] = screenResolutionString.str();
	postData["ExceptionMessage"] = exception.what();
	postData["ExceptionType"] = exceptionType;
	postData["VersionName"] = Version;
	postData["ClassFile"] = className;
	postData["LineNo"] = to_string(lineNo);
	postData["DeviceType"] = "CPP";
	postData["Severity"] = this->getSeverityString(crashSeverity);
	postData["OSName"] = osName;
	postData["OSBuild"] = osBuild;
	postData["Architecture"] = this->getArchitecture();
	if (handledCrash)
	{
		postData["CrashType"] = "Handled";
	}
	else
	{
		postData["CrashType"] = "Unhandled";
	}
	return postData;
}

void CritiMon::SendCrash(std::exception& exception, CritiMon::CrashSeverity crashSeverity, void(*eventcallback)(int statusCode, std::string message))
{
	std::map<std::string, std::string> postData = getPostData(exception, crashSeverity, true);

	APIHandler apiHandler(postData);
	apiHandler.execute(APIHandler::API_METHOD::SendCrash, eventcallback);
}

std::string CritiMon::getSeverityString(CritiMon::CrashSeverity& severity)
{
	switch (severity)
	{
		case CritiMon::CrashSeverity::Low:
			return "Low";
		case CritiMon::CrashSeverity::Medium:
			return "Medium";
		case CritiMon::CrashSeverity::Major:
			return "Major";
		case CritiMon::CrashSeverity::Critical:
			return "Critical";
		default:
			throw APIException("Invalid crash severity");
	}
}

void CritiMon::GetDesktopResolution(int& horizontal, int& vertical)
{
#ifdef _WIN32
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	horizontal = desktop.right;
	vertical = desktop.bottom;
#else
	horizontal = 0;
	vertical = 0;
#endif
}

std::string CritiMon::returnBacktrace()
{
	stringstream stacktraceStream;
	stringstream processedStacktrace;
	string line;
	stacktraceStream << boost::stacktrace::stacktrace();

#ifdef _WIN32
	string lineSeparator = "\n";
#else
	string lineSeparator = "\\n";
#endif

	while (std::getline(stacktraceStream, line))
	{
		//The stacktrace includes the boost stacktrace library and the critimon library. We don't care about this, so skip
		//any lines that contain these files, we should be left with the actual stacktrace that triggered CritiMon
		if (line.find("stacktrace.hpp") != string::npos || line.find("CritiMon.cpp") != string::npos || line.find("CritiMon::") != string::npos)
		{
			continue;
		}
		processedStacktrace << line << endl;
	}
	return processedStacktrace.str();
}

void CritiMon::parseStacktrace(std::string& backtrace, std::string* className, int* lineNo, bool handledCrash)
{
#ifdef _WIN32
	char pathSeparator = '\\';
#else
	char pathSeparator = '/';
#endif

	string line = "";
	cout << backtrace << endl;
	if (handledCrash)
	{
		//Get the first line of the stacktrace, this will include the class file (or main) and line number of where the crash started
		std::size_t newLinePos = backtrace.find("\n");
		line = backtrace.substr(0, newLinePos);
	}
	else
	{
		std::istringstream stringStream(backtrace);
		while (std::getline(stringStream, line))
		{
			if (line.find(":") == string::npos || line.find("unhandled") != string::npos)
			{
				continue;
			}
			else
			{
				break;
			}
		}
	}

	std::size_t startOfClassName = line.find_last_of(pathSeparator) + 1;
	string classAndLineNo = line.substr(startOfClassName);
	std::size_t classAndLineSplitPos = classAndLineNo.find_last_of(":");
	*className = classAndLineNo.substr(0, classAndLineSplitPos);
	string temp = classAndLineNo.substr(classAndLineSplitPos + 1);
	*lineNo = atoi(temp.c_str());
}

std::string CritiMon::generateDeviceID()
{
	int length = 20;
	static const char alphanum[] =
		"0123456789abcdefghijklmnopqrstuvwxyz";

	stringstream randomString;

	//Generate a seed for the random number generator
	srand(time(0));
	for (int i = 0; i < length; ++i)
	{
		randomString << alphanum[rand() % (sizeof(alphanum) - 1)];
	}


	return randomString.str();

}

void CritiMon::getOSDetails(std::string *osName, std::string* build)
{
#ifdef _WIN32

	
	*osName = this->readCurrentVersionRegKey("ProductName");
	*build = this->readCurrentVersionRegKey("ReleaseId");


#else
	struct utsname name;
	if (!uname(&name))
	{
		*build = name.release;
		const char* systemReleaseFile = "/etc/system-release";
		if (boost::filesystem::exists(systemReleaseFile))
		{
			fstream file(systemReleaseFile, fstream::in);
			*osName = "";
			getline(file, *osName, '\0'); //Read the entire contents of the file (there won't be a \0 in the file)
			file.close();
		}
		else
		{
			*osName = "Unknown";
		}
	}
#endif
}

std::string CritiMon::readCurrentVersionRegKey(std::string key)
{
#ifdef _WIN32
	char value[255];
	DWORD BufferSize = 8192;
	RegGetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", key.c_str(), RRF_RT_ANY, NULL, (PVOID)& value, &BufferSize);

	return string(value);
#else
	return "";
#endif
}

std::string CritiMon::getArchitecture()
{
	if (sizeof(void*) == 8)
	{
		return "x64";
	}
	else if (sizeof(void*) == 4)
	{
		return "x86";
	}
	else
	{
		return "Unknown";
	}
}

void CritiMon::sendUnhandledCrash()
{
	std::runtime_error error("Unhandled Exception Detected");

	std::map<string, string> postData = this->getPostData(error, CrashSeverity::Critical, false);
	this->SendCrash(postData);
}

void CritiMon::SendCrash(std::map<std::string, std::string> postData)
{
	APIHandler apiHandler(postData);
	apiHandler.execute(APIHandler::API_METHOD::SendCrash);
}

void CritiMon::unhandledTerminateHandler()
{
	CritiMon critimon;
	critimon.sendUnhandledCrash();
	abort();
}
