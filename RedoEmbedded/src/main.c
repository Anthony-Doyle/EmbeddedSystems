#include <Functions.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include  <errno.h>
#include "core_cm4.h"
#include  <sys/unistd.h> // STDOUT_FILENO, STDERR_FILEN
volatile uint32_t data_ready=0;

//Varibles
const int fs=50*100;//sample 50hz wanted, 1000 samples per second
    //Sinwave
    int FREQTIME=0;//Time measured to calculate frequency
    float freq=0; //Frequency of the sinusoid
    int sinusoid=0; //value of sinwave to be put through DAC
    float t =0;//Time measurment for sinwave
    float phaseshift;//phaseshift of sinewave
    float frequency=50;
    float max=0; float min=0;//Previous max or min of the sinusoid 
    float pi=3.14159265358979323846; //Pi value
//
    //filter
int changedirection=1; //Desided whether slope is up or down
int sample=0; //Info read from buffer
float processed=0; //After Filter
float previous=0; // Previous value of processed
int SampPrev=0; //Previous value of sample
int phaseclk=0; //Clock used to measure time for the sinusoid
int vin=0;
//float tin=0; //Time in seconds
float sin_table[256] = {
    0.000000f,  0.024637f,  0.049260f,  0.073853f,  0.098400f,  0.122888f,  0.147302f,  0.171626f, 
    0.195845f,  0.219946f,  0.243914f,  0.267733f,  0.291390f,  0.314870f,  0.338158f,  0.361242f, 
    0.384106f,  0.406737f,  0.429121f,  0.451244f,  0.473094f,  0.494656f,  0.515918f,  0.536867f, 
    0.557489f,  0.577774f,  0.597707f,  0.617278f,  0.636474f,  0.655284f,  0.673696f,  0.691698f, 
    0.709281f,  0.726434f,  0.743145f,  0.759405f,  0.775204f,  0.790532f,  0.805381f,  0.819740f, 
    0.833602f,  0.846958f,  0.859800f,  0.872120f,  0.883910f,  0.895163f,  0.905873f,  0.916034f, 
    0.925638f,  0.934680f,  0.943154f,  0.951057f,  0.958381f,  0.965124f,  0.971281f,  0.976848f, 
    0.981823f,  0.986201f,  0.989980f,  0.993159f,  0.995734f,  0.997705f,  0.999070f,  0.999829f, 
    0.999981f,  0.999526f,  0.998464f,  0.996795f,  0.994522f,  0.991645f,  0.988165f,  0.984086f, 
    0.979410f,  0.974139f,  0.968276f,  0.961826f,  0.954791f,  0.947177f,  0.938988f,  0.930229f, 
    0.920906f,  0.911023f,  0.900587f,  0.889604f,  0.878081f,  0.866025f,  0.853444f,  0.840344f, 
    0.826734f,  0.812622f,  0.798017f,  0.782928f,  0.767363f,  0.751332f,  0.734845f,  0.717912f, 
    0.700543f,  0.682749f,  0.664540f,  0.645928f,  0.626924f,  0.607539f,  0.587785f,  0.567675f, 
    0.547220f,  0.526432f,  0.505325f,  0.483911f,  0.462204f,  0.440216f,  0.417960f,  0.395451f, 
    0.372702f,  0.349727f,  0.326539f,  0.303153f,  0.279583f,  0.255843f,  0.231948f,  0.207912f, 
    0.183750f,  0.159476f,  0.135105f,  0.110653f,  0.086133f,  0.061561f,  0.036951f,  0.012320f, 
    -0.012320f, -0.036951f, -0.061561f, -0.086133f, -0.110653f, -0.135105f, -0.159476f, -0.183750f, 
    -0.207912f, -0.231948f, -0.255843f, -0.279583f, -0.303153f, -0.326539f, -0.349727f, -0.372702f, 
    -0.395451f, -0.417960f, -0.440216f, -0.462204f, -0.483911f, -0.505325f, -0.526432f, -0.547220f, 
    -0.567675f, -0.587785f, -0.607539f, -0.626924f, -0.645928f, -0.664540f, -0.682749f, -0.700543f, 
    -0.717912f, -0.734845f, -0.751332f, -0.767363f, -0.782928f, -0.798017f, -0.812622f, -0.826734f, 
    -0.840344f, -0.853444f, -0.866025f, -0.878081f, -0.889604f, -0.900587f, -0.911023f, -0.920906f, 
    -0.930229f, -0.938988f, -0.947177f, -0.954791f, -0.961826f, -0.968276f, -0.974139f, -0.979410f, 
    -0.984086f, -0.988165f, -0.991645f, -0.994522f, -0.996795f, -0.998464f, -0.999526f, -0.999981f, 
    -0.999829f, -0.999070f, -0.997705f, -0.995734f, -0.993159f, -0.989980f, -0.986201f, -0.981823f, 
    -0.976848f, -0.971281f, -0.965124f, -0.958381f, -0.951057f, -0.943154f, -0.934680f, -0.925638f, 
    -0.916034f, -0.905873f, -0.895163f, -0.883910f, -0.872120f, -0.859800f, -0.846958f, -0.833602f, 
    -0.819740f, -0.805381f, -0.790532f, -0.775204f, -0.759405f, -0.743145f, -0.726434f, -0.709281f, 
    -0.691698f, -0.673696f, -0.655284f, -0.636474f, -0.617278f, -0.597707f, -0.577774f, -0.557489f, 
    -0.536867f, -0.515918f, -0.494656f, -0.473094f, -0.451244f, -0.429121f, -0.406737f, -0.384106f, 
    -0.361242f, -0.338158f, -0.314870f, -0.291390f, -0.267733f, -0.243914f, -0.219946f, -0.195845f, 
    -0.171626f, -0.147302f, -0.122888f, -0.098400f, -0.073853f, -0.049260f, -0.024637f, -0.000000f
};

//Arrays for filter
float b[]={1.329372889875296e-05,	5.317491559501185e-05,	7.976237339251778e-05,	5.317491559501185e-05,	1.329372889875296e-05};
float a[]={1.000000000000000,	-3.671729089161934,	5.067998386734185,	-3.115966925201742,	0.719910327291870};
float x[5]={0};//Empty arrays on inital upload used to store previous values for difference equation
float y[5]={0};
//Buffer
#define CIRC_BUF_SIZE1 4//amount of bytes buffer can hold
#define CIRC_BUF_SIZE2 256
#define CIRC_BUF_SIZE3 256
typedef struct {//Buffer structure and varibles
	int *data;
    uint32_t size;
	uint32_t head;
	uint32_t tail;
	uint32_t count;
} circular_buffer;
int buffer1_data[CIRC_BUF_SIZE1];//amount of bytes buffer can hold
int buffer2_data[CIRC_BUF_SIZE2];//amount of bytes buffer can hold
int buffer3_data[CIRC_BUF_SIZE3];//amount of bytes buffer can hold
circular_buffer sample_buf1 = {//Buffer default state
    .data = buffer1_data,   .size = CIRC_BUF_SIZE1,
    .head = 0,  .tail = 0,  .count =0
};
circular_buffer sample_buf2 = {//Buffer default state
    .data = buffer2_data,   .size = CIRC_BUF_SIZE2,
    .head = 0,  .tail = 0,  .count =0
};
circular_buffer sample_buf3 = {//Buffer default state
    .data = buffer3_data,   .size = CIRC_BUF_SIZE3,
    .head = 0,  .tail = 0,  .count =0
};

//Project functions
float filter(int input);
//int sinusoidGen(int frequency,int max, int min,float time, int phase);
int put_circ_buf(circular_buffer *buf ,int c);
int get_circ_buf(circular_buffer *buf ,int *c);
int sinusoidGenFast(float frequency,float max, float min,float time, float phase);
void boardReset(void);

//Main Function
int main(){
    setup(); //Sets up the needed fuction before completing the rest of the code
    //Sets up the systick interrupt
    SysTick->LOAD = SystemCoreClock/fs-1; // Systick clock = 80MHz. 80000000/(50*100)-1 = 15999
	SysTick->CTRL = 7; // enable systick counter and its interrupts
	SysTick->VAL = 10; // start from a low number so we don't wait for ages for first interrupt
	__asm(" cpsie i "); // enable interrupts globally
    pinMode(GPIOA,5,1);
    while(1)//Repeat until code is ended
    {   

        if (get_circ_buf(&sample_buf1, &sample) == 0) {// process the buffer if buffer is not empty   
            processed = filter(sample);// gain on filter due to mean drop

            if(processed>previous&&changedirection==-1){//filtered value is greater then previous value //if new min value doesnt equal the old min value and direction was decreasing  
                min=previous;
                changedirection=1;
                //printf("Min %d \n",min); //debug tool
                //delay(10); //Delay to allow for reading of the serial port
                //Min is reset and prepare to calculate max
            }   
            else if(processed<previous&&changedirection==1){//filtered value is less then previous value //if previous was growing assume max was found
                freq=(float)fs/(FREQTIME);//samples per second / samples since last max = per second = frequency
                max=previous; 
                frequency=FrequencyAverage(freq); //Average the frequency over 5 samples
                      // if direction was growing and is now decreasing we can assumine max was just reached 
                changedirection=-1; //Sets expected slope of sinusoid
                FREQTIME=0; //Time is reset

            }
            previous=processed;
            SampPrev=sample;
            //frequency=50.55;max=4000;min=0;
            //phaseshift=0; //Phaseshift is set to 90 degrees
            
            if(phaseclk > fs/100){ //Every 10ms
            //printf("P"); //debug tool
            //delay(100); //Delay to allow for reading of the serial port
            phaseshift=Phaseshift();
            phaseclk=0; //Reset clock
            } 
        }

        //tin=(float)t/1000000.0f ;
        //sinusoid=sinusoidGenFast(frequency,max,min,tin,phaseshift);
        //writeDAC(sinusoid);
        
    }

}

//Interrupts
void SysTick_Handler(void)
{   
    //GPIOA->ODR |= (1 << 5); //Set pin high to show that the interrupt is working
    vin = readADC(6); //Read ADC value from channel 6
    put_circ_buf(&sample_buf1, vin); // Store sample even if processor is busy
    //printf("ADC %d \n",vin); //debug tool
    //delay(10); //Delay to allow for reading of the serial port
    phaseclk=phaseclk+1; //Clock is increased by 1 every time the interrupt is called
    FREQTIME=FREQTIME+1;
    t = t+(1.0f)/fs;
    sinusoid=sinusoidGenFast(frequency,max,min,t,phaseshift);
    writeDAC(sinusoid);
    //writeDAC(sample); //Write to DAC
    //GPIOA->ODR &= ~(1 << 5); //Set pin high to show that the interrupt is working
}

//Filters the signal that was read in
float filter(int input){
    for (int i=4;i>0;i--){ //shifts old numbers
        x[i]=x[i-1];
        y[i]=y[i-1];
    }

    x[0]=input;// gets new input
    y[0]= (x[0]*b[0]
        + x[1]*b[1]
        + x[2]*b[2]
        + x[3]*b[3]
        + x[4]*b[4]
        - y[1]*a[1]
        - y[2]*a[2]
        - y[3]*a[3]
        - y[4]*a[4])/a[0];// gets new output
    
    //int output = y[0];
    //printf("X0 %d \n",input); //debug tool
    //printf("Y0 %d \n",output); //debug tool
    //delay(10000000); //Delay to allow for reading of the serial port
    return y[0]; // returns the output of the filter
}

// int sinusoidGen(int frequency, int max,int min, float time, int phaseshift){
//     float scale=(max-min)/2;//From middle to max or min
//     float offset=(max+min)/2;//Middle of the wave
//     float phase=2*pi*frequency*time+phaseshift; //Phase of the wave
//     int sinu=offset+scale*sin(phase); // offset+scale*sin(2*pi*f*t)
//     //int value=sin(2*pi*frequency*time+phaseshift)*1000;
//     //printf(" %d \n",sinu);
//     //delay(100);
//     return sinu;
// }

// int sinusoidGenFast(float frequency, int max, int min, float time, int phaseshift) {
//     int scale = (max - min) / 2;
//     int offset = (max + min) / 2;
//     float phase = 2.0f * 3.1415f * frequency * time + (float)phaseshift;
//     // Convert phase to index
//     int index = (int)((phase * (float)256) / (2.0f * 3.1415f));
//     index = index % 256; // wrap-around safely
//     float sineValue = sin_table[index];
//     int sinu = (int)(offset + scale * sineValue);
//     //printf("sineValue %f\t offset %d\t index %d\t frequeny %0.0f\n ",sineValue, sinu, index,frequency);
//     return sinu;
// }

int sinusoidGenFast( float frequency ,float max ,float min ,float time ,float phaseshift )
{
    static float phase_acc = 0.0f;      // in radians
    static float last_time   = 0.0f;    // in seconds
    static float old_phaseshift=0.0f;
    // Compute how long since last call
    float dt = time - last_time;// Smooths out frequency changes due to smooth time changes
    last_time = time;
    phase_acc += - old_phaseshift;// Removes affect from the accumilated phase shift
    old_phaseshift=phaseshift;// Saves the new phase shift
    // Update phase accumulator
    phase_acc += 2.0f * 3.14159265f * frequency * dt + phaseshift;//Makes the phase for a sinwave
    // Wrap into [0..2π)
    if (phase_acc >= 2.0f * 3.14159265f){
        phase_acc -= 2.0f * 3.14159265f;
    }
    else if (phase_acc < 0.0f){
        phase_acc += 2.0f * 3.14159265f;
    }

    // Convert to 0–255 table index
    uint8_t index = (uint8_t)(
        (phase_acc * 256.0f)
        / (2.0f * 3.14159265f)
    );

    // Table lookup
    float sineValue = sin_table[index];

    // Scale & offset
    float scale  = ((max - min) / 2.0f)*1.2f;
    float offset = ((max + min) / 2.0f)*1.2f;
    float   v    = offset + scale * sineValue;
    return v;
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
        //buf->tail  = buf->head-100;
        //buf->count = 100;
		return -1;
	}
}
int get_circ_buf(circular_buffer *buf,int *c)
{
    if(buf->count > 0)
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
//
int _write(int file, char *data, int len){
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
