#pragma once

#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>

using namespace ost;
using namespace std;

class CzujnikZblizeniowy: public Thread
{

private:
	// socket to receive packets
	RTPSession *socket;
	// loopback network address
	InetHostAddress local_ip;
	InetHostAddress destination_ip;
	// identifier of this sender
	uint32 ssrc;

	int RECEIVER_BASE;
	int TRANSMITTER_BASE;

public:
	unsigned char *statusCzujnikZblizeniowy;

	CzujnikZblizeniowy(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx);

	~CzujnikZblizeniowy();

	// This method does almost everything.
	void run(void);
};