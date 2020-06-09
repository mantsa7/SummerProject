#ifndef CPP_FTP_FTPTRANSFER_H
#define CPP_FTP_FTPTRANSFER_H

#include <sockpp/tcp_socket.h>
#include "globals.h"

class streamTransferWriter {
public:
	dataT buffer;
	streamTransferWriter();

	// write remaining data to socket
	bool flush(sockpp::stream_socket &sock);

	// lazily write data to socket
	bool write(sockpp::stream_socket &sock, dataT data);
};

#endif //CPP_FTP_FTPTRANSFER_H
