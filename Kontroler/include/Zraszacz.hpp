#pragma once

#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
using namespace ost;
using namespace std;

class Zraszacz: public Thread, public TimerPort
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
	unsigned char statusZraszacza;
	unsigned char salute4[50]={'0'};
	Zraszacz(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx);

	~Zraszacz();

	// This method does almost everything.
	void run(void);

};