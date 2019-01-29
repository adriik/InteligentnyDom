#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>
#include "pugixml.hpp"
#include "parser.hpp"

using namespace ost;
using namespace std;

#define NAZWA_LOGU "CzujnikSwiatla_log"
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

// For this example, this is irrelevant.
//const int TIMESTAMP_RATE = 90000;

int status;
unsigned char salute[50] = {'0'};

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

	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx(
		doc.child("Dom").child("CzujnikSwiatla").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
		doc.child("Dom").child("CzujnikSwiatla").child("PortRx").attribute("Wartosc").as_int(),
		doc.child("Dom").child("CzujnikSwiatla").child("PortTx").attribute("Wartosc").as_int());

	transmitter->start();

	cout << "Zmodyfikuj stan czujnika swiatla aby uruchomic oswietlenie naturalne.\nKSiezyc wlaczony. Wcisnij 1 aby uruchomic Slonce" << endl;
	int prevStat = 0;
	for (;;)
	{

		cin >> status;
		if (status == 0 && status != prevStat)
		{
			salute[0] = '0';
			cout << "Ksiezyc wlaczony. Wcisnij 1 aby uruchomic Slonce" << endl;
		}
		else if (status == 1 && status != prevStat)
		{
			salute[0] = '1';
			cout << "Slonce wlaczone. Wcisnij 0 aby uruchomic Ksiezyc" << endl;
		}
		prevStat = status;
	}

	return 0;
}