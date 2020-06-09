#ifndef CPP_FTP_UTILS_H
#define CPP_FTP_UTILS_H

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>
// for working with filesystem
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;

// endline for logger
enum loggerEndl {ENDL};

// logger class which outputs to stdout as well to log
// if we don't supply a log file name then we simply set the logfile ptr to nullptr
struct loggerT {

	// unique_ptr for easier checking
	// also it will automatically close on really bad errors
	std::unique_ptr<std::ofstream> logFile;

	loggerT(std::string_view);

	// operators for outputting various values
	template<typename T>
	loggerT& operator<<(T value) {
		std::cout << value;
		if (logFile)
			*logFile << value;
		return *this;
	}

	// only accept the specific ENDL value
	// send std::endl to both streams, effectively flushing them
	loggerT& operator<<(loggerEndl);

	// correctly handle the closing of required file
	void close();
};

// function which returns current parameter and the rest of the string (separated by space)
std::pair<std::string, std::string> getNextParam(std::string_view);

// recursively splits a string into vector by delimiter
std::vector<std::string_view> splitByDelim(std::string_view, std::string_view);

// returns a string of file permissions
// linux-like way
std::string getFilePerms (fs::path);

#endif //CPP_FTP_UTILS_H
