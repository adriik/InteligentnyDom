#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
#include "pugixml.hpp"
#include "parser.hpp"

using namespace ost;
using namespace std;

#define NAZWA_LOGU "Piec_log"
char buforDlaLogu[255];
xml_document doc;

bool czyscLog()
{
	FILE *plik = NULL;
	plik = fopen(NAZWA_LOGU, "wt");
	if (plik != NULL)
	{
		return true;
	}
	return false;
}

bool zapiszLog(char const *tekst)
{
	FILE *plik = NULL;
	plik = fopen(NAZWA_LOGU, "a");
	if (plik != NULL)
	{
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		fprintf(plik, "%02d/%02d/%0d %02d:%02d:%02d: %s\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tekst);
		fclose(plik);
		return true;
	}
	return false;
}

unsigned char status_piec = '0';
unsigned char *status = &status_piec;
unsigned char salute[50] = {
	'0',
};

/**
 * @class ccRTP_Hello_Rx
 * Receiver of salutes.
 */
class ccRTP_Hello_Rx : public Thread
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
	ccRTP_Hello_Rx(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
	{
		// Before using ccRTP you should learn something about other
		// CommonC++ classes. We need InetHostAddress...

		// Construct loopback address
		local_ip = localIP;
		destination_ip = destinationIp;
		RECEIVER_BASE = PortRx;
		TRANSMITTER_BASE = PortTx;

		// Is that correct?
		if (!local_ip)
		{
			// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Rx: Local IP address is not correct!" << endl;
			exit();
		}
		// Is that correct?
		if (!destination_ip)
		{
			// this is equivalent to `! destination_ip.isInetAddress()'
			cerr << "Rx: Destination IP address is not correct!" << endl;
			exit();
		}

		// create socket for RTP connection and get a random
		// SSRC identifier
		socket = new RTPSession(local_ip, RECEIVER_BASE);
		ssrc = socket->getLocalSSRC();
	}

	~ccRTP_Hello_Rx()
	{
		cout << endl
			 << "Destroying receiver -ID: " << hex
			 << (int)ssrc;
		terminate();
		delete socket;
		cout << "... "
			 << "destroyed.";
	}

	// This method does almost everything.
	void run(void)
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

		cout << "Rx (" << hex << (int)ssrc
			 << "): The queue is "
			 << (socket->isActive() ? "" : "in")
			 << "active." << endl;

		unsigned char tmp = '0';
		// This is the main loop, where packets are received.
		for (int i = 0; true; i++)
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

			*status = ((unsigned char *)(adu->getData()))[0];
			salute[0] = *status;
			unsigned char actualState = ((unsigned char *)(adu->getData()))[0];
			if (actualState != tmp && actualState == '0')
			{
				cout << "Piec nie grzeje" << endl;
			}
			else if (actualState != tmp && actualState == '1')
			{
				cout << "Piec grzeje" << endl;
			}
			tmp = actualState;
			delete adu;
		}
	}
};

class ccRTP_Hello_Tx : public Thread, public TimerPort
{

  private:
	// socket to transmit
	RTPSession *socket;
	// loopback network address
	InetHostAddress local_ip;
	InetHostAddress destination_ip;
	// identifier of this sender
	uint32 ssrc;
	int RECEIVER_BASE_CT;
	int TRANSMITTER_BASE_CT;

  public:
	ccRTP_Hello_Tx(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
	{

		// Construct loopback address
		local_ip = localIP;
		destination_ip = destinationIp;
		RECEIVER_BASE_CT = PortRx;
		TRANSMITTER_BASE_CT = PortTx;

		// Is that correct?
		if (!local_ip)
		{
			// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Tx: Local IP address is not correct!" << endl;
			exit();
		}
		// Is that correct?
		if (!destination_ip)
		{
			// this is equivalent to `! destination_ip.isInetAddress()'
			cerr << "Tx: Destination IP address is not correct!" << endl;
			exit();
		}

		socket = new RTPSession(local_ip, TRANSMITTER_BASE_CT);
		ssrc = socket->getLocalSSRC();
	}

	~ccRTP_Hello_Tx()
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
	void run(void)
	{

		// Set up connection
		socket->setSchedulingTimeout(20000);
		socket->setExpireTimeout(3000000);
		if (!socket->addDestination(destination_ip, RECEIVER_BASE_CT))
			cerr << "Tx (" << hex << (int)ssrc
				 << "): could not connect to port."
				 << dec << RECEIVER_BASE_CT;

		uint32 timestamp = 0;
		// This will be useful for periodic execution
		TimerPort::setTimer(5000);

		socket->setPayloadFormat(StaticPayloadFormat(sptMP2T));
		socket->startRunning();

		cout << "Tx (" << hex << (int)ssrc << "): The queue is "
			 << (socket->isActive() ? "" : "in")
			 << "active." << endl;

		for (;;)
		{

			time_t sending_time = time(NULL);

			timestamp = socket->getCurrentTimestamp();

			socket->putData(timestamp, salute,
							strlen((char *)salute) + 1);
			char tmstring[30];
			strftime(tmstring, 30, "%X",
					 localtime(&sending_time));

			// Let's wait for the next cycle
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(5000);
		}
	}
};

int main()
{
	doc.load_file("./config.xml");

	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);

	// Construct the two main threads. they will not run yet.
	ccRTP_Hello_Rx *receiver = new ccRTP_Hello_Rx(
		doc.child("Dom").child("Piec").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Piec").child("PortRx").attribute("Wartosc").as_int(),
		doc.child("Dom").child("Piec").child("PortTx").attribute("Wartosc").as_int());

	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx(
		doc.child("Dom").child("Piec").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("CzujnikTemperatury").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Piec").child("PortRxCT").attribute("Wartosc").as_int(),
		doc.child("Dom").child("Piec").child("PortTxCT").attribute("Wartosc").as_int());

	// Start execution of hello now.
	receiver->start();
	transmitter->start();

	cin.get();

	delete receiver;

	cout << endl
		 << "That's all." << endl;

	return 0;
}