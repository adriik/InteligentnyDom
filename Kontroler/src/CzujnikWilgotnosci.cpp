#include "CzujnikWilgotnosci.hpp"
#include "Zraszacz.hpp"
#include "parser.hpp"
int wilgotnosc = 25;
CzujnikWilgotnosci::CzujnikWilgotnosci(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
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

CzujnikWilgotnosci::~CzujnikWilgotnosci()
{
	cout << endl
		 << "Destroying receiver -ID: " << hex
		 << (int)ssrc;
	terminate();
	delete socket;
	cout << "... "
		 << "destroyed.";
}
void CzujnikWilgotnosci::run(void)
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
	Zraszacz *zraszacz = new Zraszacz(
		doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Zraszacz").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Zraszacz").child("PortRx").attribute("Wartosc").as_int(),
		doc.child("Dom").child("Zraszacz").child("PortTx").attribute("Wartosc").as_int());

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

		unsigned char tmp = *statusCzujnikWilgotnosci;
		unsigned char *tmp2;
		tmp2 = (unsigned char *)&wilgotnosc;
		*statusCzujnikWilgotnosci = ((unsigned char *)(adu->getData()))[0];
		*tmp2++ = ((unsigned char *)(adu->getData()))[1];

		*tmp2++ = ((unsigned char *)(adu->getData()))[2];

		*tmp2++ = ((unsigned char *)(adu->getData()))[3];

		*tmp2++ = ((unsigned char *)(adu->getData()))[4];

		if (*statusCzujnikWilgotnosci != tmp)
		{
			zraszacz->statusZraszacza = tmp;
			zraszacz->salute4[0] = tmp;
			zraszacz->start();
		}
		delete adu;
	}
};