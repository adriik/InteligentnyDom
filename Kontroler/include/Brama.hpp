#pragma once

#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
using namespace ost;
using namespace std;

class Brama: public Thread, public TimerPort
{

private:
	// socket to transmit
	RTPSession *socket;
	// loopback network address
	InetHostAddress local_ip;
	InetHostAddress destination_ip;
	// identifier of this sender
	uint32 ssrc;

	int RECEIVER_BASE;
	int TRANSMITTER_BASE;

public:
	unsigned char salute2[50]={'0'};
	unsigned char statusBramy;
	Brama(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx);

	~Brama();

	// This method does almost everything.
	void run(void);

};