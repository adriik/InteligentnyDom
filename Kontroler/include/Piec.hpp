#pragma once

#include <cstdio>
#include <ctime>

#include <ccrtp/rtp.h>
using namespace ost;
using namespace std;

class Piec: public Thread, public TimerPort
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
	unsigned char statusPieca;
	unsigned char salute3[50]={'0'};
	Piec(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx);

	~Piec();

	// This method does almost everything.
	void run(void);

};