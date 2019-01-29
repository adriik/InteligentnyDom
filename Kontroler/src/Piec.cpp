#include "Piec.hpp"

Piec::Piec(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
{

	local_ip = localIP;
	destination_ip = destinationIp;
	RECEIVER_BASE = PortRx;
	TRANSMITTER_BASE = PortTx;

	// Is that correct?
	if (!local_ip)
	{
		// this is equivalent to `! local_ip.isInetAddress()'
		cerr << "Tx: Local IP address is not correct!" << endl;
		exit();
	}
	if (!destination_ip)
	{
		// this is equivalent to `! destination_ip.isInetAddress()'
		cerr << "Tx: Destination IP address is not correct!" << endl;
		exit();
	}

	socket = new RTPSession(local_ip, TRANSMITTER_BASE);
	ssrc = socket->getLocalSSRC();
}

Piec::~Piec()
{
	cout << endl
		 << "Destroying transmitter -ID: " << hex
		 << (int)ssrc;
	terminate();
	delete socket;
	cout << "... "
		 << "destroyed.";
}

// This method does almost everything.
void Piec::run(void)
{
	// Set up connection
	socket->setSchedulingTimeout(20000);
	socket->setExpireTimeout(3000000);
	if (!socket->addDestination(destination_ip, RECEIVER_BASE))
		cerr << "Tx (" << hex << (int)ssrc
			 << "): could not connect to port."
			 << dec << RECEIVER_BASE;

	uint32 timestamp = 0;
	// This will be useful for periodic execution
	TimerPort::setTimer(5000);

	socket->setPayloadFormat(StaticPayloadFormat(sptMP2T));
	socket->startRunning();

	time_t sending_time = time(NULL);

	timestamp = socket->getCurrentTimestamp();

	socket->putData(timestamp, salute3,
					strlen((char *)salute3) + 1);

	char tmstring[30];
	strftime(tmstring, 30, "%X",
			 localtime(&sending_time));

	exit();
}