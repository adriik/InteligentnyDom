#include "CzujnikTemperatury.hpp"
#include "Piec.hpp"
#include "parser.hpp"
int temperatura = 25;
CzujnikTemperatury::CzujnikTemperatury(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
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

	// create socket for RTP connection and get a random
	// SSRC identifier
	socket = new RTPSession(local_ip, RECEIVER_BASE);
	ssrc = socket->getLocalSSRC();
}

CzujnikTemperatury::~CzujnikTemperatury()
{
	cout << endl
		 << "Destroying receiver -ID: " << hex
		 << (int)ssrc;
	terminate();
	delete socket;
	cout << "... "
		 << "destroyed.";
}
void CzujnikTemperatury::run(void)
{

	socket->setSchedulingTimeout(20000);
	socket->setExpireTimeout(3000000);

	if (!socket->addDestination(destination_ip, TRANSMITTER_BASE))
		cerr << "Rx (" << hex << (int)ssrc
			 << "): could not connect to port."
			 << dec << TRANSMITTER_BASE;

	cout << "Rx (" << hex << (int)ssrc
		 << "): " << local_ip.getHostname()
		 << " is waiting for salutes in port "
		 << dec << RECEIVER_BASE << "..." << endl;

	socket->setPayloadFormat(StaticPayloadFormat(sptMP2T));
	socket->startRunning();
	// Let's check the queues  (you should read the documentation
	// so that you know what the queues are for).
	Piec *piec = new Piec(
		doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Piec").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Piec").child("PortRx").attribute("Wartosc").as_int(),
		doc.child("Dom").child("Piec").child("PortTx").attribute("Wartosc").as_int());

	// This is the main loop, where packets are received.
	for (;;)
	{

		// Wait for an RTP packet.
		const AppDataUnit *adu = NULL;
		while (NULL == adu)
		{
			Thread::sleep(10);
			adu = socket->getData(socket->getFirstTimestamp());
		}
		// Print content (likely a salute :))
		// Note we are sure the data is an asciiz string.
		time_t receiving_time = time(NULL);
		char tmstring[30];
		strftime(tmstring, 30, "%X", localtime(&receiving_time));

		unsigned char tmp = *statusCzujnikTemperatury;
		unsigned char *tmp2;
		tmp2 = (unsigned char *)&temperatura;
		*statusCzujnikTemperatury = ((unsigned char *)(adu->getData()))[0];
		*tmp2++ = ((unsigned char *)(adu->getData()))[1];

		*tmp2++ = ((unsigned char *)(adu->getData()))[2];

		*tmp2++ = ((unsigned char *)(adu->getData()))[3];

		*tmp2++ = ((unsigned char *)(adu->getData()))[4];

		if (*statusCzujnikTemperatury != tmp)
		{

			piec->statusPieca = tmp;
			piec->salute3[0] = tmp;
			piec->start();
		}
		delete adu;
	}
};