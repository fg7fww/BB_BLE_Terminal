/*
 * BBTerm.cpp
 *
 *  Created on: Jul 8, 2017
 *      Author: fg7fww
 */

#include "BBTerm.h"

void BBTerm::EventOK_SPI1_SendMsg(){
	FLASHREADY=0;
	if(0==ioxp_B.readPinVal(PORT0,6))
	{
		//OK	6
		uart2.printf("%s\n","OK");
		spis_out=SPI1_WriteInt32(SPI1_MSG_SEND);
		for(int i=0;i<MSGLEN;i++){
			if(CURSOR==COLICHARSET){
				COLICHARSET=CursorSwap;
				CursorSwap=CURSOR;
			}
			spis_out=SPI1_WriteInt32((int)COLICHARSET);
			lcd.setPosition(2,COLBEG+i);
			lcd.displayChar((char)COLICHARSET);
			uart2.printf("%c",(char)COLICHARSET);
			uart2.printf("echo %c\n",(char)spis_out);
			wait(0.01);
		}
		spis_out=SPI1_WriteInt32(SPI1_MSG_DONE);
		uart2.printf("\n");
		while(0==ioxp_B.readPinVal(PORT0,6)){}
	}
	FLASHREADY=1;
}
void BBTerm::EventUP(void){
	FLASHREADY=0;
	if(0==ioxp_B.readPinVal(PORT0,7))
	{
		//UP	7
		if(CURSOR==COLCHARSET){
			COLCHARSET=CursorSwap;
			CursorSwap=CURSOR;
			lcd.setPosition(1,COLBEG+colRowCur);
			lcd.displayChar((char)COLCHARSET);
		}
		row1ColCur=r1[colRowCur];
		row1ColCur<COLMAX?row1ColCur++:row1ColCur=0;
		r1[colRowCur]=row1ColCur;
		lcd.setPosition(1,COLBEG+colRowCur);
		lcd.displayChar((char)COLCHARSET);
		uart2.printf("%s\n","UP");
		while(0==ioxp_B.readPinVal(PORT0,7)){}
	}
	FLASHREADY=1;
}
void BBTerm::EventDOWN(void){
	FLASHREADY=0;
	if(0==ioxp_B.readPinVal(PORT0,4))
	{
		//DOWN	4
		if(CURSOR==COLCHARSET){
			COLCHARSET=CursorSwap;
			CursorSwap=CURSOR;
			lcd.setPosition(1,COLBEG+colRowCur);
			lcd.displayChar((char)COLCHARSET);
		}
		row1ColCur=r1[colRowCur];
		row1ColCur>0?row1ColCur--:row1ColCur=COLMAX;
		r1[colRowCur]=row1ColCur;
		lcd.setPosition(1,COLBEG+colRowCur);
		lcd.displayChar((char)COLCHARSET);
		uart2.printf("%s\n","DOWN");
		while(0==ioxp_B.readPinVal(PORT0,4)){}
	}
	FLASHREADY=1;
}
void BBTerm::EventRIGHT(void){
	FLASHREADY=0;
	if(0==ioxp_B.readPinVal(PORT1,0))
	{
		//Right	8
		if(CURSOR==COLCHARSET){
			COLCHARSET=CursorSwap;
			CursorSwap=CURSOR;
			lcd.setPosition(1,COLBEG+colRowCur);
			lcd.displayChar((char)COLCHARSET);
		}
		colRowCur<COLROWCURMAX?colRowCur++:colRowCur=0;
		uart2.printf("%s\n","Right");
		while(0==ioxp_B.readPinVal(PORT1,0)){}
	}
	FLASHREADY=1;
}
void BBTerm::EventLEFT(void){
	FLASHREADY=0;
	if(0==ioxp_B.readPinVal(PORT0,5))
	{
		//LEFT	5
		if(CURSOR==COLCHARSET){
			COLCHARSET=CursorSwap;
			CursorSwap=CURSOR;
			lcd.setPosition(1,COLBEG+colRowCur);
			lcd.displayChar((char)COLCHARSET);
		}
		colRowCur>0?colRowCur--:colRowCur=COLROWCURMAX;
		uart2.printf("%s\n","LEFT");
		while(0==ioxp_B.readPinVal(PORT0,5)){}
	}
	FLASHREADY=1;
}

int BBTerm::SPI1_WriteInt32(int write_int32){
	int ret;

	ioxp_S.setValue(PORT1,0,SET_LOW);
	ret=spim.write(write_int32);
	ioxp_S.setValue(PORT1,0,SET_HIGH);
	return ret;
}

int BBTerm::SPI1_ReadInt32(void){
	spis_out=-1;
	if(0==ioxp_S.readPinVal(PORT0, 0)){
		spis_out=SPI1_WriteInt32(SPI1_DUMMY);
		switch((unsigned int)spis_out){
		case SPI1_MSG_DONE:
			uart2.printf("\n");
			for(int i=row2ColCur;i<MSGLEN;i++){
				lcd.setPosition(2,COLBEG+i);
				lcd.displayChar((char)'0');
			}
			row2ColCur=0;
			break;
		default:
			uart2.putc((int)spis_out);
			lcd.setPosition(2,COLBEG+row2ColCur);
			lcd.displayChar((char)spis_out);
			//while(0==ioxp_S.readPinVal(PORT0,0)){}
			row2ColCur<MSGLEN?row2ColCur++:row2ColCur=0;
			break;
		}
	}
    return spis_out;

}

void BBTerm::SPI1_ReceiveMsg(void){
	FLASHREADY=0;
	d_rx=SPI1_ReadInt32();
	FLASHREADY=1;

}

void BBTerm::CursorFlash(void){
	if((FLASHREADY)&&(cFlash)){
		if(CURSOR!=COLCHARSET){
			CursorSwap=COLCHARSET;
			COLCHARSET=CURSOR;
			}else{
				COLCHARSET=CursorSwap;
			}
		lcd.setPosition(1,COLBEG+colRowCur);
		lcd.displayChar((char)COLCHARSET);
		this->CursorToggle();
	}
}

void BBTerm::CursorToggle(void){
	cFlash=!cFlash;
}
