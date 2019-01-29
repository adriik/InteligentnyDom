#pragma once
#include "pugixml.hpp"
#define NAZWA_LOGU "Kontroler_log"
using namespace pugi;
extern xml_document doc;
extern int wilgotnosc;
extern int temperatura;
extern bool zapiszLog(char const *tekst);
