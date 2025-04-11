
#include <eeng1030_lib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

void setup(void);
void delay(volatile uint32_t dly);
void initADC();
int readADC();
void initDAC();
void writeDAC(int value);
void initSerial(uint32_t baudrate);
int max(int processed);
int min(int processed);
int sinusoidGen(int frequency);

#include <stdio.h>
#include <stdint.h>
#define CIRC_BUF_SIZE 64

typedef struct {
	int data[CIRC_BUF_SIZE];
	uint32_t head;
	uint32_t tail;
	uint32_t count;
} circular_buffer;
circular_buffer sample_buf;

int filter(int input);

int FREQTIME=0;
int fs=50*100;//sample rate

float b[]={0.0010174,0.0013355,0.0018988,0.0027968,0.0041074,0.0058912,0.0081857,0.011001,0.014318,0.018085,0.022223,0.026622,0.031153,0.035667,0.040007,0.044014,0.047536,0.050435,0.052594,0.053926,0.054376,0.053926,0.052594,0.050435,0.047536,0.044014,0.040007,0.035667,0.031153,0.026622,0.022223,0.018085,0.014318,0.011001,0.0081857,0.0058912,0.0041074,0.0027968,0.0018988,0.0013355,0.0010174};// B and A coefficents of butter filter from matlab
float a[]={1};
float x[45]={0};//Empty arrays on inital upload used to store previous values for difference equation
float y[45]={0};

int main()
{
    setup();
    //int frequency=0;
    //int previous=0;
    SysTick->LOAD = (SystemCoreClock/fs)-1; // Systick clock = 80MHz. 80000000/50000 = 1600//sample rate is 1600 counts at a spead of 80MHz

	SysTick->CTRL = 7; // enable systick counter and its interrupts
	SysTick->VAL = 10; // start from a low number so we don't wait for ages for first interrupt
	__asm(" cpsie i "); // enable interrupts globally
    while(1)
    {
        int sample;
        
        if (get_circ_buf(&sample_buf, &sample) == 0) {// process the buffer if buffer is not empty

            int processed = filter(sample);
            //int changedirection=0;
            //if(processed>previous){//filtered value is greater then previous value
                //max(processed)
              //  if(changedirection==-1){//if previous was shrinking assume min was found
                //  changedirection=1;
                //  frequency=1/(FREQTIME/(SysTick->LOAD));
                //  FREQTIME=0;
                //}
            //}   
            //else if(processed<previous){//filtered value is less then previous value
                //min(processed)
                // if(changedirection==1){//if previous was growing assume man was found
                //  changedirection=-1;
                //  frequency=1/(FREQTIME/(SysTick->LOAD));
                //  FREQTIME=0;  
                //}
            //}
            //previous=processed;
            //int sinusoid=sinusoidGen(frequency);
            writeDAC(processed);
        }

        // 
    }
}

void delay(volatile uint32_t dly)
{
    while(dly--);
}
void setup()
{
    initClocks();
    RCC->AHB2ENR |= (1 << 0) + (1 << 1); // enable GPIOA and GPIOB
    pinMode(GPIOB,3,1); // make PB3 an output.
    pinMode(GPIOA,0,1);
    pinMode(GPIOA,1,3);  // PA = analog mode (ADC in)
    pinMode(GPIOA,4,3);  // PA4 = analog mode (DAC out)
    initADC();
    initDAC();
    initSerial(9600);                       // Initialise serial comms
    
}

void SysTick_Handler(void) {
    int vin = readADC();
    put_circ_buf(&sample_buf, vin); // Store sample even if processor is busy
    //FREQTIME=FREQTIME+1;
}

void initADC()
{
    // initialize the ADC
    RCC->AHB2ENR |= (1 << 13); // enable the ADC
    RCC->CCIPR |= (1 << 29) | (1 << 28); // select system clock for ADC
    ADC1_COMMON->CCR = ((0b0000) << 18) + (1 << 22) ; // set ADC clock = HCLK and turn on the voltage reference
    // start ADC calibration    
    ADC1->CR=(1 << 28); // turn on the ADC voltage regulator and disable the ADC
    delay(100); // wait for voltage regulator to stabilize (20 microseconds according to the datasheet).  This gives about 180microseconds
    ADC1->CR |= (1<< 31);
    while(ADC1->CR & (1 << 31)); // wait for calibration to finish.
    ADC1->CFGR = (1 << 31); // disable injection

    ADC1->SQR1 |= (6 << 6);
     ADC1->CR |= (1 << 0); // enable the ADC
    while ( (ADC1->ISR & (1 <<0))==0); // wait for ADC to be ready
}
int readADC()
{

    int rvalue=ADC1->DR; // get the result from the previous conversion    
    ADC1->ISR = (1 << 3); // clear EOS flag
    ADC1->CR |= (1 << 2); // start next conversion    
    return rvalue; // return the result
}
void initDAC()
{

    RCC->APB1ENR1 |= (1 << 29);   // Enable the DAC
    RCC->APB1RSTR1 &= ~(1 << 29); // Take DAC out of reset
    DAC->CR &= ~(1 << 0);         // Enable = 0
    DAC->CR |= (1 << 0);          // Enable = 1
}
void writeDAC(int value)
{
    DAC->DHR12R1 = value;
}

int filter(int input){
    for (int i=41;i>0;i--){ //shifts old numbers
        x[i]=x[i-1];
        y[i]=y[i-1];
    }
    double f;
    int i;
    x[0]=input;// gets new input
for (i=42;i>-1;i--){
      f = f + x[i]*b[i] ;// gets new output
}
    y[0]=f;
    return y[0];
}

void initSerial(uint32_t baudrate)
{
    RCC->AHB2ENR |= (1 << 0); // make sure GPIOA is turned on
    pinMode(GPIOA,2,2); // alternate function mode for PA2
    selectAlternateFunction(GPIOA,2,7); // AF7 = USART2 TX

    RCC->APB1ENR1 |= (1 << 17); // turn on USART2

	const uint32_t CLOCK_SPEED=SystemCoreClock;    
	uint32_t BaudRateDivisor;
	
	BaudRateDivisor = CLOCK_SPEED/baudrate;	
	USART2->CR1 = 0;
	USART2->CR2 = 0;
	USART2->CR3 = (1 << 12); // disable over-run errors
	USART2->BRR = BaudRateDivisor;
	USART2->CR1 =  (1 << 3);  // enable the transmitter
	USART2->CR1 |= (1 << 0);
}

void init_circ_buf(circular_buffer *buf)
{
	buf->head=0;
	buf->tail=0;
}
int put_circ_buf(circular_buffer *buf,int c)
{
	uint32_t new_head;
	if (buf->count < CIRC_BUF_SIZE)
	{
		new_head=(((buf->head)+1)%CIRC_BUF_SIZE);
		buf->data[buf->head]=c;
		buf->head=new_head;
		buf->count++;
		return 0;	
	}
	else
	{
		return -1;
	}
}
int get_circ_buf(circular_buffer *buf,int *c)
{
	uint32_t new_tail;
	if (buf->count > 0)
	{
		new_tail=(((buf->tail)+1)%CIRC_BUF_SIZE);
		*c=buf->data[buf->tail];
		buf->data[buf->tail] = '-'; //debug
		buf->tail=new_tail;
		buf->count--;
		return 0;	
	}
	else
	{
		return -1;
	}
}

//New Code from project 2

int max(int newValue){
    static int oldValue=0;

    if(newValue>oldValue){// input is growing/ tending towards max
    }
    else if(newValue<oldValue){// input is growing/ tending towards min
    }
    return 0;
}

int min(int x){
    return 0;
}

int sinusoidGen(int frequency){
    return sin(2*3.14159*frequency);
}