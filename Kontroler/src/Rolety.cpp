#include "Rolety.hpp"

const int RECEIVER_BASE = 33634;
const int TRANSMITTER_BASE = 32526;

unsigned char salute[50]={'0'};

	Rolety::Rolety(unsigned char statusRolety){

		local_ip = "10.0.2.15";
		destination_ip = "10.0.2.15";
        status = statusRolety;
        salute[0]=statusRolety;
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

		socket = new RTPSession(local_ip,TRANSMITTER_BASE);
		ssrc = socket->getLocalSSRC();
	}

	Rolety::~Rolety(){
		cout << endl << "Destroying transmitter -ID: " << hex
		     << (int)ssrc;
		terminate();
		delete socket;
		cout << "... " << "destroyed.";
	}

	// This method does almost everything.
	void Rolety::run(void){
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

		//cout << "Tx (" << hex << (int)ssrc << "): The queue is "
        //    << ( socket->isActive()? "" : "in")
		//     << "active." << endl;

		//for(;;){

			time_t sending_time = time(NULL);

            timestamp = socket->getCurrentTimestamp();


			socket->putData(timestamp,salute,
					strlen((char *)salute)+1);
			// print info
			char tmstring[30];
			strftime(tmstring,30,"%X",
				 localtime(&sending_time));
			cout << "Tx (" << hex << (int)ssrc
			     << "): Zmien stan rolet na: "<< dec << status
			     << ", at " << tmstring
			     << "..." << endl;

			// Let's wait for the next cycle
			Thread::sleep(TimerPort::getTimer());
			TimerPort::incTimer(5000);
		//}
	}