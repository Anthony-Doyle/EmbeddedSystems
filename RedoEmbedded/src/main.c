#include <Functions.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include  <errno.h>
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILEN
volatile uint32_t data_ready=0;

//Varibles
int fs=50*100;//sample 50hz wanted, 1000 samples per second
    //Sinwave
    int FREQTIME=0;//Time measured to calculate frequency
    int sinusoid=0; //value of sinwave to be put through DAC
    static float t =0;//Time measurment for sinwave
    int phaseshift;//phaseshift of sinewave
    int frequency=0;
    int max=0; int min=0;//Previous max or min of the sinusoid 
    float pi=3.14159265358979323846; //Pi value
//
int changedirection=1; //Desided whether slope is up or down
int sample; //Info read from buffer
int processed; //After Filter
int previous=0; // Previous value of processed
int phaseclk=0; //Clock used to measure time for the sinusoid

//Arrays for filter
float b[]={0.047898,0,-0.047898};
float a[]={1,-1.5425,0.9042};
float x[3]={0};//Empty arrays on inital upload used to store previous values for difference equation
float y[3]={0};
//Buffer
#define CIRC_BUF_SIZE1 64
#define CIRC_BUF_SIZE2 64
typedef struct {//Buffer structure and varibles
	int *data;
    uint32_t size;
	uint32_t head;
	uint32_t tail;
	uint32_t count;
} circular_buffer;
int buffer1_data[CIRC_BUF_SIZE1];//amount of bytes buffer can hold
int buffer2_data[CIRC_BUF_SIZE2];//amount of bytes buffer can hold
circular_buffer sample_buf1 = {//Buffer default state
    .data = buffer1_data,   .size = CIRC_BUF_SIZE1,
    .head = 0,  .tail = 0,  .count =0
};
circular_buffer sample_buf2 = {//Buffer default state
    .data = buffer2_data,   .size = CIRC_BUF_SIZE2,
    .head = 0,  .tail = 0,  .count =0
};

//Project functions
int filter(int input);
int sinusoidGen(int frequency,int max, int min,float time, int phase);
int put_circ_buf(circular_buffer *buf ,int c);
int get_circ_buf(circular_buffer *buf ,int *c);



//Main Function
int main(){
    setup(); //Sets up the needed fuction before completing the rest of the code
    //Sets up the systick interrupt
    SysTick->LOAD = (SystemCoreClock/fs)-1; // Systick clock = 80MHz. 80000000/(50*1000)-1 = 1599
	SysTick->CTRL = 7; // enable systick counter and its interrupts
	SysTick->VAL = 10; // start from a low number so we don't wait for ages for first interrupt
	__asm(" cpsie i "); // enable interrupts globally
    //printf("SineWave = [");
    //delay(1000); //Delay to allow for reading of the serial port
    //Repeat until code is ended
    while(1)
    {
        if (get_circ_buf(&sample_buf1, &sample) == 0) {// process the buffer if buffer is not empty
            //printf("Sample %d \n",sample); //debug tool
            //processed = filter(sample);// Not working atm
            if(sample>previous&&changedirection==-1){//filtered value is greater then previous value //if new min value doesnt equal the old min value and direction was decreasing  
                    min=previous;
                    changedirection=1;
                    //Min is reset and prepare to calculate max
            }   
            else if(sample<previous&&changedirection==1){//filtered value is less then previous value //if previous was growing assume max was found
                frequency=fs/(FREQTIME);//samples per second / samples since last max = per second = frequency
                frequency=FrequencyAverage(frequency)-3; //Average the frequency over 5 samples
                max=previous;       // if direction was growing and is now decreasing we can assumine max was just reached 
                changedirection=-1; //Sets expected slope of sinusoid
                
                //printf("Max %d \n",max); //debug tool
                //delay(10); //Delay to allow for reading of the serial port
                FREQTIME=0; //Time is reset
            }
            previous=sample;
        }
        if(phaseclk > 500){ //Every 100ms
            //printf("P"); //debug tool
            //delay(100); //Delay to allow for reading of the serial port
            phaseshift=Phaseshift();
            phaseclk=0; //Reset clock
        }

        //printf("FreqTime %d \t Frequency %d \n",FREQTIME,frequency); //debug tool
        //delay(10); //Delay to allow for reading of the serial port

        //sinusoid=sinusoidGen(frequency,max,min,t,phaseshift); //Frequency is times 2 due to being reset at max an min
        //writeDAC(sinusoid);  
    }
}

//Interrupts
void SysTick_Handler(void)
{    
    sinusoid=sinusoidGen(50,max,min,t,phaseshift);
    writeDAC(sinusoid);

    int vin = readADC(6); //Read ADC value from channel 6
    put_circ_buf(&sample_buf1, vin); // Store sample even if processor is busy

    //printf("ADC %d \n",vin); //debug tool
    //delay(1000); //Delay to allow for reading of the serial port

    phaseclk=phaseclk+1; //Clock is increased by 1 every time the interrupt is called
    FREQTIME=FREQTIME+1;
    t = t+ (1.0f)/fs;


}

//Filters the signal that was read in
int filter(int input){
    for (int i=2;i>0;i--){ //shifts old numbers
        x[i]=x[i-1];
        y[i]=y[i-1];
    }

    x[0]=input;// gets new input
    y[0]= x[0]*b[0]
        + x[1]*b[1]
        + x[2]*b[2]
        - y[1]*a[1]
        - y[2]*a[2];// gets new output
    
    int output = y[0];
    //printf("X0 %d \n",input); //debug tool
    //printf("Y0 %d \n",output); //debug tool
    //delay(10000000); //Delay to allow for reading of the serial port
    return output; // returns the output of the filter
}

int sinusoidGen(int frequency, int max,int min, float time, int phaseshift){
    float scale=(max-min)/2;//From middle to max or min
    float offset=(max+min)/2;//Middle of the wave

    int sinu=offset+scale*sin(2*pi*frequency*time+phaseshift); // offset+scale*sin(2*pi*f*t)
    //int value=sin(2*pi*frequency*time+phaseshift)*1000;
    //printf(" %d \n",sinu);
    //delay(100);
    return sinu;
}

//Buffer Functions
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
//Writing to serial port
int _write(int file, char *data, int len)
{
    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
    {
        errno = EBADF;
        return -1;
    }
    while(len--)
    {
        while( (USART2->ISR & (1 << 6))==0); // wait for ongoing transmission to finish
        USART2->TDR=*data;    
        data++;
    }    
    return 0;
}
void USART2_IRQHandler()
{
    char c;
    if ((USART2->ISR &(1<<5)) )
    {
        c = USART2->RDR;
        if (c==']')
        {
            data_ready = 1;
        }
        else if(c=='[')
        {
            init_circ_buf(&sample_buf2);
        }
        else
            put_circ_buf(&sample_buf2,c);
    }
}
