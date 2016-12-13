/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <analog.h>
#include <dcmotor.h>

// Timer macros
#define TIMER 3
#include <timer.h>

//  val = /*0xFFFF - */pulseWidth;

#define SET_TIMER(us) do {\
  val = 0xFFFF - us;\
  TIMER_H = val>>8;\
  TIMER_L = val&255;\
  TIMER_IF = 0;\
  TIMER_ON = 1; } while(0)

#define TIMER_INIT() do{\
	TIMER_CON = 0; \
	TIMER_PS1 = 0;  /* 	no prescaler */\
	TIMER_16BIT = 1;/* 	16bits                          */\
	TIMER_IP = 1;	/* 	high/low priority interrupt */\
	TIMER_ON = 0;	/* 	stop timer                      */\
	TIMER_IF = 0;   /*  clear flag                      */\
	TIMER_IE = 1;	/* 	enable timer interrupt         */\
} while(0)


t_delay mainDelay, pulseDelay;
char pulsePhase = 0;
unsigned int pulseWidth = 1000;
unsigned int val;

DCMOTOR_DECLARE(C);
DCMOTOR_DECLARE(D);

void pulseFire()
{
  //delayStart(pulseDelay, pulseWidth);
  pulsePhase = 2;
  digitalSet(MC1);
  digitalClear(MC2);
  SET_TIMER(pulseWidth);
}

/*void pulseService()
{
  if(pulsePhase == 1) {
    if(!delayFinished(pulseDelay)) return;
    digitalClear(MC1);
    digitalSet(MC2);
    delayStart(pulseDelay, pulseWidth);
    pulsePhase = 2;
  } else if(pulsePhase == 2) {
    if(!delayFinished(pulseDelay)) return;
    digitalClear(MC1);
    digitalClear(MC2);
    pulsePhase = 0;
  }
}*/

// ----------- Interrupts --------------
 
void highInterrupts(void)
{
    //static unsigned int val;
    
    if(TIMER_IF == 0) return;
    
    TIMER_ON = 0;
    TIMER_IF = 0;

    if(pulsePhase == 1) {
      digitalClear(MC1);
      digitalClear(MC2);
      SET_TIMER(pulseWidth);
      pulsePhase = 2;
      return;
    } else if(pulsePhase == 2) {
      digitalSet(MC2);
      digitalClear(MC1);
      SET_TIMER(pulseWidth);
      pulsePhase = 3;
      return;
    } else if(pulsePhase == 3) {
      digitalClear(MC1);
      digitalClear(MC2);
      pulsePhase = 0;
      return;
    }
}

//----------- Setup ----------------

void setup(void) {	
	fruitInit();
//	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
//	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

// Analog setup ----------------
	analogInit();		// init analog module
//	analogSelect(0, MOTA_CURRENT);	// assign MotorA current sense to analog channel 0
//	analogSelect(1, MOTB_CURRENT);	// assign MotorB current sense to analog channel 1

// dcmotor setup ----------------

    /*dcmotorInit(C);
    DCMOTOR(C).Setting.onlyPositive = 0;
    dcmotorInit(D);
    DCMOTOR(D).Setting.onlyPositive = 0;*/
    digitalClear(MC1);
    digitalClear(MC2);
    digitalSet(MCEN);
    pinModeDigitalOut(MC1);
    pinModeDigitalOut(MC2);
    pinModeDigitalOut(MCEN);
    
    pulsePhase = 0;
    
    TIMER_INIT();			

}

// ---------- Main loop ------------

void loop() {
	fraiseService();	// listen to Fraise events
	//analogService();	// analog management routine
  //pulseService();
  
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		//analogSend();		// send analog channels that changed
		//DCMOTOR_COMPUTE(C,ASYM);
		//DCMOTOR_COMPUTE(D,ASYM);
	}
}


// ---------- Receiving ------------

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		//digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}


void fraiseReceive() // receive raw
{
	unsigned char c;
	c=fraiseGetChar();
	
	switch(c) {
	    PARAM_INT(10, pulseWidth); pulseFire(); break;
	    case 120 : DCMOTOR_INPUT(C) ; break;
	    case 121 : DCMOTOR_INPUT(D) ; break;
	}
}

