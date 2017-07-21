/*
 * BBTerm.h
 *
 *  Created on: Jul 8, 2017
 *      Author: fg7fww
 */
#include <ticker.h>
#include "mbed.h"
#include "Shields.h"

#ifndef BBTERM_H_
#define BBTERM_H_

// SPI1 handshake commands
#define	SPI1_DUMMY			0xF0000005
#define SPI1_MSG_SEND		0xF0000006
#define SPI1_INT2BCD		0xF0000007
#define SPI1_MSG_DONE		0xF0000008

#define TITLE   "ToMob:+000000000"
#define TITLE2	"Tx/Rx:"
Serial uart2(USBTX, USBRX);
NHD_C0216CZ lcd;
PCA9655E ioxp_S(p3,p2,0x4A,p17);
PCA9655E ioxp_B(p3,p2,0x20,p17);
int spis_out=0;
SPI spim(p5,p6,p4, NC);


class BBTerm /* :Ticker */ {

private:
#define COLBEG 			7
#define COLROWCURMAX	9
#define COL0MAX			13
#define COL1TOMAX		11
#define MSGLEN			10
#define CURSORPERIOD	0.5
#define CURSOR			'#'
#define COL0CHARSET		col0[r1[colRowCur]]
#define	COL1CHARSET		col1[r1[colRowCur]]
#define COLCHARSET		(colRowCur==0?COL0CHARSET:COL1CHARSET)
#define COL0ICHARSET	col0[r1[i]]
#define	COL1ICHARSET	col1[r1[i]]
#define COLICHARSET		(i==0?COL0ICHARSET:COL1ICHARSET)
#define COLMAX			((colRowCur==0?COL0MAX:COL1TOMAX)-1)
	//SPI1 send, receive int32
	int SPI1_WriteInt32(int);
	int SPI1_ReadInt32(void);
	//Transfer data field type
	/*
	union data_t {
		unsigned char byte[MSGLEN];
		char c[MSGLEN]={'+', '0', '0', '0', '0', '0','0','0','0','0'};
	};
	*/
	//Set of characters for first row column
	char col0[COL0MAX]={'0','1','2','3','4','5','6','7','8','9',',','+','-'};
	//Set of characters for other row column
	char col1[COL1TOMAX]={'0','1','2','3','4','5','6','7','8','9',','};
	//Message map for sending
	int	r1[MSGLEN]={0xB,0,0,0,0,0,0,0,0,0};
	int row1ColCur, row2ColCur, colRowCur;
	//data_t data_rx;
	int d_rx;
	char CursorSwap=CURSOR;
	void CursorToggle(void);
	int cFlash;
	int FLASHREADY=1;

public:

	BBTerm(NHD_C0216CZ lcd) {
		lcd.init();
		lcd.displayStringToPos(TITLE, 1, 1);
		lcd.displayStringToPos(TITLE2, 2, 1);
		spim.format(32, 0);
		spim.frequency(100);
		ioxp_S.setDirection(PORT1, 0, OUTPUT);
		ioxp_S.setValue(PORT1,0,SET_HIGH);
		ioxp_S.setDirection(PORT0, 0, INPUT);
		ioxp_S.setDirection(PORT0, 4, INPUT);
		uart2.printf("\n");
		row1ColCur=row2ColCur=0;
		colRowCur=0;
		cFlash=0;
		BBTerm::CToggle=&BBTerm::CursorToggle;
		/*
		BBTerm::Ticker Tckr2;
		Tckr2.attach(this,this->CToggle,(float)0.5);
		*/
		d_rx=0;
	}

	virtual ~BBTerm() {
	}

	void EventOK_SPI1_SendMsg(void);
	void EventUP(void);
	void EventDOWN(void);
	void EventRIGHT(void);
	void EventLEFT(void);
	void SPI1_ReceiveMsg(void);
	void CursorFlash(void);
	void (BBTerm::*CToggle)(void);
	//Transfered fields
	//data_t data_tx;
	//Received field
	//data_t data_rx;

};

#endif /* BBTERM_H_ */
