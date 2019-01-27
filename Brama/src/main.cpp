#include <cstdio>
#include <ctime>
// In order to use ccRTP, the RTP stack of CommonC++, just include...
#include <ccrtp/rtp.h>
//#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
//#endif
#define NAZWA_LOGU "Brama_log"
char buforDlaLogu[255];

bool czyscLog()
{
    FILE *plik = NULL;
    plik = fopen(NAZWA_LOGU, "wt");
    if(plik != NULL)
    {
        return true;
    }
    return false;
}

bool zapiszLog(char* tekst)
{
    FILE *plik = NULL;
    plik = fopen(NAZWA_LOGU, "a");
    if(plik != NULL)
    {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        fprintf(plik, "%02d/%02d/%0d %02d:%02d:%02d: %s\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, tekst);
        fclose(plik);
        return true;
    }
    return false;
}

// base ports
const int RECEIVER_BASE = 33630;
const int TRANSMITTER_BASE = 32522;

// For this example, this is irrelevant.
//const int TIMESTAMP_RATE = 90000;

/**
 * @class ccRTP_Hello_Rx
 * Receiver of salutes.
 */
class ccRTP_Hello_Rx: public Thread
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
	ccRTP_Hello_Rx(){
		// Before using ccRTP you should learn something about other
		// CommonC++ classes. We need InetHostAddress...

		// Construct loopback address
		local_ip = "10.0.2.15";
		destination_ip = "10.0.2.15";

		// Is that correct?
		if( ! local_ip ){
			// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Rx: Local IP address is not correct!" << endl;
			exit();
		}
		// Is that correct?
		if( ! destination_ip ){
			// this is equivalent to `! destination_ip.isInetAddress()'
			cerr << "Rx: Destination IP address is not correct!" << endl;
			exit();
		}

		// create socket for RTP connection and get a random
		// SSRC identifier
		socket = new RTPSession(local_ip,RECEIVER_BASE);
		ssrc = socket->getLocalSSRC();
	}

	~ccRTP_Hello_Rx(){
		cout << endl << "Destroying receiver -ID: " << hex
		     << (int)ssrc;
		terminate();
		delete socket;
		cout << "... " << "destroyed.";
	}

	// This method does almost everything.
	void run(void){


		socket->setSchedulingTimeout(20000);
		socket->setExpireTimeout(3000000);
		//socket->UDPTransmit::setTypeOfService(SOCKET_IPTOS_LOWDELAY);
		if( !socket->addDestination(destination_ip,TRANSMITTER_BASE) )
			cerr << "Rx (" << hex << (int)ssrc
			     << "): could not connect to port."
			     <<  dec << TRANSMITTER_BASE;

		cout << "Rx (" << hex << (int)ssrc
		     << "): " << local_ip.getHostname()
		     <<	" is waiting for salutes in port "
		     <<  dec << RECEIVER_BASE << "..." << endl;

		socket->setPayloadFormat(StaticPayloadFormat(sptMP2T));
		socket->startRunning();

		cout << "Rx (" << hex << (int)ssrc
		     << "): The queue is "
		     << ( socket->isActive() ? "" : "in")
		     << "active." << endl;

		// This is the main loop, where packets are received.
		for( int i = 0 ; true ; i++ ){

			// Wait for an RTP packet.
			const AppDataUnit *adu = NULL;
			while ( NULL == adu ) {
				Thread::sleep(10);
				adu = socket->getData(socket->getFirstTimestamp());
			}

			// Print content (likely a salute :))
			// Note we are sure the data is an asciiz string.
			time_t receiving_time = time(NULL);
			char tmstring[30];
			strftime(tmstring,30,"%X",localtime(&receiving_time));
			cout << "Rx (" << hex << (int)ssrc
			     << "): [receiving at " << tmstring << "]: "
			     <<	adu->getData() << endl;
			delete adu;
		}
	}
};


int main(int argc, char *argv[])
{
	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);

	// Construct the two main threads. they will not run yet.
	ccRTP_Hello_Rx *receiver = new ccRTP_Hello_Rx;



	// Start execution of hello now.
	receiver->start();
	//transmitter->start();

	cin.get();


	delete receiver;

	cout << endl << "That's all." << endl;

	return 0;
}