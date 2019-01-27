#include "CzujnikSwiatla.hpp"
#include "Rolety.hpp"

const int RECEIVER_BASE = 33632;
const int TRANSMITTER_BASE = 32524;

unsigned char *statusCS;

CzujnikSwiatla::CzujnikSwiatla(unsigned char *statusCzujnikSwiatla)
{
        local_ip = "10.0.2.15";
		destination_ip = "10.0.2.15";
        statusCS=statusCzujnikSwiatla;
		// Is that correct?
		if( ! local_ip ){
		// this is equivalent to `! local_ip.isInetAddress()'
			cerr << "Tx: Local IP address is not correct!" << endl;
			exit();
		}
		if( ! destination_ip ){
		// this is equivalent to `! destination_ip.isInetAddress()'
			cerr << "Tx: Destination IP address is not correct!" << endl;
			exit();
		}

		// create socket for RTP connection and get a random
		// SSRC identifier
		socket = new RTPSession(local_ip,RECEIVER_BASE);
		ssrc = socket->getLocalSSRC();
}

CzujnikSwiatla::~CzujnikSwiatla()
{
    cout << endl << "Destroying receiver -ID: " << hex
		     << (int)ssrc;
		terminate();
		delete socket;
		cout << "... " << "destroyed.";
}
void CzujnikSwiatla::run(void){


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
		// Let's check the queues  (you should read the documentation
		// so that you know what the queues are for).


		// This is the main loop, where packets are received.
		for( ;; ){

			// Wait for an RTP packet.
			const AppDataUnit *adu = NULL;
			while ( NULL == adu ) {
				Thread::sleep(10);
				adu = socket->getData(socket->getFirstTimestamp());
			}
            //cout<<"waiting";
			// Print content (likely a salute :))
			// Note we are sure the data is an asciiz string.
			time_t receiving_time = time(NULL);
			char tmstring[30];
			strftime(tmstring,30,"%X",localtime(&receiving_time));
			cout << "Rx (" << hex << (int)ssrc
			     << "): [receiving at " << tmstring << "]: "
			     <<	adu->getData() << endl;
            unsigned char tmp = *statusCS;
            *statusCS=((unsigned char *)(adu->getData()))[0];
            if(tmp!=*statusCS)
            {
                Rolety *rolety = new Rolety(*statusCS);
                rolety->start();
            }
			delete adu;
		}
};