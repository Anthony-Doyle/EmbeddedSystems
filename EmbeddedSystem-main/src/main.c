
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

int sinusoidGen(float frequency,int max, int min,float time);

#include <stdio.h>
#include <stdint.h>
#define CIRC_BUF_SIZE1 6400
#define CIRC_BUF_SIZE2 128

typedef struct {
	int *data;
    uint32_t size;
	uint32_t head;
	uint32_t tail;
	uint32_t count;
} circular_buffer;

int buffer1_data[CIRC_BUF_SIZE1];
int buffer2_data[CIRC_BUF_SIZE2];

circular_buffer sample_buf1 = {
    .data = buffer1_data,   .size = CIRC_BUF_SIZE1,
    .head = 0,  .tail = 0,  .count =0
};
circular_buffer sample_buf2 = {
    .data = buffer2_data,   .size = CIRC_BUF_SIZE2,
    .head = 0,  .tail = 0,  .count =0
};

int filter(int input);

int FREQTIME=0;
int fs=50*100;//sample rate
int sinusoid=0;

float b[]={0.0010174,0.0013355,0.0018988,0.0027968,0.0041074,0.0058912,0.0081857,0.011001,0.014318,0.018085,0.022223,0.026622,0.031153,0.035667,0.040007,0.044014,0.047536,0.050435,0.052594,0.053926,0.054376,0.053926,0.052594,0.050435,0.047536,0.044014,0.040007,0.035667,0.031153,0.026622,0.022223,0.018085,0.014318,0.011001,0.0081857,0.0058912,0.0041074,0.0027968,0.0018988,0.0013355,0.0010174};// B and A coefficents of butter filter from matlab
float a[]={1};
float x[45]={0};//Empty arrays on inital upload used to store previous values for difference equation
float y[45]={0};

int main()
{
    setup();
    float frequency=0;
    float previous=0; 
    int max=0; int min=0;
    static float t =0;
    int changedirection=1; //Keeps value when ran again

    SysTick->LOAD = (SystemCoreClock/fs)-1; // Systick clock = 80MHz. 80000000/5000 = 16000//sample rate is 16000 counts at a speed of 80MHz

	SysTick->CTRL = 7; // enable systick counter and its interrupts
	SysTick->VAL = 10; // start from a low number so we don't wait for ages for first interrupt
	__asm(" cpsie i "); // enable interrupts globally

    int sample;
    int processed;
    while(1)
    {
        
        if (get_circ_buf(&sample_buf1, &sample) == 0) {// process the buffer if buffer is not empty

            processed = filter(sample);
            if(processed>previous&&changedirection==-1){//filtered value is greater then previous value //if new min value doesnt equal the old min value and direction was decreasing  
                    min=previous;
                    changedirection=1;
                    frequency=1/(FREQTIME/16000); //16000 is the ticks between measurments
                    FREQTIME=0;
                    //Min is reset and prepare to calculate max
            }   
            else if(processed<previous&&changedirection==1){//filtered value is less then previous value //if previous was growing assume max was found
                max=previous;       // if direction was growing and is now decreasing we can assumine max was just reached 
                changedirection=-1; //Sets expected slope of sinusoid
                frequency=1/(FREQTIME/16000);//Time since last max/min is turned to a frequency
                FREQTIME=0; //Time is reset
                //Max is reset and prepare to calculate min
            }
            previous=processed;
            if(frequency<20.1){
                frequency=20;
            }
            
            
        }

        FREQTIME=FREQTIME+1;
        sinusoid=sinusoidGen(2*frequency,max,min,t); //Frequency is times 2 due to being reset at max an min
        writeDAC(sinusoid);  
        t = t+ (1.0f)/fs;
    

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
    put_circ_buf(&sample_buf1, vin); // Store sample even if processor is busy
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
    int f=0;
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
    buf->count=0;
	buf->head=0;
	buf->tail=0;
}
int put_circ_buf(circular_buffer *buf,int c)
{
	if (buf->count < buf->size)
	{
		buf->data[buf->head]=c;
		buf->head=((buf->head)+1)%buf->size;
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
	if (buf->count > 0)
	{
		*c=buf->data[buf->tail];
		buf->tail=(((buf->tail)+1)%buf->size);
		buf->count--;
		return 0;	
	}
	else
	{
		return -1;
	}
}

//New Code from project 2
//Buffer was updated to be more dynamic as a second buffer will be used


int sinusoidGen(float frequency, int max,int min, float time){
    float scale=(max-min)/2;
    float offset=(max+min)/2;

    return offset+scale*sinf(2*3.14159*frequency*time); // offset+scale*sin(2*pi*f*t)
}
