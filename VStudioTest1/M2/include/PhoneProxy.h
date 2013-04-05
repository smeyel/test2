#ifndef __PHONEPROXY_H
#define __PHONEPROXY_H
#include <winsock.h>

class PhoneProxy
{
private:
	SOCKET sock;

public:
	void Connect(char *ip, int port);
	void Disconnect();

	void RequestPhoto(int desiredTimeStamp);
	void RequestPing();
	void Receive(char *filename);	// For PONG, filename has no effect.
	void ReceiveDebug();

private:
	void ProcessIncomingJSON(int sock,char *buffer, char *filename);
};

#endif