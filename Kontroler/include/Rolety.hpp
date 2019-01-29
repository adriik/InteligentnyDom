#pragma once

#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
using namespace ost;
using namespace std;

class Rolety: public Thread, public TimerPort
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
	unsigned char salute[50]={'0'};
	unsigned char statusRolety;
	Rolety(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx);

	~Rolety();

	// This method does almost everything.
	void run(void);

};