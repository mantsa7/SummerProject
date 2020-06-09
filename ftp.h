#ifndef CPP_FTP_FTP_HPP
#define CPP_FTP_FTP_HPP

#include <sockpp/tcp_socket.h>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_connector.h>
#include <sockpp/inet_address.h>
#include <algorithm>
#include <fstream>
#include "globals.h"
#include "utils.h"
#include "ftptransfer.h"

// ftp structure for holding the connections and the state of the ftp control connection
class FTP {
public:
	// control connection and data connection as specified in rfc 959
	sockpp::tcp_socket controlSock;
	// we need to store the passive socket separately cause we need to listen and accept on it multiple times
	// after we accepted we just write the socket to dataSocket
	sockpp::tcp_acceptor pasvSock;
	// if we are using an active data connection (server connects to client)
	// then we use tcp_connector and then store the socket here
	sockpp::tcp_socket dataSocket;
	// we need to store the control connection peer for logging
	// and data socket peer for error logging
	sockpp::inet_address peer, dataSockAddr;
	// logger class
	// if we don't have logging to file enabled then we don't need to worry
	// because logger handles it on its own
	loggerT &logger;
	// server root directory path and current path
	fs::path serverRoot, workDir, curDir;
	// we store the valid users in this map so we can authenticate easily
	stringHashMap &users;
	// the buffer of the ftp control socket
	netbuffer ftpBuf;

	// set active to false and the server quits
	bool passiveMode = false, active = true;
	// store the user here for auth check
	std::pair<std::string, std::string> user {};
	// needed to check previous received command to validate the order
	std::string prevCommand {};

	// ftp data transfer formatting
	// we only support ascii-nonprint and image(binary), everything else is obsolete
	enum FMTTYPE {ASCII_N, IMAGE} ftpFormatType = ASCII_N;
	// we don't support compressed mode or block mode
	enum FMTMODE {STREAM} ftpFormatMode = STREAM;
	// accept only file structure
	enum FTPSTRU {FILE} ftpFormatStru = FILE;


	// we use std::move to move unique_ptr type variables that can't be copied
	FTP(stringHashMap &, sockpp::tcp_socket, sockpp::inet_address, fs::path, loggerT &);
};

// helper function to get the peer of ftp connection
std::string getPeer(FTP &);

// helper function which sends an error string and writes it to the logger
bool shutdownError(FTP&, std::string_view);

// helper function for sending simple c++ string replies
bool sendString(FTP&, std::string_view);

// helper function for sending simple replies
bool sendReply(FTP&, uint32_t, std::string_view);

// function to setup the data connection
std::tuple<bool, int32_t, std::string> initDataConnection(FTP &);

// helper function to validate path
// tries to get the canonical path and then the absolute path
// and then checks if the path starts with the serverRoot path
// this is secure, we can't go out of our secure directory
std::pair<fs::path, bool> getPath(FTP &, std::string);

// helper function to check if user is logged in
// we don't support anonymous FTP connection without a password
// because it isn't a necessity
bool isAuthed(FTP&);

// ftp noop
// doesn't do anything
response noopFTP(FTP &, std::string_view);

// ftp help
// sends multiline reply of available commands and help message
response helpFTP(FTP &, std::string_view);

// function to handle USER
// USER [username] tries to begin authentication with the specified username
// if the username is invalid then the process must start again
// susceptible to username enumeration through bruteforce
response userFTP(FTP &, std::string_view);

// function to handle PASS
// PASS [password] tries to authenticate the user after the username has been specified with USER command
// if incorrect then the process must start again
// no bruteforce protection because that shouldn't be the worries of the server
response passFTP(FTP &, std::string_view);

// function to handle REIN
// REIN logs out the user, allowing a new user to login on the same control connection
response reinFTP(FTP &, std::string_view);

// handle FTP quit
// QUIT just stops the control connection
response quitFTP(FTP &, std::string_view);

// handle FTP pwd
// we create a fake filesystem where we are in /$workdir and can't go up
// PWD prints the current directory (starting from the server root)
response pwdFTP(FTP &, std::string_view);

// handle FTP type
// ASCII and binary format are pretty much indifferent nowadays
// ASCII used to support different newline sequences but nowadays it doesn't matter
// so we don't need to convert CRLF to LF and vice-versa
response typeFTP(FTP &, std::string_view);

// handle FTP mode
// we only support stream mode
// MODE [MODE]
response modeFTP(FTP &, std::string_view);

// handle FTP structure
// we don't support anything other than file
response struFTP(FTP &, std::string_view);

// handle FTP pasv
// PASV requests passive data connection
// the return is a code and the connection in the form ip1, ip2, ip3, ip4, port1, port2
// which the client transforms into ip1.ip2.ip3.ip4:port1*256+port
// the client must connect to the specified connection for data transfer commands
// YOU SHOULDN'T rely on the ip1.ip2.ip3.ip4 for connections and should use the server's actual IP
// because the server listens on ALL network interfaces
response pasvFTP(FTP &, std::string_view);

// handle FTP port
// PORT ip1, ip2, ip3, ip4, port1, port2
// specifies the active connection address as ip1.ip2.ip3.ip4:port1*256+port
// as specified in RFC 959
// the client must listen on this address for data connections
response portFTP(FTP &, std::string_view);

// handle FTP cwd
// CWD [PATH] tries to change the working directory to PATH
// we don't actually cd anywhere, we just change the curDir variable in the class
response cwdFTP(FTP &, std::string_view);

// handle FTP cdup, just call cwd with .. parameter
// CDUP goes up one directory, but it's basically cwd ..
response cdupFTP(FTP &, std::string_view);

// handle FTP mkd
// MKD [PATH] tries to create the directories in PATH
response mkdFTP(FTP &, std::string_view);

// handle FTP SYST
// let's just fake our server type and always say we are on linux
response systFTP(FTP &, std::string_view);

// handle FTP LIST
// LIST [PATH/-a]
// LIST -a/-al/-la prints "verbose" output with . and ..
// default LIST sends the directory listing to the data connection
response listFTP(FTP &, std::string_view);

// handle FTP STOR
// STOR [PATH] tries to write the file to path
// only writes if we have access to this path and if the path points to a file in an existing folder
response storFTP(FTP &, std::string_view);

// handle FTP RETR
// RETR [PATH] tries to retrieve requested file
response retrFTP(FTP &, std::string_view);

#endif //CPP_FTP_FTP_HPP
