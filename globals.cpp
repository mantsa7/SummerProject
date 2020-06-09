#include "globals.h"

netbuffer::netbuffer() { buffer.reserve(BUFSIZE); };

void clearBuffer(netbuffer &netbuff) {
	netbuff.buffer.clear();
	netbuff.buffer.reserve(BUFSIZE);
}

void readyBuffer(netbuffer &netbuff, dataT::iterator pos) {
	std::move_backward(pos + 2, netbuff.buffer.end(), netbuff.buffer.begin() + (netbuff.buffer.end() - pos - 2));
	netbuff.buffer.resize(netbuff.buffer.end() - pos - 2);
}

dataT::iterator findPair(dataT::iterator start, dataT::iterator end, std::pair<byte, byte> &toFind) {
	// if we reached the end (only one char left)
	if (start == end or start + 1 == end)
		return end;
	if (*start == toFind.first and *(start + 1) == toFind.second)
		return start;
	return findPair(start + 1, end, toFind);
}

dataT readline(sockpp::tcp_socket &socket, netbuffer &netbuff) {
	dataT::iterator ptrToCRLF;
	// while we can't find CRLF and while the buffer isn't full
	// if buffer is full then let's return a zero sized buffer, and cause a 500 error
	while ((ptrToCRLF = findPair(netbuff.buffer.begin(), netbuff.buffer.end(), const_cast<std::pair<byte, byte> &>(CRLFp))) == netbuff.buffer.end() and
		   netbuff.buffer.size() != netbuff.buffer.capacity()) {
		// number of bytes read
		int32_t readn = socket.read(netbuff.buffer.data() + netbuff.buffer.size(), netbuff.buffer.capacity() - netbuff.buffer.size());
		// we can't read anymore
		// connection either ended, reset or dropped
		// OR
		// some error happened, we can't read anymore, we should close
		if (readn <= 0) {
			return {'X','Q','U','I','T','N','O','W'};
		} else {
			// fix up the vector structure
			// resize vector so that the size is correct
			netbuff.buffer.assign(netbuff.buffer.data(), netbuff.buffer.data() + netbuff.buffer.size() + readn);
		}
	}
	// if the command is too long return empty buffer
	if (ptrToCRLF == netbuff.buffer.end()) {
		clearBuffer(netbuff);
		return {};
	}
	const dataT returnBuffer(netbuff.buffer.begin(), ptrToCRLF);
	readyBuffer(netbuff, ptrToCRLF);
	return returnBuffer;
}

dataT read(sockpp::tcp_socket &socket, netbuffer &netbuff) {
	// while the socket is open and while the buffer still has free space try to read
	while(socket and netbuff.buffer.size() != netbuff.buffer.capacity()) {
		int32_t readn = socket.read(netbuff.buffer.data() + netbuff.buffer.size(), netbuff.buffer.capacity() - netbuff.buffer.size());
		// oops, can't read anymore
		if (readn <= 0)
			break;
		// make the vector fix its structure and size for memory safe handling of data
		netbuff.buffer.assign(netbuff.buffer.data(), netbuff.buffer.data() + netbuff.buffer.size() + readn);
	}
	// copy to a new buffer the data and then clear the buffer
	// so that if we call again and sock is closed we get a correctly empty buffer
	const dataT returnVal(netbuff.buffer.begin(), netbuff.buffer.end());
	netbuff.buffer.clear();
	return returnVal;
}
