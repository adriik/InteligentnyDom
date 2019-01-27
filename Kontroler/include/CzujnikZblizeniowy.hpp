#pragma once

#include <cstdio>
#include <ctime>
// In order to use ccRTP, the RTP stack of CommonC++, just include...
#include <ccrtp/rtp.h>
//#ifdef  CCXX_NAMESPACES
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

public:
	CzujnikZblizeniowy(unsigned char * statusCzujnikZblizeniowy);

	~CzujnikZblizeniowy();

	// This method does almost everything.
	void run(void);
};