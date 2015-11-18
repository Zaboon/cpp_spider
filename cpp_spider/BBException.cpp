#include		"BBException.hpp"

BBException::BBException(const std::string & message) throw() : _message(message) {
}

BBException::~BBException() throw() {
}

BBException::BBException(const std::string & message, int line) throw() {

	std::ostringstream ss;
	ss << line;
	this->_message = "Line " + ss.str() + ": " + message;
}

const char *        BBException::what() const throw () {
	return (this->_message.c_str());
}