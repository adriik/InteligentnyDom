#include <cstdio>
#include <ctime>
// In order to use ccRTP, the RTP stack of CommonC++, just include...
#include <ccrtp/rtp.h>
//#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
//#endif
#define NAZWA_LOGU "CzujnikTemperatury_log"
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
const int RECEIVER_BASE = 33626;
const int TRANSMITTER_BASE = 32518;

const int RECEIVER_BASE_PIEC = 33688;
const int TRANSMITTER_BASE_PIEC = 32588;

// For this example, this is irrelevant.
//const int TIMESTAMP_RATE = 90000;

//int status;
unsigned char salute[50]={'0'};
unsigned char status_piec='0';
unsigned char *status_piec_ptr=&status_piec;

/**
 * @class ccRTP_Hello_Tx
 * Transmitter of salutes.
 */
class ccRTP_Hello_Tx: public Thread, public TimerPort
{

private:
	// socket to transmit
	RTPSession *socket;
	// loopback network address
	InetHostAddress local_ip;
	InetHostAddress destination_ip;
	// identifier of this sender
	uint32 ssrc;


public:
	ccRTP_Hello_Tx(){


		// Construct loopback address
		local_ip = "10.0.2.15";
		destination_ip = "10.0.2.15";

		// Is that correct?
		if( ! local_ip ){
		// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Tx: Local IP address is not correct!" << endl;
			exit();
		}
		// Is that correct?
		if( ! destination_ip ){
		// this is equivalent to `! destination_ip.isInetAddress()'
			cerr << "Tx: Destination IP address is not correct!" << endl;
			exit();
		}

		socket = new RTPSession(local_ip,TRANSMITTER_BASE);
		ssrc = socket->getLocalSSRC();
	}

	~ccRTP_Hello_Tx(){
		cout << endl << "Destroying transmitter -ID: " << hex
		     << (int)ssrc;
		terminate();
		delete socket;
		cout << "... " << "destroyed.";
	}

	// This method does almost everything.
	void run(void){


		// Set up connection
		socket->setSchedulingTimeout(20000);
		socket->setExpireTimeout(3000000);
		if( !socket->addDestination(destination_ip,RECEIVER_BASE) )
			cerr << "Tx (" << hex << (int)ssrc
			     << "): could not connect to port."
			     <<  dec << RECEIVER_BASE;



		uint32 timestamp = 0;
		// This will be useful for periodic execution
		TimerPort::setTimer(5000);

		socket->setPayloadFormat(StaticPayloadFormat(sptMP2T));
		socket->startRunning();

		cout << "Tx (" << hex << (int)ssrc << "): The queue is "
             << ( socket->isActive()? "" : "in")
		     << "active." << endl;

		for(  ;  ; ){



			time_t sending_time = time(NULL);

            timestamp = socket->getCurrentTimestamp();


			socket->putData(timestamp,salute,
					strlen((char *)salute)+1);
			// print info
			char tmstring[30];
			strftime(tmstring,30,"%X",
				 localtime(&sending_time));
			cout << "Tx (" << hex << (int)ssrc
			     << "): Czujnik temperatury stan "<< dec << salute[0]
			     << ", at " << tmstring
			     << "..." << endl;

			// Let's wait for the next cycle
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(5000);
		}
	}
};

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
		socket = new RTPSession(local_ip,RECEIVER_BASE_PIEC);
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
		if( !socket->addDestination(destination_ip,TRANSMITTER_BASE_PIEC) )
			cerr << "Rx (" << hex << (int)ssrc
			     << "): could not connect to port."
			     <<  dec << TRANSMITTER_BASE_PIEC;

		cout << "Rx (" << hex << (int)ssrc
		     << "): " << local_ip.getHostname()
		     <<	" is waiting for salutes in port "
		     <<  dec << RECEIVER_BASE_PIEC << "..." << endl;

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
			     << "): [receiving from piec at " << tmstring << "]: "
			     <<	adu->getData() << endl;
			*status_piec_ptr=((unsigned char *)(adu->getData()))[0];
			delete adu;
		}
	}
};

int main(int argc, char *argv[])
{
	int temperatura_min=15;
	int temperatura_aktualna=18;
	int zadana= 24;
	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);

	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx;
	ccRTP_Hello_Rx *receiver = new ccRTP_Hello_Rx;
	transmitter->start();
	receiver->start();
    cout<<"Czujnik temperatury: "<<endl;
        cout<<""<<endl;
        cout<<""<<endl;
	for(;;){
		
        //cin>>status;
        
		if(temperatura_aktualna<temperatura_min)
        {
            salute[0]='0';
        }
        else if(temperatura_aktualna>zadana)
        {
            salute[0]='1';
        }
		if(status_piec!='0')
		{
			temperatura_aktualna++;
		}
		else
		{
			temperatura_aktualna--;
			
		}
		cout<<"Aktualna temperatura: "<<dec<<temperatura_aktualna<<endl;
		sleep(5);
    }


	return 0;
}