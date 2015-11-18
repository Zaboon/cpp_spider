#pragma once

#include		<iostream>
#include		<sstream>
#include		<vector>

class BBException : public std::exception
{
public:
	BBException(const std::string &message) throw();
	BBException(const std::string &message, int line) throw();
	virtual ~BBException() throw ();
	const char * what() const throw();

protected:
	std::string _message;
};
