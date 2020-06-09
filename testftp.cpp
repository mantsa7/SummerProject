#include <sockpp/tcp_connector.h>
#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp_socket.h>
#include "globals.h"
#include "utils.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>
using std::cout;
using std::endl;

void failed() {
	cout << "TESTS FAILED" << endl;
	exit(-1);
}

const bool sendString(sockpp::tcp_socket &sock, std::string str) {
	if (sock.write_n(str.data(), str.size()) < str.size())
		return true;
	return false;
}

const std::string readLine(sockpp::tcp_socket &sock, netbuffer &netbuff) {
	dataT recvd = readline(sock, netbuff);
	return std::string(recvd.begin(), recvd.end());
}

void tryt(bool res, std::string error) {
	if (res) {
		cout << "ERROR - " << error << endl;
		failed();
	}
}

int main(int, char**) {
	const std::string customDir = "mydir";
	const std::string logfileName = "tmplog.txt";
	const in_port_t customPort = 61976;
	const std::string launchCmd = "./cpp_ftp -d " + customDir + " -l " + logfileName + " -p " + std::to_string(customPort);
	int child;
	if ((child = fork()) == 0) {
		// close the standard fd's for child process
		close(0);
		close(1);
		close(2);
		system(launchCmd.c_str());
		return 0;
	}
	sleep(2);
	cout << "Launched ftp server with command \"" << launchCmd << "\"" << endl;
	{
		std::ofstream userfile("users.txt");
		userfile << "admin1:pass1\nadmin2:pass2" << endl;
		userfile.close();
	}
	cout << "Written test usernames and passwords to \"users.txt\"" << endl;
	cout << "Test 1. Connection and authentication with various users" << endl;
	cout << "\tTrying to connect..." << endl;
	sockpp::inet_address serverAddr("127.0.0.1", customPort);
	sockpp::tcp_connector mainConn(serverAddr);
	if (not mainConn) {
		cout << "ERROR can't connect to address " << serverAddr.to_string() << endl;
		failed();
	}
	cout << "\tConnected, testing authorization" << endl;
	sockpp::tcp_socket mainConnSock = std::move(mainConn);
	netbuffer netbuff;
	tryt(not readLine(mainConnSock, netbuff).starts_with("220"), "server welcome reply code isn't 220");
	tryt(sendString(mainConnSock, "user admin1"+CRLF), "couldn't send user command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("331"), " failed user command with admin1");
	tryt(sendString(mainConnSock, "pass pass1"+CRLF), "couldn't send pass command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("230"), "failed to authorize with creds admin1:pass1");
	cout << "\tSuccessfully authorized as user admin1:pass1!" << endl;
	tryt(sendString(mainConnSock, "rein"+CRLF), "couldn't send pass command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("220"), "couldn't log out of user admin1");
	tryt(sendString(mainConnSock, "user admin2"+CRLF), "couldn't send user command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("331"), " failed user command with admin2");
	tryt(sendString(mainConnSock, "pass pass2"+CRLF), "couldn't send pass command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("230"), "failed to authorize with creds admin2:pass2");
	cout << "\tSuccessfully authorized as user admin2:pass2!" << endl;
	tryt(sendString(mainConnSock, "pasv"+CRLF), "couldn't send pasv command");
	std::string response = readLine(mainConnSock, netbuff);
	const auto [code, leftover] = getNextParam(response);
	const auto [address, tmp1] = getNextParam(leftover);
	tryt(not (code == "227"), "couldn't setup passive connection on server");
	auto tokens = splitByDelim(address, ",");
	tryt(tokens.size() != 6, "invalid address returned by pasv");
	sockpp::inet_address passiveDataAddr("127.0.0.1", std::stoi(tokens[4].data())*256+std::stoi(tokens[5].data()));
	cout << "\tPassive connection address is " << passiveDataAddr.to_string() << endl;
	cout << "\\Test 1 Passed/" << endl;
	cout << "Test 2. Directory creation and path manipulation" << endl;
	tryt(sendString(mainConnSock, "pwd"+CRLF), "couldn't send pwd command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("257 /"+customDir), "wrong start directory or answer from pwd");
	cout << "\tCorrect pwd returned /"+customDir << endl;
	tryt(sendString(mainConnSock, "mkd tmp1"+CRLF), "couldn't send \"mkd tmp1\" command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("200"), "can't create directory tmp1");
	tryt(sendString(mainConnSock, "mkd tmp2"+CRLF), "couldn't send \"mkd tmp2\" command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("200"), "can't create directory tmp2");
	tryt(sendString(mainConnSock, "mkd tmp1/temporary"+CRLF), "couldn't send \"mkd tmp1/temporary\" command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("200"), "can't create directory tmp1/temporary");
	cout << "\tDirectory creation seems successful, trying dir listing" << endl;
	tryt(sendString(mainConnSock, "list"+CRLF), "couldn't send \"mkd tmp1/temporary\" command");
	sockpp::tcp_connector dataConn(passiveDataAddr);
	tryt(not dataConn, "couldn't connect to passive data connection");
	cout << "\tConnection to data socket successful" << endl;
	sockpp::tcp_socket dataConnSock = std::move(dataConn);
	std::vector<std::string> replies(3);
	tryt(not readLine(mainConnSock, netbuff).starts_with("125"), "didn't get confirmation from server about directory listing");
	tryt(not readLine(mainConnSock, netbuff).starts_with("226"), "invalid reply from directory list command");
	netbuffer datanetbuff;
	replies[0] = readLine(dataConnSock, datanetbuff);
	replies[1] = readLine(dataConnSock, datanetbuff);
	replies[2] = replies[0] + replies[1];
	tryt(replies[2].find("tmp1") == std::string::npos, "\"tmp1\" directory not in dir listing");
	tryt(replies[2].find("tmp2") == std::string::npos, "\"tmp2\" directory not in dir listing");
	cout << "\tDirectory listing of main directory successful" << endl;
	tryt(sendString(mainConnSock, "cwd tmp1"+CRLF), "couldn't send directory change command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("200"), "couldn't change current directory to \"tmp1\"");
	replies.clear();
	replies.resize(1);
	cout << "\tChanged dir to tmp1, trying to list its contents" << endl;
	dataConn = sockpp::tcp_connector(passiveDataAddr);
	tryt(not dataConn, "couldn't connect to passive data connection");
	dataConnSock = std::move(dataConn);
	tryt(sendString(mainConnSock, "list"+CRLF), "couldn't send \"mkd tmp1/temporary\" command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("125"), "didn't get confirmation from server about directory listing");
	tryt(not readLine(mainConnSock, netbuff).starts_with("226"), "invalid reply from directory list command");
	replies[0] = readLine(dataConnSock, datanetbuff);
	tryt(replies[0].find("temporary") == std::string::npos, "\"temporary\" directory not in dir listing of \"tmp1\"");
	cout << "\tDir listing of tmp1 is good" << endl;
	tryt(sendString(mainConnSock, "cdup"+CRLF), "couldn't send cdup command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("200"), "couldn't go up to main directory");
	tryt(sendString(mainConnSock, "cdup"+CRLF), "couldn't send cdup command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("550"), "directory permissions incorrect, can escape main dir");
	cout << "\tDirectory permission test correct, can't get out of main directory" << endl;
	cout << "\\Test 2 Passed/" << endl;
	cout << "Test 3. Sending and retrieving files" << endl;
	tryt(sendString(mainConnSock, "cwd tmp2"+CRLF), "couldn't send directory change command");
	tryt(not readLine(mainConnSock, netbuff).starts_with("200"), "couldn't change current directory to \"tmp2\"");
	tryt(sendString(mainConnSock, "stor ../tmp2/tempfile"+CRLF), "couldn't send store file command");
	cout << "\tChanged dir to tmp2 and trying to store file with path\"../tmp2/tempfile\"" << endl;
	dataConn = sockpp::tcp_connector(passiveDataAddr);
	tryt(not dataConn, "couldn't connect to passive data connection");
	dataConnSock = std::move(dataConn);
	tryt(sendString(dataConnSock, "this is a test file hahaha hahaha\r\ntmp\r\n"), "couldn't send file contents to data socket");
	dataConnSock.shutdown();
	dataConn.close();
	tryt(not readLine(mainConnSock, netbuff).starts_with("125"), "didn't get confirmation from server about file storing");
	tryt(not readLine(mainConnSock, netbuff).starts_with("226"), "invalid reply from file store command");
	cout << "\tSeems like storing the file is successful, going to try to retrieve it with path \"tempfile\"" << endl;
	tryt(sendString(mainConnSock, "retr tempfile"+CRLF), "couldn't send send file retrieve command");
	dataConn = sockpp::tcp_connector(passiveDataAddr);
	tryt(not dataConn, "couldn't connect to passive data connection");
	dataConnSock = std::move(dataConn);
	datanetbuff.buffer.clear();
	tryt(not readLine(dataConnSock, datanetbuff).starts_with("this is a test file hahaha hahaha"), "first line of retrieved file is incorrect");
	tryt(not readLine(dataConnSock, datanetbuff).starts_with("tmp"), "second line of retrieved file is incorrect");
	tryt(not readLine(mainConnSock, netbuff).starts_with("125"), "didn't get confirmation from server about file retrieving");
	tryt(not readLine(mainConnSock, netbuff).starts_with("226"), "invalid reply from file retrieve command");
	tryt(sendString(mainConnSock, "XQUITNOW"+CRLF), "couldn't send server kill command");
	cout << "\tRetrieving the file \"tempfile\" returned the same file, file storing is correct" << endl;
	cout << "\\Test 3 Passed/" << endl;
	cout << "ALL TESTS WERE SUCCESSFUL" << endl;
	cout << "CHECK DIRECTORY \"" + customDir + "\" AND LOG FILE \"" + logfileName + "\" TO CONFIRM THE RESULTS\"" << endl;
	kill(child, SIGKILL);
	return 0;
}
