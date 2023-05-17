#include <avr/io.h>
#include <avr/interrupt.h>

#define PERIOD 255
#define DUTY_CYCLE	PERIOD/2

#define ENTRY_TIME 1000
#define EXIT_TIME 400
#define THRESS 30
volatile int ARMED=0;
int digit_code=1;
int tries = 3;
int suc_code=0;
int test=1;
int timer=1;
int alarm=0;
int falsecode=0;

void init_timer_entry_exit(int time){
	TCA0.SINGLE.CTRLB =0x00;
	//xekiano metrisi apo 0
	TCA0.SINGLE.CNT = 0x00;
	//interrypt otan ftaso timi cmp
	TCA0.SINGLE.CMP0= time;
	
	//interrupt gia otan ftasv tini cmp
	TCA0.SINGLE.INTCTRL=TCA_SINGLE_CMP0_bm;
	//den kano dieresi sixnotitas/xekianao metrisi
	TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV256_gc|TCA_SINGLE_ENABLE_bm;
	timer=1;
}
void init_timer_PWM_siren(void){
	//ENERGOPIHSI GIA PWM
	//
	TCB0.CTRLA = (TCB0.CTRLA & ~TCB_CLKSEL_gm) | TCB_CLKSEL_CLKDIV1_gc;
	TCB0.CTRLB |= TCB_CCMPEN_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB0_CCMPL=	PERIOD;
	TCB0_CCMPH= DUTY_CYCLE;
	//xekinav apo 0
	TCB0.CNT=0;
	//H EXODOS NA BGENEI STO PORTA
	PORTMUX.TCBROUTEA |=0x0;
	//energopoihsh diakopon
	TCB0.INTCTRL|=TCB_CAPT_bm;
	TCB0.CTRLA |=TCB_ENABLE_bm;
}

void ADC_init(void){
	//resolution 10 bit
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;
	//analogiki eisodo apo pin 7 
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	//energopioisi interrupt
	ADC0.INTCTRL |= ADC_WCMP_bm;
	//synexeia epopteyei
	ADC0.CTRLA |= ADC_FREERUN_bm;
	//katofli
	ADC0.WINLT |= THRESS; 
	
	//otan res<thress
	ADC0.CTRLE |= ADC_WINCM0_bm;
	//ADC0.RES    =0xff;
	//xejinaei
	ADC0.CTRLA |= ADC_ENABLE_bm;
	ADC0.COMMAND |= ADC_STCONV_bm;
	//alarm=0;
} 

int main(){
	 //orizo ta led poy tha exo kai ola kleista gia arxi
	 PORTA.DIR |=PIN0_bm|PIN1_bm|PIN2_bm;
	 PORTA.OUT |=PIN0_bm;
	 PORTD.DIR |=PIN0_bm|PIN1_bm|PIN2_bm;
	 PORTD.OUT |=PIN0_bm|PIN1_bm|PIN2_bm;
	//energopoio ta dyo switch
	PORTF.PIN5CTRL |=PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
	PORTF.PIN6CTRL |=PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
	
	
	sei();
	while(1){
			if (!ARMED)
			{
				while(!suc_code){}
				   
			    //pleon exoyme balei sosto kodiko kai pame na figoyme 
				//apo ton xoro.
				//arxikopio metriti gia xrono exodoy
				init_timer_entry_exit(EXIT_TIME);
			   
			   //edo menoyme oso xrono xreiazete to systhma na oplisei
			   //afoy exoyme valei kodiko :XRONOS EXODOY
			   while(timer){}
			   
		     }
			if(ARMED){
				//xekianei o aisthitiras kinisis meso toy ADC
				ADC_init();
				
				//edo menoyme oso to systhma einai oplismeno alla  
				//den xtypaei synagermos,o esthitiras den pianei kati. 
				while(!alarm){}
					
				//se ayth thn if btha mpoyme an eno eimaste aplismenoi exoyme pathsei 
				//3 fores lathos ton kodiko,den tha xreiastei na metrisoyme xrono eisodoy. 
				if(falsecode==0){	
				 //xekinaei o xronos eisodoy	
			      init_timer_entry_exit(ENTRY_TIME);
				  
				 //edo menoyme oso metrane ta sec gia eisodo efoson epoiase o aisthitiras.
				 //an omos exoyme balei 3 fores lathos kodiko tha xekolisoyme apo tin
				 //whilw kai akariaia tha xtipisei alarm
				while(timer){}
				 if(!ARMED){
					 alarm=0;	
				   }
				}
				if(ARMED){
			    	//exoyme synagermo den apenergopihthike kata ton xrono eisodoy
					//energopoioyme thn sirina
			    	init_timer_PWM_siren();
				}
			    while(alarm){}//tha mino edo oso xtyaei o sinagermos
				//stamatao na parago PWM opote kai h sirina	
				TCB0.CTRLA &=0x0;
				falsecode=0;
				suc_code=0;
				ARMED=0;
				//PREPEI NA SVISO LED SIRINA
			    PORTD.OUT |=0B00000001;
			}
 }
		 cli();
}
// Interrupt handler for TCA0 
ISR(TCA0_CMP0_vect) {
	//Disable TCA0
	TCA0.SINGLE.CTRLA &= 0xFE;
	//Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	timer=0;
	if (!ARMED )
	    ARMED=1; //oplizo to systhma	
}
ISR(PORTF_PORT_vect) {
	
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	if(!(intflags ^ 0b01100000))
	{
		if(ARMED | suc_code) {
			//meiononte oi prospathies mono an eimaste armed
			tries--;
			if(tries==0){
				alarm=1;
				falsecode=1;
				timer=0;
				ARMED=1;
	    		}
	      }
		  digit_code = 1;
		  return;
	}
	
	if (intflags & PIN5_bm) {//otan patithei to pin 5
		if (digit_code == 1 || digit_code == 3)
		{
			digit_code++;
		} 
		else {
			if(ARMED | suc_code) { 
				//meiononte oi prospathies mono an eimaste armed
				      tries--; 
				      if(tries==0){
					      alarm=1;
					      falsecode=1;
						  timer=0;
						  ARMED=1;
				      }
				//afoy egine lathos pame pali apo to 1
				
				}
			//se kathe periptosi me lathos kodiko pame pali sto proto psifio
			digit_code = 1;
			
	   }
}
	if ( intflags & PIN6_bm) {//otasn patithei to 6
		if (digit_code == 2) {
			digit_code++;
			}
	   else if(digit_code == 4) {
		   //edo tha mpo otan exo valei sosto kodiko
		   //opote oi prospathies tha ginoyn pali 3 
		   //kai tha pame sto proto psifio kodikoy gia thn epomenh fora
			 if(suc_code==0)
			 {suc_code=1;}
			 else{ suc_code=0;}
			if(ARMED)ARMED=0;	 
			digit_code = 1;
			tries = 3;
			timer=0;
			alarm=0;
			}
			else{ 
				  //edo tha mpoyme an to systima einai oplismeno
				  if(ARMED | suc_code){
					   tries--;
					   if(tries==0){
					     alarm=1;
						 falsecode=1;
						 timer=0;
						 ARMED=1;
					   }
				  }
				  digit_code=1;
				   
			}
	    }
    
}
ISR(ADC0_WCOMP_vect) {
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	TCA0.SINGLE.CNT = 0;
	alarm=1;
	
	//TCA0.SINGLE.CTRLA |= 1;
	PORTD.OUT  |= PIN0_bm;
	ADC0.CTRLA &=0xfe;
}
ISR(TCB0_INT_vect)
{
	
	int intflags = TCB0.INTFLAGS;
	TCB0.INTFLAGS = intflags;
	//led on off
	PORTD.OUTTGL =PIN0_bm;
}
//------------------------------------------
