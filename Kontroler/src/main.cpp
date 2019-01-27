#include<CzujnikSwiatla.hpp>
#include<CzujnikZblizeniowy.hpp>
#include<CzujnikTemperatury.hpp>
#include<Rolety.hpp>
#include<Brama.hpp>
#include<Piec.hpp>
#include "pugixml.hpp"

#include <cstdio>
#include <ctime>
// In order to use ccRTP, the RTP stack of CommonC++, just include...
#include <ccrtp/rtp.h>
//#ifdef  CCXX_NAMESPACES
using namespace ost;
using namespace std;
using namespace pugi;
//#endif
#define NAZWA_LOGU "Kontroler_log"



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

unsigned char statusCzujnikSwiatla = '9';
unsigned char statusCzujnikZblizeniowy = '9';
unsigned char statusCzujnikTemperatury = '9';
unsigned char *ptr, *ptr1, *ptr2;
//w pierwszym wywoaniu w kontrolerze () dorobic napisy do Logow dot. startu kontrolnego

int main(int argc, char *argv[])
{

    xml_document doc;
    xml_parse_result result = doc.load_file("./config.xml");
    int wartosc = doc.child("Dom").child("Kontroler").child("WysokoscDolnegoSensoraWody").attribute("Wartosc").as_int();

    cout << "Wartosc = " << wartosc << endl;

	zapiszLog("Tresc przykladowego logu");
	snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Przykladowa wartosc parametryzowana %d", 50);
	zapiszLog(buforDlaLogu);
    /*
    na start trzeba uruchomic watki, ktore odbieraja dane
    bo do rolet, bramy to my wysylamy sygnal aby sie odpalily
    czyli jak cos odczytamy z czujnika swiatla to dopiero wtedy wysylamy
    */ 
    ptr=&statusCzujnikSwiatla;
    ptr1=&statusCzujnikZblizeniowy;
    ptr2=&statusCzujnikTemperatury;

	//klasy odbierajace dane
	CzujnikSwiatla *czujnikSwiatla = new CzujnikSwiatla(ptr);
    CzujnikZblizeniowy *czujnikZblizeniowy = new CzujnikZblizeniowy(ptr1);
    CzujnikTemperatury *czujnikTemperatury = new CzujnikTemperatury(ptr2);




	// Start execution of hello now.
	czujnikSwiatla->start();
    czujnikZblizeniowy->start();
    czujnikTemperatury->start();

    for(;;)
    {
        cout<<statusCzujnikSwiatla<<endl;
        cout<<statusCzujnikZblizeniowy<<endl;
        cout<<statusCzujnikTemperatury<<endl;
        sleep(3);

    }


	//delete transmitter;
    //delete receiver;

	return 0;
}

