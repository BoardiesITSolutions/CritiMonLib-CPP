/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/


#pragma once
#ifndef COOKIEOBJ_H
#define COOKIEOBJ_H

#include <stdio.h>
#include <stdlib.h>
#include <string>

class CookieObj
{
public :
	void setDomain(std::string& domain);
	void setIncludeSubdomain(std::string& includeSubdomain);
	void setPath(std::string& path);
	void setSecureTransport(std::string& secureTransport);
	void setExpires(std::string& expires);
	void setName(std::string& name);
	void setValue(std::string& value);

	std::string getDomain();
	std::string getIncludeSubdomains();
	std::string getPath();
	std::string getSecureTransport();
	std::string getExpires();
	std::string getName();
	std::string getValue();
private:
	std::string domain;
	std::string includeSubdomains;
	std::string path;
	std::string secureTransport;
	std::string expires;
	std::string name;
	std::string value;
};

#endif

