#include <stdint.h>
#include <stm32l432xx.h>

void initClocks();
void enablePullUp(GPIO_TypeDef *Port, uint32_t BitNumber);
void pinMode(GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t Mode);
void selectAlternateFunction (GPIO_TypeDef *Port, uint32_t BitNumber, uint32_t AF);

//System
void setup(void);
void delay(volatile uint32_t dly);
void initSerial(uint32_t baudrate);

//Read Write
void initADC();
int readADC(int channel);
void initDAC();
void writeDAC(int value);

//Sinusoid Functions
int Phaseshift();
int FrequencyAverage(int frequency);