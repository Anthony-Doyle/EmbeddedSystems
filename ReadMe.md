This projects objective is to create a filter that can read in a signal from a power line, Filter it, phase shift where nessiscary and output the signal.
DAC, ADC, Buffers will be used throughout the process.

Issue caused by systemclock being 4Mhz not 80Mhz as expected which cahnged my frequency calculations, debugger was neeeded to realied and fix the issue

phase shift done using peteniometer to determine how leading or lagging the phase is

L432kc set to run at 80MHz or 12.5ns between executions, to take 50000 sample per second, a sample is taken every 1600 ticks 20us
To calculate frequency we take 1/(num_of_samples*time_per_sample)

Project works with resolution issue, DMA must be implemented to allow for increased resolution without systick taking up all the resources of the cpu 

Look up Tables used to reduce resources when calculating the signwave


> This is a quote

The specialist embedded system pick was a sinusoidal generator that would read an excisting noicy and broken sinusoid and repecated as a clean source.
The phase should be able to be adjusted to allow for voltage control in a power grid.
There should be a high frequency filter to remove noice from the signal.
DAC, ADC should be used to read and write a signal allowing for easy storage of units and modification.
//DMA should be used to allow for faster excicution** of code while read and write will be done without slowing the main() code.
Buffers should be used to store the read units, the filtered units and printf() characters.

By the completion of the project, a signal should be able to be replicated with the same frequency, maximums, minimums, and an adjustable phase of 180 -> -180 degrees.
This project will be built around the idea of a power system signal cleaning 


# Clean sinusoidal generator based on a input dirty sinusoid

## Table of contents
Introduction

Circuit diagram

Matlab
--Filter
--SinewaveTable
  
Systick 
--Error with ADC in systick
  
Main
--GettingVaribles
--Frequency
--Phaseshift
  
Filter

SinusoidGenerator
--Slow
--Fast
  
Buffers
--Usecases of buffers
  
Conclution

Results

FutureWork


## Introduction
This goal of this project was to be able to replicate the base values of a sinusoid removing noice and effects.
To do this ADCs, DAC, buffers, filters, systick and a sinusoid generator.
A base sinusoid is read in through ADC channel 6, stored in a buffer, and used to gather required information to created a clean sinusoid.
The buffer stores samples until it can be filtered and processed to gather the frequency, maximum value, minimum value.
Using MATLAB a basic low pass filter was designed to remove high frequency noice from the incoming samples.
A phaseshift is calcualted using a petentiometer which can be adjusted and read using ADC channel 8.

Systick was used to allow timing systems within the code. 
The ADC, DAC, sinusoid generation ,and some varible increments are done within the systick handler.




## Circuit Diagram
![image](https://github.com/user-attachments/assets/23ad9dfc-6cd9-4187-9f12-ff8bcbe39301)

![image](https://github.com/user-attachments/assets/9b4fa2b1-b9f5-4076-8ed2-b20d6f13b07b)

The circuit diagram above shows the ease of construction. 
As a digial signal process almost all of the adjustments are done in the code.


## Matlab
Matlab was used to create a filter aswell as the points for the sinewave table.

### Filter
Using butter, a butterworth filter was created to filter out high frequencys
![Filter matlab code](https://github.com/user-attachments/assets/a2b89cc2-dc1c-446b-8ff5-ebb18ca380fa)
![image](https://github.com/user-attachments/assets/b9865cdc-8eeb-48fb-82f1-e818aa376071)

FIR, High pass and band pass filters were tested but would cause excess computation to achieve the same results or to much attenuention at 50hz, the expected input.
FIR needed much greater orders to achieve the same attenuation, upwards of 40 orders vs the 4 orders used by the butterworth.
Bandpass lead to much greater errors involving losing signals that should be accepted.
I believe this was caused by rounding errors when tranferring the A and B coefficeints from MATLAB to C

### Sinewave Look-Up Table
Creating a sinewave look-up table allows for quick calculation of a sinewave rather then the computationally expensive taylor series which slowed down excucition of the code.
This allowed for more code to be used and allowed the systick handler to function without overrunning its sample rate.
There needed to be enough samples in the table to allow for a good resolution without having excess waste.
256 was used as with a sample rate of 100 per cycle 256 gave enough of a resolution while costing minimal resources.

![Table code](https://github.com/user-attachments/assets/375f0ccc-d60e-4950-9f01-10865c6b3ff3)
![Table](https://github.com/user-attachments/assets/452c7f46-4951-4134-851a-818696a7e488)

## Systick
Systick was used to read and write at a given sample rate.
A sample rate of 5000hz as it allowed for 100 samples at the 50hz which is the hertz of the Irish power system.
This sample rate give us a systick interruption every 200uS due to the L432kc 80Mhz speed.
To ensure this speed the systick was loaded with a varible to increment to.

![image](https://github.com/user-attachments/assets/fdd5c8bb-8459-4dab-b2b0-ad8f66bcd7ff)

Within the Systick the ADC and DAC must be called to allow for consistantly timed sampling aswell as a few incremental time based variable.
The sinusoid value was also calculated in the systick which isnt strictly nessiscary but ensure accurate DAC outputs.
This may slow it down if there is is to much computation 
The Systick must be completeted before the next interrupt is called or there would be delays in the sample rate.
![image](https://github.com/user-attachments/assets/ea603486-568c-42dd-872e-48bf5ba17569)

### ADC error
During the process of creating this code, I found that there was data being lost but i couldn't track from where.
Ititally i believed this was due to the large sinewave generation using "sin()" but that being removed didn't resolve the issue.
After a discution with my lecturure it was found that there was a delay in when ADC was run cauing ADC to take approximely 60-80uS out of the 200uS available for the complete process.
This being fixed freed up alot of computation time which I had to try an limit up until then with ADC being reduced to under 20uS with further reductions possible by reducing the delay further
![ADC code](https://github.com/user-attachments/assets/294542aa-19ae-43d0-9edc-79c74f664e99)
![Picture of delay being 80uS]()

DMA was looked at to off load the ADC and DAC process but was unneeded due to this reduction in time usage. 
This would have caused excess complexity which was unneeded for the process.

## Main
Within the main fuction the samples from the ADC were filtered and then processed.
This Process consisted of finding when the slope of the signal changed and using that to calculat the max,min and frequency.
"FREQTIME" was a varible that incremented with the systick and was used as a way of tracking time in relation to the sample rate which could be sued to find the frequency of the input signal.
Max and min were calculated by finding the point at which the slope would change from positive to negitive or vice versa.

Phaseshift was also calulated ever 10ms allowing change to occure without taking up excess processing power.
This used the systick as a counter in comparison to the sample rate which would find them "phaseclk" became greater then 50.

"Directionchange" kept track of the slope changing from positive to negitive after reaching the max and negitive to positive after reaching the minimum.

All of this was kept inside the if statement which checked if there was any sample in the buffer which again reduced processing time 

![image](https://github.com/user-attachments/assets/a0c5289d-ad1c-48c1-8e51-500fff2086bf)

## Filter
The filter used in this project was a butterworth low pass filter.
An FIR filter was tested but due to how this filter is implemented it took alot more resources to compute for an aduquate attenuation.

The filtered worked by shifting the old values of singal input and feedback andusing them in the difference equation 
This allowed for smoother and less noicy processing of the signal in the main loop

![image](https://github.com/user-attachments/assets/ca0599b5-90c8-475e-8b03-ee70aa8cfc30)

## Sinusoid Generator

![Slow](https://github.com/user-attachments/assets/26271acf-7726-4ba3-906b-6b0ac3890aee)


![FAST](https://github.com/user-attachments/assets/f19d6853-c330-42f7-b326-4cf4a6f864fc)


## Buffer

![Setup](https://github.com/user-attachments/assets/a921a22c-f5cf-4eb9-8806-b16c7dbb7b2c)


![Functions](https://github.com/user-attachments/assets/7ad4cc6b-a35b-49be-9884-55f5d2918c95)



## Conclusion

## Results

## Future Work
