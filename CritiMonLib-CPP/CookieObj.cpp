/**
* CritiMon C++ Library
* Copyright (C) 2019 - Boardies IT Solutions
* https://www.critimon.com
* https://www.boardiesitsolutions.com
* https://support.boardiesitsolutions.com
*/


#include "CookieObj.h"

void CookieObj::setDomain(std::string& domain)
{
	this->domain = domain;
}

void CookieObj::setIncludeSubdomain(std::string& includeSubdomain)
{
	this->includeSubdomains = includeSubdomain;
}

void CookieObj::setPath(std::string& path)
{
	this->path = path;
}

void CookieObj::setSecureTransport(std::string& secureTransport)
{
	this->secureTransport = secureTransport;
}

void CookieObj::setExpires(std::string& expires)
{
	this->expires = expires;
}

void CookieObj::setName(std::string& name)
{
	this->name = name;
}

void CookieObj::setValue(std::string& value)
{
	this->value = value;
}

std::string CookieObj::getDomain()
{
	return this->domain;
}

std::string CookieObj::getIncludeSubdomains()
{
	return this->includeSubdomains;
}

std::string CookieObj::getPath()
{
	return this->path;
}

std::string CookieObj::getSecureTransport()
{
	return this->secureTransport;
}

std::string CookieObj::getExpires()
{
	return this->expires;
}

std::string CookieObj::getName()
{
	return this->name;
}

std::string CookieObj::getValue()
{
	return this->value;
}