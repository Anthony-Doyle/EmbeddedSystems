#include <stdint.h>
#include <stm32l432xx.h>
#include <Functions.h>
#include <stdio.h>

void initClocks()
{
	// Initialize the clock system to a higher speed.
	// At boot time, the clock is derived from the MSI clock 
	// which defaults to 4MHz.  Will set it to 80MHz
	// See chapter 6 of the reference manual (RM0393)
	    RCC->CR &= ~(1 << 24); // Make sure PLL is off
	
	// PLL Input clock = MSI so BIT1 = 1, BIT 0 = 0
	// PLLM = Divisor for input clock : set = 1 so BIT6,5,4 = 0
	// PLL-VCO speed = PLL_N x PLL Input clock
	// This must be < 344MHz
	// PLL Input clock = 4MHz from MSI
	// PLL_N can range from 8 to 86.  
	// Will use 80 for PLL_N as 80 * 4 = 320MHz
	// Put value 80 into bits 14:8 (being sure to clear bits as necessary)
	// PLLSAI3 : Serial audio interface : not using leave BIT16 = 0
	// PLLP : Must pick a value that divides 320MHz down to <= 80MHz
	// If BIT17 = 1 then divisor is 17; 320/17 = 18.82MHz : ok (PLLP used by SAI)
	// PLLQEN : Don't need this so set BIT20 = 0
	// PLLQ : Must divide 320 down to value <=80MHz.  
	// Set BIT22,21 to 1 to get a divisor of 8 : ok
	// PLLREN : This enables the PLLCLK output of the PLL
	// I think we need this so set to 1. BIT24 = 1 
	// PLLR : Pick a value that divides 320 down to <= 80MHz
	// Choose 4 to give an 80MHz output.  
	// BIT26 = 0; BIT25 = 1
	// All other bits reserved and zero at reset
	    RCC->PLLCFGR = (1 << 25) + (1 << 24) + (1 << 22) + (1 << 21) + (1 << 17) + (80 << 8) + (1 << 0);	
	    RCC->CR |= (1 << 24); // Turn PLL on
	    while( (RCC->CR & (1 << 25))== 0); // Wait for PLL to be ready
	// configure flash for 4 wait states (required at 80MHz)
	    FLASH->ACR &= ~((1 << 2)+ (1 << 1) + (1 << 0));
	    FLASH->ACR |= (1 << 2); 
	    RCC->CFGR |= (1 << 1)+(1 << 0); // Select PLL as system clock
}
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber)
{
	Port->PUPDR = Port->PUPDR &~(3u << BitNumber*2); // clear pull-up resistor bits
	Port->PUPDR = Port->PUPDR | (1u << BitNumber*2); // set pull-up bit
}
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode)
{
	/*
        Modes : 00 = input
                01 = output
                10 = special function
                11 = analog mode
	*/
	uint32_t mode_value = Port->MODER;
	Mode = Mode << (2 * BitNumber);
	mode_value = mode_value & ~(3u << (BitNumber * 2));
	mode_value = mode_value | Mode;
	Port->MODER = mode_value;
}
void selectAlternateFunction (GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t AF)
{
    // The alternative function control is spread across two 32 bit registers AFR[0] and AFR[1]
    // There are 4 bits for each port bit.
    if (BitNumber < 8)
    {
        Port->AFR[0] &= ~(0x0f << (4*BitNumber));
        Port->AFR[0] |= (AF << (4*BitNumber));
    }
    else
    {
        BitNumber = BitNumber - 8;
        Port->AFR[1] &= ~(0x0f << (4*BitNumber));
        Port->AFR[1] |= (AF << (4*BitNumber));
    }
}

// System
void delay(volatile uint32_t dly)
{
    while(dly--);
}
void setup()
{
    initClocks();
    RCC->AHB2ENR |= (1 << 0) + (1 << 1); // enable GPIOA and GPIOB
    pinMode(GPIOB,3,1); // make PB3 an output.
    pinMode(GPIOA,1,3);  // PA1 = analog mode (ADC in)
    pinMode(GPIOA,4,3);  // PA4 = analog mode (DAC out)
    initADC();
    initDAC();
    SystemCoreClock=80000000;
    initSerial(9600);
}
void initSerial(uint32_t baudrate)
{
    RCC->AHB2ENR |= (1 << 0); // make sure GPIOA is turned on
    pinMode(GPIOA,2,2); // alternate function mode for PA2
    selectAlternateFunction(GPIOA,2,7); // AF7 = USART2 TX

    pinMode(GPIOA,15,2); // alternate function mode for PA15
    selectAlternateFunction(GPIOA,15,3); // AF3 = USART2 RX


    RCC->APB1ENR1 |= (1 << 17); // turn on USART2

	const uint32_t CLOCK_SPEED=80000000;    
	uint32_t BaudRateDivisor;
	
	BaudRateDivisor = CLOCK_SPEED/baudrate;	
	USART2->CR1 = 0;
	USART2->CR2 = 0;
	USART2->CR3 = (1 << 12); // disable over-run errors
	USART2->BRR = BaudRateDivisor;
	USART2->CR1 =  (1 << 3) | (1 << 2);  // enable the transmitter and receiver
    USART2->CR1 |= (1 << 5); // enable receiver interrupts
    USART2->CR3 = (1 << 12); // disable over-run errors
	USART2->CR1 |= (1 << 0); // enable the UART
    USART2->ICR = (1 << 1); // clear any old framing errors
    
}

//Read and write
//
void initADC()
{
    // initialize the ADC
    RCC->AHB2ENR |= (1 << 13); // enable the ADC
    RCC->CCIPR |= (1 << 29) | (1 << 28); // select system clock for ADC
    ADC1_COMMON->CCR = ((0b0000) << 18) + (1 << 22); // set ADC clock = HCLK and turn on the voltage reference
    // start ADC calibration    
    ADC1->CR=(1 << 28); // turn on the ADC voltage regulator and disable the ADC
    delay(100); // wait for voltage regulator to stabilize (20 microseconds according to the datasheet).  This gives about 180microseconds
    ADC1->CR |= (1<< 31);
    while(ADC1->CR & (1 << 31)); // wait for calibration to finish.
    ADC1->CFGR = (1 << 31); // disable injection
    ADC1_COMMON->CCR |= (0x07 << 18);
    ADC1->SQR1 |= (6 << 6);
     ADC1->CR |= (1 << 0); // enable the ADC
    while ( (ADC1->ISR & (1 <<0))==0); // wait for ADC to be ready
}
int readADC(int chan)
{
    ADC1->SQR1 &= ~(0x1F << 6);       // Clear bits 6–10 (channel select)
    ADC1->SQR1 |= (chan << 6);
    ADC1->ISR =  (1 << 3); // clear EOS flag
    ADC1->CR |= (1 << 0); // enable the ADC
    while ( (ADC1->ISR & (1 <<0))==0); // wait for ADC to be ready
    ADC1->CR |= (1 << 2); // start conversion
    while ( (ADC1->ISR & (1 <<3))==0); // wait for conversion to finish
    ADC1->CR &= ~(1 << 0); // disable the ADC
    return ADC1->DR; // return the result
}
void initDAC()
{
    // Configure PA4 as a DAC output
    RCC->AHB2ENR |= (1 << 0);     // ensure GPIOA is enabled    
    GPIOA->MODER |= (1 << 8) | (1 << 9); // Set mode to analogue (DAC) 
    RCC->APB1ENR1 |= (1 << 29);   // Enable the DAC
    RCC->APB1RSTR1 &= ~(1 << 29); // Take DAC out of reset
    DAC->CR &= ~(1 << 0);         // Enable = 0
    DAC->CR |= (1 << 0);          // Enable = 1
}
void writeDAC(int value)
{
    DAC->DHR12R1 = value;
}

//Sinusoid gens
float Phaseshift(){//Phase shift averages the last 5 values of the ADC channel 8 and converts it to degrees
    int vin1 = readADC(8); //Read ADC value from channel 8
    // static int phz[3]={0};
    // for (int i=2;i>0;i--){ //shifts old numbers
    //     phz[i]=phz[i-1];
    // }
    // phz[0]=(((vin1-15)*2*3.14159)/4050)-3.14159; //Convert to pi
    float phase=(((vin1-15)*2*3.14159)/4050)-3.14159; //Convert to pi
    // vin1=0; //Reset vin to 0
    // for (int i=0;i<3;i++){
    //     vin1=vin1+phz[i]; //Sum the values to get the average
    // }
    // vin1=vin1/3; //Average the values
    //readADC(6); //Read ADC value from channel 8
    //printf("ADC %d \n",vin1); //debug tool
    //delay(10); //Delay to allow for reading of the serial port
    readADC(6); //Read ADC value from channel 8
    return phase;
}
float FrequencyAverage(float frequency){ //Frequency average averages the last 5 values of the frequency and converts it to degrees
    static float freq[40]={50};    //
    //static float oldfreq=50.0f; //Frequency is set to 0 
    static float filtered_frequency = 50.0f; // start reasonable 
    //const float alpha = 0.1f; // Smoothing factor (0.0 -> very smooth, 1.0 -> no smooth)
    for (int i=39;i>0;i--){ //shifts old numbers
        freq[i]=freq[i-1];
    }
    freq[0]=frequency*1.008; //Convert to degrees
    frequency=0; //Reset frequency to 0
    for (int i=0;i<40;i++){
        frequency=frequency+freq[i]; //Sum the values to get the average
    }
    filtered_frequency=frequency/40; //Average the values
    // if(oldfreq>frequency*2||oldfreq<frequency*2){ //If the frequency is lower than the old frequency

    //     filtered_frequency = alpha * (frequency+0.10f) + (1.0f - alpha) * filtered_frequency;
    // }
    // oldfreq=frequency; //Set the old frequency to the new frequency

    return filtered_frequency;
}