/*
============================================================================
 Name : main.cpp
 Author : P.Kutej
 Version :
 Copyright : Your copyright notice
 Description : Exe source file

Required:                   
    Connect Power through USB/External supply 12v
             ___________
            |   IDK     |
            |___________|
 
 ============================================================================
*/

#include "BBTerm.h"

int main() {

	BBTerm bbT(lcd);
	Ticker tckr;
	tckr.attach(&bbT, bbT.CToggle,(float)0.5);


    while(1) {           // Idle Loop
    	bbT.SPI1_ReceiveMsg();
    	bbT.EventOK_SPI1_SendMsg();
    	bbT.EventUP();
    	bbT.EventDOWN();
    	bbT.EventRIGHT();
    	bbT.EventLEFT();
    	bbT.CursorFlash();
     }
}
