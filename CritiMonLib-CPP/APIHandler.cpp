/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/


#include "APIHandler.h"
#include <curl/curl.h>

using namespace std;

APIHandler::APIHandler(std::map<std::string, std::string>& postData)
{
	this->postData = postData;
}

void APIHandler::execute(APIHandler::API_METHOD apiMethod, void(*eventcallback)(int statusCode, std::string message))
{
	CURL* curl = NULL;

	curl = curl_easy_init();
	if (curl)
	{
		struct curl_slist* headers = NULL;
		string api_key = postData["APIKey"];
		stringstream authorisation_token;
		authorisation_token << "authorisation-token:" << postData["APIKey"];
		if (apiMethod == APIHandler::API_METHOD::SendCrash)
		{
			stringstream cookies;
			cookies << "SESSIONID=" << CritiMon::SessionID << "; " << endl;
			curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.str().c_str());
			//headers = curl_slist_append(headers, cookies.str().c_str());
		}

		headers = curl_slist_append(headers, authorisation_token.str().c_str());
		headers = curl_slist_append(headers, "Expect:");
		string url = CritiMon::api_url + this->getStringFromAPIMethod(apiMethod);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POST, true);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlStaticAPIHandler);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "");
		//curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &curlStaticHeader);
		string postDataString = this->returnPostDataString(apiMethod);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postDataString.c_str());
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			this->sendCallbackResult(-1, curl_easy_strerror(res), eventcallback);
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			return;
		}
		long httpResponseCode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);


		if (httpResponseCode != 200)
		{
			switch (httpResponseCode)
			{
				case 403:
					if (apiMethod == APIHandler::API_METHOD::SendCrash && CritiMon::CritiMonInitialised) //Not initialised while sending crash
					{
						CritiMon::retryCrashQueue.push_back(this->postData);
						CritiMon critiMon;
						critiMon.Initialise(CritiMon::APIKey, CritiMon::AppID, CritiMon::Version, eventcallback);
					}
					this->sendCallbackResult(1, "Acess denied, check that your api key is correct and that you call initialise before attempting to send a crash", eventcallback);
					break;
				case 404:
					this->sendCallbackResult(1, "Application id was not found", eventcallback);
					break;
				default:
					this->sendCallbackResult(1, "Unknown error occurred", eventcallback);
			}
			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);
			return;
		}
		else
		{
			if (apiMethod == APIHandler::API_METHOD::Initialise)
			{
				struct curl_slist* cookies;
				curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);

				int i = 1;
				std::vector<CookieObj> cookieObjects;
				struct curl_slist* current = cookies;
				if (current != NULL)
				{
					string cookieString = string(current->data);
					CookieObj cookieObj = storeCookie(cookieString);
					cookieObjects.push_back(cookieObj);

					while (current->next != NULL)
					{
						string cookieString = string(current->data);
						CookieObj cookieObj = this->storeCookie(cookieString);
						cookieObjects.push_back(cookieObj);
						current = cookies->next;

						i++;
					}

					for (std::vector<CookieObj>::iterator it = cookieObjects.begin(); it != cookieObjects.end(); ++it)
					{
						if (it->getName().find("SESSIONID") != string::npos)
						{
							CritiMon::SessionID = it->getValue();
						}
						else if (it->getName().find("DO-LB") != string::npos)
						{
							CritiMon::DOLB = it->getValue();
						}
					}
					if (cookies != NULL)
					{
						curl_slist_free_all(cookies);
					}
				}

				if (CritiMon::SessionID.empty())
				{
					this->sendCallbackResult(-1, "Session ID cookie was not found in CritiMon response - can't continue", eventcallback);
					//curl_slist_free_all(headers);
					//curl_easy_cleanup(curl);
				}
			}
		}

		if (!curlResponse.empty())
		{
			rapidjson::Document jsonObject;
			jsonObject.Parse(curlResponse.c_str());
			int result = jsonObject["result"].GetInt();
			if (apiMethod == APIHandler::API_METHOD::Initialise && CritiMon::retryCrashQueue.size() > 0)
			{
				//Retry the crashes
				for (std::vector<std::map<std::string, std::string >> ::iterator it = CritiMon::retryCrashQueue.begin(); it != CritiMon::retryCrashQueue.end(); ++it)
				{
					APIHandler apiHandler(*it);
					apiHandler.execute(APIHandler::API_METHOD::SendCrash);
				}
			}
			if (apiMethod == APIHandler::API_METHOD::Initialise && result == 0)
			{
				CritiMon::CritiMonInitialised = true;
			}
			else if (result == 5) //Undergoing maintenance retry maintenance
			{
				CritiMon critimon;
				critimon.Initialise(CritiMon::APIKey, CritiMon::AppID, CritiMon::Version);
			}
			string message = string(jsonObject["message"].GetString());
			this->sendCallbackResult(result, message, eventcallback);
		}
		else
		{
			this->sendCallbackResult(0, "", eventcallback);
		}
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}
	else
	{
		this->sendCallbackResult(-1, "Failed to initialise curl", eventcallback);
	}
}

CookieObj APIHandler::storeCookie(std::string& cookieString)
{
	char* token = NULL;
	char* writable = new char[cookieString.length() + 1];
	memset(writable, 0, cookieString.length()+1);
	std::copy(cookieString.begin(), cookieString.end(), writable);
	token = strtok(writable, "\t");
	int i = 0;
	CookieObj cookie;
	string tokenString = token;
	while (token != NULL)
	{
		
		switch (i)
		{
		case 0:
			cookie.setDomain(tokenString);
			break;
		case 1:
			cookie.setIncludeSubdomain(tokenString);
			break;
		case 2:
			cookie.setPath(tokenString);
			break;
		case 3:
			cookie.setSecureTransport(tokenString);
			break;
		case 4:
			cookie.setExpires(tokenString);
			break;
		case 5:
			cookie.setName(tokenString);
			break;
		case 6:
			cookie.setValue(tokenString);
			break;
		}
		token = strtok(NULL, "\t");
		if (token != NULL)
		{
			tokenString = token;
		}
		i++;
	}
	delete[] writable;
	return cookie;
}

std::string APIHandler::returnPostDataString(APIHandler::API_METHOD& apiMethod)
{
	stringstream postStream;
	switch (apiMethod)
	{
		case APIHandler::API_METHOD::Initialise:
			postStream << "ApplicationID=" << this->postData["AppID"] << "&DeviceID=" << postData["DeviceID"] << "&AppVersion=" << postData["AppVersion"];
			break;
		case APIHandler::API_METHOD::SendCrash:
			postStream << "AppID=" << this->postData["AppID"] << "&DeviceID=" << this->postData["DeviceID"] << "&VersionName=" << this->postData["VersionName"] <<
				"&Severity=" << this->postData["Severity"] << "&Stacktrace=" << this->postData["Stacktrace"] << "&ScreenResolution=" << this->postData["ScreenResolution"] <<
				"&ExceptionMessage=" << this->postData["ExceptionMessage"] << "&ExceptionType=" << this->postData["ExceptionType"] << "&ClassFile=" << this->postData["ClassFile"] <<
				"&LineNo=" << this->postData["LineNo"] << "&DeviceType=" << this->postData["DeviceType"] << "&CrashType=" << this->postData["CrashType"] << "&OSName=" <<
				this->postData["OSName"] << "&OSBuild=" << this->postData["OSBuild"] << "&Architecture=" << this->postData["Architecture"];

			if (this->postData.find("CustomProperty") != postData.end())
			{
				postStream << "&CustomProperty=" << this->postData["CustomProperty"];
			}
			break;
		default:
			const char* error = "Invalid api method";
			throw APIException(error);
	}
	return postStream.str();
}

std::string APIHandler::getStringFromAPIMethod(APIHandler::API_METHOD& apiMethod)
{
	switch (apiMethod)
	{
		case API_METHOD::Initialise:
			return std::string("/initialise");
		case API_METHOD::SendCrash:
			return std::string("/crash");
		default:
			const char* error = "Invalid api method";
			throw APIException(error);
	}
}

size_t curlStaticAPIHandler(void* contents, size_t size, size_t nmemb, void* userp)
{
	APIHandler* apiHandler = (APIHandler*)userp;
	apiHandler->curlResponseWriteCallback(contents, size, nmemb, userp);
	return size * nmemb;
}

size_t curlStaticHeader(void* contents, size_t size, size_t nmemb, void* userp)
{
	APIHandler* apiHandler = (APIHandler*)userp;
	apiHandler->curlResponseHeaderCallback(contents, size, nmemb, userp);
	return size * nmemb;
}

size_t APIHandler::curlResponseWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	curlResponse.append((char*)contents, size * nmemb);
	return size * nmemb;
}

size_t APIHandler::curlResponseHeaderCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	curlResponseHeaders.append((char*)contents, size * nmemb);
	return size * nmemb;
}

void APIHandler::sendCallbackResult(int result, std::string message, void(*eventcallback)(int statusCode, std::string message))
{
	if (eventcallback != nullptr)
	{
		eventcallback(result, message);
	}
}