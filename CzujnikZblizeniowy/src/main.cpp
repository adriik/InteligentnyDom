#include <cstdio>
#include <ctime>
// In order to use ccRTP, the RTP stack of CommonC++, just include...
#include <ccrtp/rtp.h>
//#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
//#endif
#define NAZWA_LOGU "CzujnikZblizeniowy_log"
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
const int RECEIVER_BASE = 33628;
const int TRANSMITTER_BASE = 32520;

// For this example, this is irrelevant.
//const int TIMESTAMP_RATE = 90000;

int status;
unsigned char salute[50]={'0'};

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
			     << "): Czujnik zblizeniowy stan "<< dec << status
			     << ", at " << tmstring
			     << "..." << endl;

			// Let's wait for the next cycle
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(5000);
		}
	}
};

int main(int argc, char *argv[])
{
	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);

	ccRTP_Hello_Tx *transmitter = new ccRTP_Hello_Tx;

	transmitter->start();

    for(;;){
        cout<<"zmodyfikuj stan czujnika zblizeniowego aby otworzyc/zamknac brame"<<endl;
        cout<<"wprowadz 0 lub 1 i potwierdz"<<endl;
        cin>>status;
        if(status==0)
        {
            salute[0]='0';
        }
        else
        {
            salute[0]='1';
        }
    }


	return 0;
}