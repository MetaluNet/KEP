/*********************************************************************
 *
 *                Gratte for 2X298
 *
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  jan 26 2013     Original.
 ********************************************************************/

/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/
#define BOARD 2X298
#include "config.h"
#include <fraise.h>
#include <dcmotor.h>
#include <pid.h>
#include <ramp.h>
#include <servo.h>

#define MOTC_INC K8
//-------------  Timer1 macros :  ---------------------------------------- 
//prescaler=PS fTMR1=FOSC/(4*PS) nbCycles=0xffff-TMR1init T=nbCycles/fTMR1=(0xffff-TMR1init)*4PS/FOSC
//TMR1init=0xffff-(T*FOSC/4PS) ; max=65536*4PS/FOSC : 
//ex: PS=8 : T=0.01s : TMR1init=0xffff-15000
//Maximum 1s !!
#define	TMR1init(T) (0xffff-((T*FOSC)/32000)) //ms ; maximum: 8MHz:262ms 48MHz:43ms 64MHz:32ms
#define	TMR1initUS(T) (0xffff-((T*FOSC)/32000000)) //us ; 
#define InitTimer(T) do{ TMR1H=TMR1init(T)/256 ; TMR1L=TMR1init(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define InitTimerUS(T) do{ TMR1H=TMR1initUS(T)/256 ; TMR1L=TMR1initUS(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define TimerOut() (PIR1bits.TMR1IF)

//t_DCMotor MotorA,MotorB,MotorC,MotorD;
t_DCMotor MotorC,MotorD;

//unsigned int T1=0;
//volatile long int DeltaT; // time between 2 incs
//volatile long int ICount=0;
//volatile char ILastInc=1; //1=incr -1=decr 

t_PID SpeedPID_C; //speed pid motor C
//volatile DWORD LastCapture4={0,0,0,0};
volatile unsigned long LastIncTimeC;
volatile unsigned long int DeltaTC; // interval between incs
volatile long int PosC;
volatile char DirectionC=1; //1=incr -1=decr 
int SpeedConsignC,PWMConsignC;
int ModeC ; //0:pwm 1:speed 2:pos 

unsigned int StroboTime=0xFFFF;
//0: off 1:on >1: strob
unsigned char StroboMode1=0;
unsigned char StroboMode2=0;
#define STROBOTIMEMIN 2048

/*unsigned*/ long int TestVar,TestVar2;

unsigned char t,t2;
#define GETTIMER(t) (((unsigned int)TMR##t##)|((unsigned int)TMR##t##H<<8))
	
#define SET_PWM(pwm,val) do{ CCP##pwm##CONbits.DC##pwm##B1 = val&2;  CCP##pwm##CONbits.DC##pwm##B0 = val&1; CCPR##pwm##L=val>>2; } while(0)

void UpdateMotorC()
{
	int v,vabs,pos,deltapos;
	long error;
	unsigned long delta,deltat,lastcapture;
	//static long int oldDeltaT=0;
	//static long int out;
	static long int actualSpeed;
	static unsigned long lastDelta;
	//static long lastspeed;
	static int LastPos;
	static char Stalled=0;
	/*static int computetime=1<<15-1;
	int _computetime;
	long begin;*/
//#define OUTMUL (512UL)
#define MAXSPEED ((unsigned long)(1UL<<22))
	
	//begin=GetTimer3();
	__critical { 
		deltat=DeltaTC;
		DeltaTC=0;
		lastcapture=LastIncTimeC;
		pos=PosC;
	}
	
	deltapos=pos-LastPos;
		//TestVar=deltapos;
	LastPos=pos;

	if(!deltapos) {
		delta=Elapsed(lastcapture);
		if(!Stalled) {
			if((delta>>16)>64) Stalled=1;	// more than (64<<16)/2 us = 2 sec -> speed=0
		}
		if(!Stalled) {
			if(delta<lastDelta) delta=lastDelta;
			deltapos=1; //make like a new step was occuring now.
		}
		//deltapos=1;
	}
	else {
		Stalled=0;
		delta=deltat;
		lastDelta=delta;
		//TestVar2=delta;
	}
		
		actualSpeed=deltapos*(MAXSPEED/((unsigned long)(delta)));
		TestVar=actualSpeed;
		TestVar2=delta;
	/*if(deltat) {
		delta=Elapsed3(lastcapture);
		if(delta<deltat) delta=deltat;
		if((delta>>16)>64) __critical {DeltaT=0;} // more than (64<<16)/2 us = 2 sec -> speed=0
	}
	else delta=0;*/
	
	/*if(delta>1000000) {
		TestVar2=delta;
		TestVar=deltat;
	}*/
	/*if(delta) actualSpeed=MAXSPEED/((unsigned long)(delta));
	else actualSpeed=0;*/
	
	//actualSpeed=deltapos*(MAXSPEED/((unsigned long)(delta)));
	
	//TestVar2=delta/deltapos;
	//TestVar=deltapos;
	//if(actualSpeed<1000) TestVar=actualSpeed;
	/*if(abs(lastspeed-actualSpeed)>10000) {
		TestVar=lastspeed;
		TestVar2=actualSpeed;
	}
	lastspeed=actualSpeed;*/
	
	
	//if(ILastInc==-1) actualSpeed=-actualSpeed;
	//TestVar=actualSpeed;
		
	if(ModeC==1) {
		error=((long)(SpeedConsignC)-(long)actualSpeed)/*>>4*/;
	
		if(error>=(1L<<15)) error=(1L<<15)-1;
		else if(error<-(1L<<15)) error=-(1L<<15);
		PIDCompute(&SpeedPID_C,(int)error);
	
		v=SpeedPID_C.Out>>8;
	}
	else v=PWMConsignC;
	
	if(v<0) v=0;
	//TestVar2=v;
	
	//v=PosPID_C.Out>>8;
	
	/*_computetime=Elapsed3(begin);
	if(_computetime<computetime) computetime=_computetime;
	TestVar=computetime;
	TestVar2=_computetime;*/
	
	if(v>1023) v=1023;
	if(v<-1023) v=-1023;
	//if(v<0) v=0; // one way
	v=(MotorC.Status.reversed?-v:v);
	
	vabs=v<0?1023+v:v;
	SET_PWM(2,vabs);
	
	if(v<0) { MOTC_IN1=1 ;  }
	else { MOTC_IN1=0 ; }	
}

void CaptureService()
{
	//static long int Last=0;
	//long int Now;
	//static unsigned long Capture; //,Now,LastNow;//={0,0,0,0},Now={0,0,0,0};//={(dword)0};
	static unsigned long d;
	static char LastincC, incC;
	static unsigned long int now;
	
	//if(!PIR4bits.CCP4IF) return;
	incC=DigitalRead(MOTC_INC);
	//if(!DigitalRead(MOTC_END)) { PosC=0; MotorC.Status.homed=1; }
	if( incC == LastincC) return;
	
	LastincC=incC;
	
	if(incC!=0) {
		now=GetTimeISR();
		//Capture=Now;
		d=(now-LastIncTimeC);//&0xFFFFFF;
		DeltaTC+=d;
		LastIncTimeC=now;
		PosC++;
		DirectionC=1;
		//TestVar=GetTime();
		//TestVar=Now;
		//TestVar2=d;
	}
}


void HighInterrupts()
{
	static char s;
	Servo_ISR();
	if(PIR1bits.TMR1IF) {
		CaptureService();
		InitTimerUS(10);
	}
	if(PIR2bits.TMR3IF) {
		PIR2bits.TMR3IF=0;
		if(s==0) s=1; 
		else s=0;
		if(StroboMode1>1) MOTD_IN1=s;
		if(StroboMode2>1) MOTD_IN2=!s;
		//DigitalWrite(MD1,s!=0);
		//DigitalWrite(MD2,s==0);
		TMR3H=0xFF-(StroboTime>>8);
		TMR3L=0xFF-(StroboTime&0xff);
	}	
}

void LowInterrupts()
{
	static char s;
	
	/*if(PIR2bits.TMR3IF) {
		PIR2bits.TMR3IF=0;
		if(StroboTime!=0) {
			s=(s==0);
			DigitalWrite(MD1,s);
			DigitalWrite(MD2,s==0);
			TMR3H=StroboTime>>8;
			TMR3L=StroboTime&0xff;
		}
		else {
			DigitalWrite(MD1,0);
			DigitalWrite(MD2,0);
			TMR3H=255;
			TMR3L=255;
		}
	}*/	
}

unsigned char PERIOD=255;

void main(void)
{
	unsigned long t3,servoSOF;

	//T1CON=0b00110011;//src=fosc/4,ps=8,16bit r/w,on.
	FraiseInit();
	
	MOTA_IN1=0;	
	MOTA_IN2=0;
	MOTB_IN1=0;	
	MOTB_IN2=0;
	MOTC_IN1=0;	
	MOTC_IN2=0;
	MOTD_IN1=0;	
	MOTD_IN2=0;

//	MOTA_EN=1;	
//	MOTB_EN=1;	
	MOTC_EN=1;	
	MOTD_EN=1;	
	
	//SETPORT_MOTA;
	//SETPORT_MOTB;
	SETPORT_MOTC;
	SETPORT_MOTD;
	SetPinDigiIn(MOTC_INC);	//SetPinAnsel(MOTC_INC,0);
		
	//SETPORT_MOT_STOP;

	SpeedConsignC=0;
	PWMConsignC=0;
	ModeC=0;
	DCMInit(&MotorC);
	PIDInit(&SpeedPID_C);

	EEreadMain();

// --------- init motors PWM on TMR2 -----------
	CCP2CON=CCP4CON=0b00001100 ; // single PWM active high
	//CCP2CON=
	T2CONbits.T2CKPS0=1; //prescaler 4
	PR2=255;
	T2CONbits.TMR2ON=1;

// ------- init PWM1 to pulse MOTC_IN1
//	PSTR1CON=0;
//	PSTR1CONbits.STR1D=1;
	
// ---------- capture timer : TMR1 ------------
	T1CON=0b00110011;//src=fosc/4,ps=8,16bit r/w,on.
	PIE1bits.TMR1IE=1;
	IPR1bits.TMR1IP=1;
	
/*// ---------- init Capture : CCP4(=RB0) on TMR3 -------------
	
	T3CONbits.T3CKPS0=1; //
	T3CONbits.T3CKPS1=1; //prescaler 8 : f=fosc/(4*8)=64MHz/32=2MHz : Troll=32ms->30Hz
	T3CONbits.T3RD16=1; // 16bits
	T3CONbits.TMR3ON=1;
	PIE2bits.TMR3IE=1;	// enable TMR3 interrupt
	IPR2bits.TMR3IP=1;	// high priority
	
	TRISBbits.TRISB0=1;
	ANSELBbits.ANSB0=0; // enable RB0 digital input buffer
	CCPTMRS1bits.C4TSEL0=1; //CCP4 capture uses TMR3
	CCP4CON=0b00000100 ; // capture every falling edge
	PIE4bits.CCP4IE=1;
	IPR4bits.CCP4IP=1;*/

// Led strobo on TMR3:
	T3CONbits.T3CKPS0=1; //
	T3CONbits.T3CKPS1=1; //prescaler 8 : f=fosc/(4*8)=64MHz/32=2MHz : Trollmax=32ms->30Hz
	T3CONbits.T3RD16=1; // 16bits
	T3CONbits.TMR3ON=1;
	PIE2bits.TMR3IE=1;	// enable TMR3 interrupt
	IPR2bits.TMR3IP=1;	// high priority
	
	Servo_Init();
	
	SERVO_SET_PORT(0,K2);

	t=PERIOD;
	servoSOF=t3=GetTime();
	
	while(1){
		FraiseService();
		Servo_Service();
		if(Elapsed(servoSOF)>Micros(18000UL)) {
			Servo_Rewind();
			servoSOF=GetTime();
		}

		if(Elapsed(t3)>Micros(5000UL))/*if(oldClock!=Clock)*/
		{
			t3=GetTime();
			UpdateMotorC();
			if(!t--){
				t=PERIOD;
				t2++;
				printf("C s2 %ld %ld %ld\n",PosC,TestVar,TestVar2);
			}
		}
	}
}

void CharBroadcast()
{
	/*unsigned char c;
	
	printf("Brdcst: ");
	for(c=0;c<FrRXout_len;c++) printf("%c",FrRXgetchar());
	putchar('\n');*/
}

void Broadcast()
{
	/*unsigned char c;
	
	printf("brdcst: ");
	for(c=0;c<FrRXout_len;c++) printf("%d ",FrRXgetchar());
	putchar('\n');*/
}

void CharInput()
{
	unsigned char c;
	
	c=FrRXgetchar();
	//printf("Cchar in:%c %c\n",c,c2);
	if(c=='L'){	
		c=FrRXgetchar();
		/*if(c=='0') 
			{LED=0;}
		else LED=1;*/
	}
	else if(c=='E') {
		printf("C");
		for(c=1;c<FrRXout_len;c++) printf("%c",FrRXgetchar());
		putchar('\n');
	}	
}

void Input()
{
	unsigned char c,c2;
	unsigned int i;
	
	c=FrRXgetchar();

	switch(c) {
		PARAM_CHAR(1,t2); break;
		PARAM_CHAR(2,PERIOD); break;
		//PARAM_CHAR(3,c2); DigitalWrite(MD1,c2!=0); break;
		//PARAM_CHAR(4,c2); DigitalWrite(MD2,c2!=0); break;
		PARAM_CHAR(3,StroboMode1); if(StroboMode1<2) MOTD_IN1=(StroboMode1!=0); break;
		PARAM_CHAR(4,StroboMode2); if(StroboMode2<2) MOTD_IN2=(StroboMode2!=0); break;
		PARAM_INT(5,StroboTime); if(StroboTime<STROBOTIMEMIN) StroboTime=STROBOTIMEMIN; break;
		PARAM_INT(6,i); SET_PWM(4,i); break;
		/*PARAM_INT(4,PWMVal);
			if(!FrStepStatus.pwm) { SetupPWM(); FrStepStatus.pwm=1; }
			CCP3CONbits.DC3B1=PWMVal&2;
			CCP3CONbits.DC3B0=PWMVal&1;
			CCPR3L=PWMVal>>2;
			break;*/
		#ifdef _DMX_H_
			PARAM_INT(6,i);
			if(!FrStepStatus.dmx) { DMXInit(); FrStepStatus.dmx=1; }
			DMXSet(i,FrRXgetchar());
			break;
		#endif
		case 20 : Servo_Input(20);break;
		//case 100 : DCMInput(&MotorA,EE_MOTORA,100);break;
		//case 101 : DCMInput(&MotorB,101);break;
		case 102 : DCMInput(&MotorC,102);break;
		//case 103 : DCMInput(&MotorD,EE_MOTORD,103);break;
		case 130 : PIDInput(&SpeedPID_C,130);break;
		PARAM_INT(131,SpeedConsignC); ModeC=1;break;
		PARAM_INT(132,PWMConsignC); ModeC=0;break;
		case 255 : EEwriteMain();break;
	}
}

void EEdeclareMain()
{
	EEsetaddress(EEUSER);
	
	DCM_declareEE(&MotorC);
	PID_declareEE(&SpeedPID_C);
}
