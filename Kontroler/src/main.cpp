#include<CzujnikSwiatla.hpp>
#include<CzujnikZblizeniowy.hpp>
#include<CzujnikTemperatury.hpp>
#include<CzujnikWilgotnosci.hpp>
#include<Rolety.hpp>
#include<Brama.hpp>
#include<Piec.hpp>
#include<Zraszacz.hpp>
#include "pugixml.hpp"
#include "parser.hpp"
#include <cstdio>
#include <ctime>
#include <ccrtp/rtp.h>

using namespace ost;
using namespace std;
using namespace pugi;

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

bool zapiszLog(char const *tekst)
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
unsigned char statusCzujnikTemperatury = '1';
unsigned char statusCzujnikWilgotnosci = '1';
unsigned char *ptr, *ptr1, *ptr2, *ptr3;

xml_document doc;

int main()
{
    doc.load_file("./config.xml");

    ptr=&statusCzujnikSwiatla;
    ptr1=&statusCzujnikZblizeniowy;
    ptr2=&statusCzujnikTemperatury;
    ptr3=&statusCzujnikWilgotnosci;

	//klasy odbierajace dane
	CzujnikSwiatla *czujnikSwiatla = new CzujnikSwiatla(
        doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikSwiatla").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikSwiatla").child("PortRx").attribute("Wartosc").as_int(),
        doc.child("Dom").child("CzujnikSwiatla").child("PortTx").attribute("Wartosc").as_int()   
    );
    czujnikSwiatla->statusCzujnikSwiatla = ptr;

    CzujnikZblizeniowy *czujnikZblizeniowy = new CzujnikZblizeniowy(
        doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikZblizeniowy").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikZblizeniowy").child("PortRx").attribute("Wartosc").as_int(),
        doc.child("Dom").child("CzujnikZblizeniowy").child("PortTx").attribute("Wartosc").as_int()
    );
    czujnikZblizeniowy->statusCzujnikZblizeniowy = ptr1;



    CzujnikTemperatury *czujnikTemperatury = new CzujnikTemperatury(
        doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikTemperatury").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikTemperatury").child("PortRx").attribute("Wartosc").as_int(),
        doc.child("Dom").child("CzujnikTemperatury").child("PortTx").attribute("Wartosc").as_int()       
    );
    czujnikTemperatury->statusCzujnikTemperatury = ptr2;

    CzujnikWilgotnosci *czujnikWilgotnosci = new CzujnikWilgotnosci(
        doc.child("Dom").child("Kontroler").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikWilgotnosci").child("AdresIP").attribute("Wartosc").as_string(),
        doc.child("Dom").child("CzujnikWilgotnosci").child("PortRx").attribute("Wartosc").as_int(),
        doc.child("Dom").child("CzujnikWilgotnosci").child("PortTx").attribute("Wartosc").as_int()       
    );
    czujnikWilgotnosci->statusCzujnikWilgotnosci = ptr3;


	czujnikSwiatla->start();
    czujnikZblizeniowy->start();
    czujnikTemperatury->start();
    czujnikWilgotnosci->start();

    for(;;)
    {

		printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
		printf("║                               Inteligentny dom                               ║\n");
		printf("╠═══════════════════════════════════════╦══════════════════════════════════════╣\n");
		printf("║               Czujniki                ║               Efektory               ║\n");
		printf("╠═══════════════════════════════════════╬══════════════════════════════════════╣\n");
        
		if(statusCzujnikSwiatla == '0')
        {
        zapiszLog("Status czujnika swiatla: wylaczony");
		printf("║ Czujnik swiatla: Nieaktywny           ║ Rolety: Opuszczone                   ║\n");
        }
        else if(statusCzujnikSwiatla == '1')
        {
        zapiszLog("Status czujnika swiatla: wlaczony");
		printf("║ Czujnik swiatla: Aktywny              ║ Rolety: Podniesione                  ║\n");
        }
		printf("╠═══════════════════════════════════════╬══════════════════════════════════════╣\n");
        if(statusCzujnikZblizeniowy == '0')
        {
        zapiszLog("Status zblizeniowy: wylaczony");
		printf("║ Czujnik zblizeniowy: Nieaktywny       ║ Brama: Zamknieta                     ║\n");
        }
        else if(statusCzujnikZblizeniowy == '1')
        {
        zapiszLog("Status zblizeniowy: wlaczony");
		printf("║ Czujnik zblizeniowy: Aktywny          ║ Brama: Otwarta                       ║\n");
        }
        printf("╠═══════════════════════════════════════╬══════════════════════════════════════╣\n");
		if(statusCzujnikTemperatury == '0')
        {
        snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Piec wlaczony. Obecna wartosc temperatury: %d", temperatura);
        zapiszLog(buforDlaLogu);
		printf("║ Czujnik temperatury: %d               ║ Piec: Wlaczony                       ║\n",temperatura);
        }
        else if(statusCzujnikTemperatury == '1')
        {
        snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Piec wylaczony. Obecna wartosc temperatury: %d", temperatura);
        zapiszLog(buforDlaLogu);
		printf("║ Czujnik temperatury: %d               ║ Piec: Wylaczony                      ║\n",temperatura);
        }
        printf("╠═══════════════════════════════════════╬══════════════════════════════════════╣\n");
		if(statusCzujnikWilgotnosci == '0')
        {
        snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Zraszacz wlaczony. Obecna wartosc wilgotnosci: %d", wilgotnosc);
        zapiszLog(buforDlaLogu);
		printf("║ Czujnik wilgotnosci: %d               ║ Zraszacz: Wlaczony                   ║\n", wilgotnosc);
        }
        else if(statusCzujnikWilgotnosci == '1')
        {
        snprintf(buforDlaLogu, sizeof(buforDlaLogu), "Zraszacz wylaczony. Obecna wartosc wilgotnosci: %d", wilgotnosc);
        zapiszLog(buforDlaLogu);
		printf("║ Czujnik wilgotnosci: %d               ║ Zraszacz: Wylaczony                  ║\n", wilgotnosc);
        }
		printf("╚═══════════════════════════════════════╩══════════════════════════════════════╝\n"); 
        sleep(2);
        system("clear");
    }
	return 0;
}

