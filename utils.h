#include "utils.h"

loggerT::loggerT(std::string_view logFileName) {
	if (logFileName == "")
		logFile = std::make_unique<std::ofstream>(nullptr);
	else {
		logFile = std::make_unique<std::ofstream>(std::ofstream(logFileName.data()));
		std::cout << "Logging to file " << logFileName << std::endl;
	}
}

loggerT& loggerT::operator<<(loggerEndl) {
	std::cout << std::endl;
	if (logFile)
		*logFile << std::endl;
	return *this;
}

void loggerT::close() {
	if (logFile)
		logFile->close();
}

std::pair<std::string, std::string> getNextParam(std::string_view str) {
	auto pos = str.find(' ');
	if (pos == std::string::npos)
		return std::pair<std::string, std::string>{str, ""};
	return std::pair<std::string, std::string>{str.substr(0, pos), str.substr(pos + 1)};
}

std::vector<std::string_view> splitByDelim(std::string_view str, std::string_view delim) {
	if (str == "")
		return {};
	int32_t pos = str.find(delim);
	if (pos == std::string::npos)
		return {str};
	std::vector<std::string_view> tmp {str.substr(0, pos)};
	std::vector<std::string_view> append = splitByDelim(str.substr(pos + 1), delim);
	tmp.insert(tmp.end(), append.begin(), append.end());
	return tmp;
}

std::string getFilePerms (fs::path path) {
	const fs::file_status fileStat = fs::status(path);
	const fs::perms filePerms = fileStat.permissions();
	return std::string{} + (fs::is_directory(fileStat) ? "d" : "-") +
		   ((filePerms & fs::perms::owner_read) != fs::perms::none ? "r": "-") +
		   ((filePerms & fs::perms::owner_write) != fs::perms::none ? "w": "-") +
		   ((filePerms & fs::perms::owner_exec) != fs::perms::none ? "x": "-") +
		   ((filePerms & fs::perms::group_read) != fs::perms::none ? "r": "-") +
		   ((filePerms & fs::perms::group_write) != fs::perms::none ? "w": "-") +
		   ((filePerms & fs::perms::group_exec) != fs::perms::none ? "x": "-") +
		   ((filePerms & fs::perms::others_read) != fs::perms::none ? "r": "-") +
		   ((filePerms & fs::perms::others_write) != fs::perms::none ? "w": "-") +
		   ((filePerms & fs::perms::others_exec) != fs::perms::none ? "x": "-");
}
