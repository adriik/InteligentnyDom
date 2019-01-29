#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
#include "pugixml.hpp"
#include "parser.hpp"

using namespace ost;
using namespace std;

#define NAZWA_LOGU "CzujnikTemperatury_log"
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
unsigned char status_piec = '0';
unsigned char *status_piec_ptr = &status_piec;

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
	int RECEIVER_BASE_PIEC;
	int TRANSMITTER_BASE_PIEC;

  public:
	ccRTP_Hello_Rx(InetHostAddress localIP, InetHostAddress destinationIp, int PortRx, int PortTx)
	{
		// Before using ccRTP you should learn something about other
		// CommonC++ classes. We need InetHostAddress...

		// Construct loopback address
		local_ip = localIP;
		destination_ip = destinationIp;
		RECEIVER_BASE_PIEC = PortRx;
		TRANSMITTER_BASE_PIEC = PortTx;

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
		socket = new RTPSession(local_ip, RECEIVER_BASE_PIEC);
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

		if (!socket->addDestination(destination_ip, TRANSMITTER_BASE_PIEC))
			cerr << "Rx (" << hex << (int)ssrc
				 << "): could not connect to port."
				 << dec << TRANSMITTER_BASE_PIEC;

		cout << "Rx (" << hex << (int)ssrc
			 << "): " << local_ip.getHostname()
			 << " is waiting for salutes in port "
			 << dec << RECEIVER_BASE_PIEC << "..." << endl;

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

			*status_piec_ptr = ((unsigned char *)(adu->getData()))[0];
			delete adu;
		}
	}
};

int main()
{
	doc.load_file("./config.xml");

	int temperatura_min = doc.child("Dom").child("CzujnikTemperatury").child("ProgUruchomieniaPieca").attribute("Wartosc").as_int();
	int zwiekszTempO = doc.child("Dom").child("CzujnikTemperatury").child("WartoscZwiekszeniaTemperatury").attribute("Wartosc").as_int();
	int temperatura_aktualna = doc.child("Dom").child("CzujnikTemperatury").child("ZadanaTemperatura").attribute("Wartosc").as_int();
	int maxTemp = doc.child("Dom").child("CzujnikTemperatury").child("ProgZatrzymaniaGrzania").attribute("Wartosc").as_int();
	int zmniejszTempO = doc.child("Dom").child("CzujnikTemperatury").child("WartoscSpadaniaTemperatury").attribute("Wartosc").as_int();
	//int krokZmiany = doc.child("Dom").child("CzujnikTemperatury").child("KrokZmianyTemperatury").attribute("Wartosc").as_int();
	//krok zmiany nie wykorzystany, bo poslugujemy sie sleepem na 5 lub 6sekund
	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);

	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx(
		doc.child("Dom").child("CzujnikTemperatury").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("CzujnikTemperatury").child("PortRx").attribute("Wartosc").as_int(),
		doc.child("Dom").child("CzujnikTemperatury").child("PortTx").attribute("Wartosc").as_int());

	ccRTP_Hello_Rx *receiver = new ccRTP_Hello_Rx(
		doc.child("Dom").child("CzujnikTemperatury").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Piec").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("CzujnikTemperatury").child("PortRxPiec").attribute("Wartosc").as_int(),
		doc.child("Dom").child("CzujnikTemperatury").child("PortTxPiec").attribute("Wartosc").as_int());
	transmitter->start();
	receiver->start();
	cout << "Czujnik temperatury: " << endl;
	cout << "" << endl;
	cout << "" << endl;

	for (;;)
	{

		if (status_piec != '0')
		{
			temperatura_aktualna += zwiekszTempO;
		}
		else
		{
			temperatura_aktualna -= zmniejszTempO;
		}

		memcpy(&salute[1], (char *)&temperatura_aktualna, sizeof(int));
		cout << "Aktualna temperatura: " << dec << temperatura_aktualna << endl;

		if (temperatura_aktualna < temperatura_min)
		{
			salute[0] = '0';
		}
		else if (temperatura_aktualna > maxTemp)
		{
			salute[0] = '1';
		}

		sleep(5);
	}

	return 0;
}