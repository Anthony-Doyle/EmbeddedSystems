This projects objective is to create a filter that can read in a signal from a power line, Filter it, phase shift where nessiscary and output the signal.
DAC, ADC, Buffers will be used throughout the process.

phase shift done using peteniometer to determine how leading or lagging the phase is

L432kc set to run at 80MHz or 12.5ns between executions, to take 50000 sample per second, a sample is taken every 1600 ticks 20us
To calculate frequency we take 1/(num_of_samples*time_per_sample)



> This is a quote

The specialist embedded system pick was a sinusoidal generator that would read an excisting noicy and broken sinusoid and repecated as a clean source.
The phase should be able to be adjusted to allow for voltage control in a power grid.
There should be a high frequency filter to remove noice from the signal.
DAC, ADC should be used to read and write a signal allowing for easy storage of units and modification.
Buffers should be used to store the read units, the filtered units and printf() characters.

By the completion of the project, a signal should be able to be replicated with the same frequency, maximums, minimums, and an adjustable phase of 180 -> -180 degrees.
This project will be built around the idea of a power system signal cleaning 


# Clean sinusoidal generator based on a input dirty sinusoid

## Table of contents
[Introduction](https://github.com/Anthony-Doyle/EmbeddedSystems/edit/main/ReadMe.md#clean-sinusoidal-generator-based-on-a-input-dirty-sinusoid)

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
A base sinusoid is read in through ADC channel 6(PA1), stored in a buffer, and used to gather required information to created a clean sinusoid.
The buffer stores samples until it can be filtered and processed to gather the frequency, maximum value, minimum value.
Using MATLAB a basic low pass filter was designed to remove high frequency noice from the incoming samples.
A phaseshift is calcualted using a petentiometer which can be adjusted and read using ADC channel 8(PA3).
Once the code was processed, it was output through DAC1(PA4).

Systick was used to allow timing systems within the code. 
The ADC, DAC, sinusoid generation ,and some varible increments are done within the systick handler.

## Circuit Diagram
<img src="https://github.com/user-attachments/assets/23ad9dfc-6cd9-4187-9f12-ff8bcbe39301" alt= "Circuit Diagram" width="500" height="500">

<img src="https://github.com/user-attachments/assets/9b4fa2b1-b9f5-4076-8ed2-b20d6f13b07b" alt= "Circuit Diagram" width="500" height="500">

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
This may slow it down if there is is to much computation.
The Systick must be completeted before the next interrupt is called or there would be delays in the sample rate.

![image](https://github.com/user-attachments/assets/ea603486-568c-42dd-872e-48bf5ba17569)

### ADC error
During the process of creating this code, I found that there was data being lost but i couldn't track from where.
Ititally i believed this was due to the large sinewave generation using "sin()" but that being removed didn't resolve the issue.
After a discution with my lecturure it was found that there was a delay in when ADC was run cauing ADC to take approximely 60-80uS out of the 200uS available for the complete process.
This being fixed freed up alot of computation time which I had to try an limit up until then with ADC being reduced to under 20uS with further reductions possible by reducing the delay further

![ADC code](https://github.com/user-attachments/assets/294542aa-19ae-43d0-9edc-79c74f664e99)

![TimeWithDelay(0x0f)](https://github.com/user-attachments/assets/77e9aab6-9bc8-4010-b926-a95e86f0d09d) Delay set to 0x0f

![TimeWithDelay(0x07)](https://github.com/user-attachments/assets/cd001de1-5755-495f-949a-9dbfa17e8aae) Delay set to 0x07

![TimeWithDelay(0x01)](https://github.com/user-attachments/assets/ae4a3412-12c8-444a-9767-02d6977907f0) Delay set to 0x01

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

To run the code completely after all the optimisation took approximatly 45uS

![Time for full code](https://github.com/user-attachments/assets/cf6171bf-f28f-4adf-a134-18a7516b18cb)

## Filter
The filter used in this project was a butterworth low pass filter.
An FIR filter was tested but due to how this filter is implemented it took alot more resources to compute for an aduquate attenuation.

The filtered worked by shifting the old values of singal input and feedback andusing them in the difference equation 
This allowed for smoother and less noicy processing of the signal in the main loop

![image](https://github.com/user-attachments/assets/ca0599b5-90c8-475e-8b03-ee70aa8cfc30)

## Sinusoid Generator
To start i created a a fuction which created a sine wave which was scaled and offset using the information gotten from the main(). 
This worked well as the sinwave could be produced using sin() taken from math.h.
Scale and offset were gotten from equations using the max and min to find the middle between them and the distance from that middle to each.

![Slow](https://github.com/user-attachments/assets/26271acf-7726-4ba3-906b-6b0ac3890aee)

This way of calculating a sinewave  is very computationally expecsive as it uses the taylor series. 
Issues occured as the sinwave is constantly calculated with new values.
To fix this, a look up table was implemented.
The lookup table allowed for quick calculations of the sinewave by using truckation.
The phase value was calculated and used to find the closest phase value for an 8 bit sinwave.

![FAST](https://github.com/user-attachments/assets/f19d6853-c330-42f7-b326-4cf4a6f864fc)

Due to the truncation, when frequency was changed at the maximum, there was an error that caused massive diviations in the sinwave.
To fix this the phase was made as accumilitive, this allowed for softer transitions between frequencys. 
The phaseshift was not an accumilated shift and so must be subtracted and readded to avoid major changes froma small shift.
An if state wasused to normalze the phase to between 0 and 2pi or 0 and 360 degrees.

A 1.2 multiplier was also given to offset any losses caused by the filter at 50hz. 
This allowed for accurate reproduction of a sinwave with minimal cost.

## Buffer
A Buffer was used at multiple points through out the coding process.
ADC, printf and DAC used buffers to avoid losses if the job couldnt emidiatly be completed.
Due to optomazations in the code buffer 3 was unneeded as the dac had enough time to complete its task emmediatly following the ADC in the systick.

The Buffer contained data, start, end, count and total size of buffer.
This allowed the buffer to track its contents and hold enough data to fuction correctly.
Buffer 1 was able to be reduced in size due to the speed the program ran at by the end of the development.

![Setup](https://github.com/user-attachments/assets/a921a22c-f5cf-4eb9-8806-b16c7dbb7b2c)

Below shows the functions associated with the buffer and how data was treated when being fed into and extracted from the buffer.
The count keeps track of how much data is contained. 
The head is the most recent added data 
The tail is the first data in that has yet to be processed.
This runs on a FIFO system.

![Functions](https://github.com/user-attachments/assets/7ad4cc6b-a35b-49be-9884-55f5d2918c95)

## Conclusion
This project successfully demonstrated the creation of a real-time signal processing system for filtering and phase-shifting a sinusoidal signal. 
The system achieved its goal of removing noise from the input signal and generating a clean, adjustable-phase output. 
By leveraging hardware peripherals like ADCs, DACs, and buffers, and applying software optimizations such as the use of look-up tables and the Butterworth filter, the system was able to meet the stringent timing requirements of 5000 samples per second.

The project was able to run efficiently, with the main code execution taking around 40-50 µs per cycle, leaving ample room for further optimization. 
The successful implementation of phase shift control via a potentiometer also opens up potential for further refinement and use in applications like power grid voltage control.


## Results

## Future Work
Further Optimization: There may still be opportunities to optimize the ADC sampling process to reduce delays further, possibly implementing DMA (Direct Memory Access) to offload ADC and DAC processing and avoid additional complexity.

Dynamic Filtering: Introduce adaptive filtering to handle different types of noise or changing conditions in the input signal dynamically.

Increased Sampling Rate: With further hardware resources or optimizations, the system's sample rate could be increased to handle higher-frequency signals for more demanding applications.

Testing in Real-World Scenarios: The next step would involve testing this system in real-world power grid environments, where noisy signals and variable phases are common. This would allow for further refinement and validation of the approach.
