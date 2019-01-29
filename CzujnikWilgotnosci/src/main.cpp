#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
#include "pugixml.hpp"
#include "parser.hpp"

using namespace ost;
using namespace std;

#define NAZWA_LOGU "CzujnikWilgotnosci_log"
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

unsigned char salute[50] = {'0'};
unsigned char status_zraszacz = '0';
unsigned char *status_zraszacz_ptr = &status_zraszacz;

/**
 * @class ccRTP_Hello_Tx
 * Transmitter of salutes.
 */
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
	int RECEIVER_BASE;
	int TRANSMITTER_BASE;

  public:
	ccRTP_Hello_Tx(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
	{

		// Construct loopback address
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
		// Is that correct?
		if (!destination_ip)
		{
			// this is equivalent to `! destination_ip.isInetAddress()'
			cerr << "Tx: Destination IP address is not correct!" << endl;
			exit();
		}

		socket = new RTPSession(local_ip, TRANSMITTER_BASE);
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
		if (!socket->addDestination(destination_ip, RECEIVER_BASE))
			cerr << "Tx (" << hex << (int)ssrc
				 << "): could not connect to port."
				 << dec << RECEIVER_BASE;

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
	int RECEIVER_BASE_ZRASZACZ;
	int TRANSMITTER_BASE_ZRASZACZ;

  public:
	ccRTP_Hello_Rx(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
	{
		// Before using ccRTP you should learn something about other
		// CommonC++ classes. We need InetHostAddress...

		// Construct loopback address
		local_ip = localIP;
		destination_ip = destinationIp;
		RECEIVER_BASE_ZRASZACZ = PortRx;
		TRANSMITTER_BASE_ZRASZACZ = PortTx;

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
		socket = new RTPSession(local_ip, RECEIVER_BASE_ZRASZACZ);
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
		//socket->UDPTransmit::setTypeOfService(SOCKET_IPTOS_LOWDELAY);
		if (!socket->addDestination(destination_ip, TRANSMITTER_BASE_ZRASZACZ))
			cerr << "Rx (" << hex << (int)ssrc
				 << "): could not connect to port."
				 << dec << TRANSMITTER_BASE_ZRASZACZ;

		cout << "Rx (" << hex << (int)ssrc
			 << "): " << local_ip.getHostname()
			 << " is waiting for salutes in port "
			 << dec << RECEIVER_BASE_ZRASZACZ << "..." << endl;

		socket->setPayloadFormat(StaticPayloadFormat(sptMP2T));
		socket->startRunning();

		cout << "Rx (" << hex << (int)ssrc
			 << "): The queue is "
			 << (socket->isActive() ? "" : "in")
			 << "active." << endl;

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
			*status_zraszacz_ptr = ((unsigned char *)(adu->getData()))[0];
			delete adu;
		}
	}
};

int main()
{
	doc.load_file("./config.xml");

	int wilgotnosc_min = doc.child("Dom").child("CzujnikWilgotnosci").child("ProgUruchomieniaZraszacza").attribute("Wartosc").as_int();
	int zwiekszWilgO = doc.child("Dom").child("CzujnikWilgotnosci").child("WartoscZwiekszeniaWilgotnosci").attribute("Wartosc").as_int();
	int wilgotnosc_aktualna = doc.child("Dom").child("CzujnikWilgotnosci").child("ZadanaWilgotnosc").attribute("Wartosc").as_int();
	int maxWilg = doc.child("Dom").child("CzujnikWilgotnosci").child("ProgZatrzymaniaZraszania").attribute("Wartosc").as_int();
	int zmniejszWilgO = doc.child("Dom").child("CzujnikWilgotnosci").child("WartoscSpadaniaWilgotnosci").attribute("Wartosc").as_int();
	//int krokZmiany = doc.child("Dom").child("CzujnikWilgotnosci").child("KrokZmianyWilgotnosci").attribute("Wartosc").as_int();
	//krok zmiany nie wykorzystany, bo poslugujemy sie sleepem na 5 lub 6sekund

	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);

	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx(
		doc.child("Dom").child("CzujnikWilgotnosci").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("CzujnikWilgotnosci").child("PortRx").attribute("Wartosc").as_int(),
		doc.child("Dom").child("CzujnikWilgotnosci").child("PortTx").attribute("Wartosc").as_int());

	ccRTP_Hello_Rx *receiver = new ccRTP_Hello_Rx(
		doc.child("Dom").child("CzujnikWilgotnosci").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Zraszacz").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("CzujnikWilgotnosci").child("PortRxZraszacz").attribute("Wartosc").as_int(),
		doc.child("Dom").child("CzujnikWilgotnosci").child("PortTxZraszacz").attribute("Wartosc").as_int());
	transmitter->start();
	receiver->start();
	cout << "Czujnik wilgotnosci: " << endl;
	cout << "" << endl;
	cout << "" << endl;

	for (;;)
	{

		if (status_zraszacz != '0')
		{
			wilgotnosc_aktualna += zwiekszWilgO;
		}
		else
		{

			wilgotnosc_aktualna -= zmniejszWilgO;
		}

		memcpy(&salute[1], (char *)&wilgotnosc_aktualna, sizeof(int));
		cout << "wilgotnosc aktualna: " << wilgotnosc_aktualna << endl;

		if (wilgotnosc_aktualna < wilgotnosc_min)
		{
			salute[0] = '0';
		}
		else if (wilgotnosc_aktualna > maxWilg)
		{
			salute[0] = '1';
		}

		sleep(5);
	}

	return 0;
}