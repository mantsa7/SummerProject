#include "ftptransfer.h"

streamTransferWriter::streamTransferWriter() {
	buffer.reserve(BUFSIZE);
}

bool streamTransferWriter::flush(sockpp::stream_socket &sock) {
	// error happened
	if (sock.write_n(buffer.data(), buffer.size()) < buffer.size())
		return true;
	return false;
}

bool streamTransferWriter::write(sockpp::stream_socket &sock, dataT data) {
	// if we don't have enough space then flush
	int32_t leftspace = buffer.capacity() - buffer.size();
	if (leftspace < data.size()) {
		buffer.insert(buffer.end(), data.begin(), data.begin() + leftspace);
		if (flush(sock))
			return true;
		buffer.clear();
		buffer.insert(buffer.end(), data.begin() + leftspace, data.end());
		return false;
	}
	buffer.insert(buffer.end(), data.begin(), data.end());
	return false;
}