#ifndef CPP_FTP_GLOBALS_HPP
#define CPP_FTP_GLOBALS_HPP

#include <sockpp/socket.h>
#include <sockpp/tcp_socket.h>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

// server version
inline const std::string serverVersion("v0.1");
// default listen port for server
constexpr in_port_t defaultPort = 2020;
// telnet end-of-line
inline const std::string CRLF = "\r\n";
constexpr std::pair<unsigned char, unsigned char> CRLFp = {'\r', '\n'};
// list of users and passwords
inline const std::string defaultUserFile = "users.txt";
// the working directory for logged in users
inline const std::string defaultWorkdir = "myftpserver";
// the default size of a buffer
// large so that the reads are fast
inline const uint32_t BUFSIZE = (1 << 16);
// data type for sending bytes
typedef std::vector<unsigned char> dataT;
// ftp LIST -a . and ..
inline const std::string listVerbose = "drwxr-xr-x 0b ."+CRLF+"drwxr-xr-x 0b .."+CRLF;
inline const dataT listVerboseData(listVerbose.begin(), listVerbose.end());
// hashmap type for user:pass
typedef std::unordered_map<std::string, std::string> stringHashMap;
// byte type
typedef unsigned char byte;
// type of return string from command
typedef std::pair<int, std::string> response;

// class for reading from sockpp line-by-line
// we simply read into the buffer chunk-by-chunk
// if we encounter CRLF in the buffer then return up until CRLF
// and move the rest of the contents to the beginning
// this way we can get large amounts of data and don't have to call socket read char-by-char
// also we can use the same class for simply reading into the buffer until the connection is closed
struct netbuffer {
	dataT buffer;
	netbuffer();
};

// function for cleaning up the buffer
void clearBuffer(netbuffer &);

// function for moving leftovers beginning at pos to the beginning of the buffer (-2 to account for CRLF)
void readyBuffer(netbuffer &, dataT::iterator);

// function to find a pair of bytes in dataT
// for finding CRLF
dataT::iterator findPair(dataT::iterator, dataT::iterator, std::pair<byte, byte> &);

// read CRLF line from net buffer and return the line read
dataT readline(sockpp::tcp_socket &, netbuffer &);

// function for simply reading the full buffer if we can and returning it
// if some error happened then simply return zero size buffer
dataT read(sockpp::tcp_socket &, netbuffer &);

#endif //CPP_FTP_GLOBALS_HPP
