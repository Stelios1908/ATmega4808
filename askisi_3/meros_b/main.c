
#include <avr/io.h>
#include <avr/interrupt.h>

#define FREQ_BASE 255
#define DUTY_BASE FREQ_BASE*0.40

#define FREQ_LEPIDES 127
#define DUTY_LEPIDES FREQ_LEPIDES/2
int fun_on_off=0;
//-------------------sinarthseis----------------------------------
void init_timer_base(void){
	TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1_gc;//xoris prescaler
	TCA0.SINGLE.PER=FREQ_BASE;
	TCA0.SINGLE.CMP0=DUTY_BASE;
	//RITHMISI MODE
	TCA0.SINGLE.CTRLB=TCA_SINGLE_CMP0EN_bm|TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	//ENABLE INTERRUPR
	TCA0.SINGLE.INTCTRL=TCA_SINGLE_OVF_bm;
	//TCA0.SINGLE.INTCTRL|=TCA_SINGLE_CMP0_bm;
	TCA0.SINGLE.CNT=0;
	//H EXODOS PWM THA BGENEI STO PORTC
	PORTMUX.TCAROUTEA |=PORTMUX_TCA0_PORTC_gc;
	//Disable kai ta dio PWM
	TCA0.SINGLE.CTRLA &=0x0;
	TCB0.CTRLA &=0x0;
	
}
void init_timer_lepides(void){
	//ENERGOPIHSI GIA PWM
	TCB0.CTRLA = (TCB0.CTRLA & ~TCB_CLKSEL_gm) | TCB_CLKSEL_CLKDIV1_gc;
	TCB0.CTRLB |= TCB_CCMPEN_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB0_CCMPL=	FREQ_LEPIDES;
	TCB0_CCMPH= DUTY_LEPIDES;
	TCB0.CNT=50;
	//H EXODOS NA BGENEI STO PORTA
	PORTMUX.TCBROUTEA |=0x0;
	TCB0.INTCTRL|=TCB_CAPT_bm;
}
void init_ADC(void){
	//DEXOMASTE ANALOGIKH EISIDO APO PIN7
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	//OTAN XEPRASO TO KATOFLI NA KANO INTERRUPT
	ADC0.INTCTRL |= ADC_WCMP_bm;
	// IFREE RUNNING MODE
	ADC0.CTRLA |= ADC_FREERUN_bm;
	//ORIO KATOFLI
	ADC0.WINLT |= 10;
	//OTAN RES < KATOFLI
	ADC0.CTRLE |= ADC_WINCM0_bm;
	
	}

int main(void)
{
      //epitrepsi prosvasis se eidikoys kataxorites
      CPU_CCP = CCP_IOREG_gc;
      //syxnothtia sta 32 KHh
      CLKCTRL.MCLKCTRLA=CLKCTRL_CLKSEL_OSCULP32K_gc;
      //pali theloyme prosvasi
       CPU_CCP = CCP_IOREG_gc;
      //autpo xreiazete mono gia test,an to ektelesoyme
      //se ATmega den theloyme prescaler
      CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm;
	
	//ELENXOS AN TO ROLOI EINAI STATHERO
	while (!(CLKCTRL.MCLKSTATUS & 0x20))
	{
		;
	}
	
	
	//Led gia sixnothta kai ADC
	PORTD.DIR  |= PIN0_bm|PIN1_bm|PIN2_bm;
	//svista ola
	PORTD.OUT |= PIN0_bm|PIN1_bm|PIN2_bm;
	//Exodoi gia pwm palmo
	PORTC.DIR  |= PIN0_bm;
	PORTA.DIR  |= PIN2_bm;
	

	//arxiopihsi se ola
	init_timer_base();
	init_timer_lepides();
	init_ADC();
	
	//on off switch mono se kathodoki akmi
	PORTF.PIN5CTRL |=PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
	
	sei();
	while (1)
	{
		PORTE.OUT|= 0b00000000;
	}
	cli();
}
//----------------------ISR------------------------------
ISR(PORTF_PORT_vect){
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	if(!fun_on_off){
		//prota afoy anavoyme energopoioyme ton ADC 
		ADC0.CTRLA |= ADC_ENABLE_bm;
		ADC0.COMMAND |= ADC_STCONV_bm;
		//meta energopioyme ta PWM
		TCA0.SINGLE.CNT=10;
		TCA0.SINGLE.CTRLA |=TCA_SINGLE_ENABLE_bm;
		TCB0_CNT=50;
		TCB0.CTRLA |=TCB_ENABLE_bm;
		fun_on_off=1;
		//svino pithano anameno to led ADC
		PORTD.OUT |=PIN2_bm; 
		
	}
	else{
		TCA0.SINGLE.CTRLA &=0x0;
		TCB0.CTRLA &=0x0;
		//sbino ta led
		PORTD.OUT |=0x03;
		//disable ADC
		ADC0.CTRLA &=0x02;
		fun_on_off=0;
	}
}

ISR(TCA0_OVF_vect){//gia otan exoyme ena pliri kyklo
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	//led on off
	PORTD.OUTTGL=PIN1_bm;
}

ISR(TCA0_CMP0_vect){//otan teleivsei to duty cycle
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
}

ISR(TCB0_INT_vect)//lepides otan exoyme ena pliri kyklo
{
	
		int intflags = TCB0.INTFLAGS;
	TCB0.INTFLAGS = intflags;
	//led on off
	PORTD.OUTTGL =PIN0_bm;
}
ISR(ADC0_WCOMP_vect) {	//interrupt ??? ??? ADC
		//katharizo  tis simaies gia tous metrites giati
		//thelo na apofoigo na kanei interrupt meta apo edo
		int intflags_1 = TCA0.SINGLE.INTFLAGS;
	    TCA0.SINGLE.INTFLAGS = intflags_1;
		int intflags_2 = TCB0.INTFLAGS;
		TCB0.INTFLAGS = intflags_2;
		
		//apenergopoio PWM
		TCA0.SINGLE.CTRLA &=0x0;
		TCB0.CTRLA &=0x0;
		//disable ADC
		ADC0.CTRLA &=0x02;
		int intflags = ADC0.INTFLAGS;
	    ADC0.INTFLAGS = intflags;
		//led ADC ON
	    PORTD.OUTCLR= 0b00000100;
    	//led kinisis anemistiron off
    	PORTD.OUT |= PIN0_bm|PIN1_bm;
	    //gia na xero oti eimai off
	    fun_on_off=0;
		
		
		 
	 }
