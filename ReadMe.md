
# Clean sinusoidal generator based on a input dirty sinusoid

## Table of contents
1. [Introduction](#introduction)
2. [Circuit Diagram](#circuit-diagram)
3. [MATLAB](#matlab)
   - [Filter](#filter)
   - [Sinewave Look-Up Table](#sinewave-look-up-table)
4. [Systick](#systick)
   - [Error with ADC in Systick](#error-with-adc-in-systick)
5. [Main](#main)
   - [Frequency](#frequency-average)
   - [Phase Shift](#phaseshift)
6. [Filter](#filter-1)
7. [Sinusoid Generator](#sinusoid-generator)
8. [Buffers](#buffer)
9. [Conclusion](#conclusion)
10. [Results](#results)
11. [Future Work](#future-work)


## Introduction
This goal of this project was to be able to replicate the base values of a sinusoid removing noise and effects.
To do this ADCs, DAC, buffers, filters, systick and a sinusoid generator.
A base sinusoid is read in through ADC channel 6(PA1), stored in a buffer, and used to gather required information to create a clean sinusoid.
The buffer stores samples until it can be filtered and processed to gather the frequency, maximum value, minimum value.
Using MATLAB a basic low pass filter was designed to remove high frequency noise from the incoming samples.
A phase shift is calculated using a potentiometer which can be adjusted and read using ADC channel 8(PA3).
Once the code was processed, it was output through DAC1(PA4).

Systick was used to allow timing systems within the code. 
The ADC, DAC, sinusoid generation, and some variable increments are done within the systick handler.

## Circuit Diagram
<img src="https://github.com/user-attachments/assets/23ad9dfc-6cd9-4187-9f12-ff8bcbe39301" alt= "Circuit Diagram" width="500" height="500">

<img src="https://github.com/user-attachments/assets/9b4fa2b1-b9f5-4076-8ed2-b20d6f13b07b" alt= "Circuit Diagram" width="500" height="500">

The circuit diagram above shows the ease of construction. 
As a digital signal process all of the adjustments are done in the code.


## MATLAB
MATLAB was used to create a filter as well as the points for the sinewave table.

### Filter
Using butter, a Butterworth filter was created to filter out high frequency
![Filter MATLAB code](https://github.com/user-attachments/assets/a2b89cc2-dc1c-446b-8ff5-ebb18ca380fa)
![image](https://github.com/user-attachments/assets/b9865cdc-8eeb-48fb-82f1-e818aa376071)

FIR, High pass, and band pass filters were tested but would cause excess computation to achieve the same results or to much attenuation at 50hz, the expected input.
FIR needed much greater orders to achieve the same attenuation, upwards of 40 orders vs the 4 orders used by the Butterworth.
Bandpass lead to much greater errors involving losing signals that should be accepted.
I believe this was caused by rounding errors when transferring the A and B coefficients from MATLAB to C

### Sinewave Look-Up Table
Creating a sinewave look-up table allows for quick calculation of a sinewave rather than the computationally expensive Taylor series which slowed down execution of the code.
This allowed for more code to be used and allowed the systick handler to function without overrunning its sample rate.
There needed to be enough samples in the table to allow for a good resolution without having excess waste.
256 was used as with a sample rate of one hundred per cycle 256 gave enough of a resolution while costing minimal resources.

![Table code](https://github.com/user-attachments/assets/375f0ccc-d60e-4950-9f01-10865c6b3ff3)
![Table](https://github.com/user-attachments/assets/452c7f46-4951-4134-851a-818696a7e488)

## Systick
Systick was used to read and write at a given sample rate.
A sample rate of 5000hz as it allowed for 100 samples at the 50hz which is the hertz of the Irish power system.
This sample rate give us a systick interruption every 200uS due to the L432kc 80Mhz speed.
To ensure this speed the systick was loaded with a variable to increment to.

![image](https://github.com/user-attachments/assets/fdd5c8bb-8459-4dab-b2b0-ad8f66bcd7ff)

Within the Systick the ADC and DAC must be called to allow for consistently timed sampling as well as a few incremental time-based variable.
The sinusoid value was also calculated in the systick which isn’t strictly necessary but ensure accurate DAC outputs.
This may slow it down if there is to much computation.
The Systick must be completed before the next interrupt is called or there would be delays in the sample rate.

![image](https://github.com/user-attachments/assets/ea603486-568c-42dd-872e-48bf5ba17569)

### Error with ADC in Systick
During the process of creating this code, I found that there was data being lost but I couldn't track from were.
Initially I believed this was due to the large sinewave generation using "sin()" but that being removed didn't resolve the issue.
After a discussion with my lecturer it was found that there was a delay in when ADC was run causing ADC to take approximately 60-80uS out of the 200uS available for the complete process.
This being fixed freed up a lot of computation time which I had to try a limit up until then with ADC being reduced to under 20uS with further reductions possible by reducing the delay further.

![ADC code](https://github.com/user-attachments/assets/294542aa-19ae-43d0-9edc-79c74f664e99)

![TimeWithDelay(0x0f)](https://github.com/user-attachments/assets/77e9aab6-9bc8-4010-b926-a95e86f0d09d) Delay set to 0x0f

![TimeWithDelay(0x07)](https://github.com/user-attachments/assets/cd001de1-5755-495f-949a-9dbfa17e8aae) Delay set to 0x07

![TimeWithDelay(0x01)](https://github.com/user-attachments/assets/ae4a3412-12c8-444a-9767-02d6977907f0) Delay set to 0x01

DMA was looked at to off load the ADC and DAC process but was unneeded due to this reduction in time usage. 
This would have caused excess complexity which was unneeded for the process.

## Main
Within the main function the samples from the ADC were filtered and then processed.
This Process consisted of finding when the slope of the signal changed and using that to calculate the max, min, and frequency.
"FREQTIME" was a variable that incremented with the systick and was used as a way of tracking time in relation to the sample rate which could be sued to find the frequency of the input signal.
Max and min were calculated by finding the point at which the slope would change from positive to negative or vice versa.

Phase shift was also calculated ever 10ms allowing change to occur without taking up excess processing power.
This used the systick as a counter in comparison to the sample rate which would find them "phaseclk" became greater then 50.

"Directionchange" kept track of the slope changing from positive to negative after reaching the max and negative to positive after reaching the minimum.

All of this was kept inside the if statement which checked if there was any sample in the buffer which again reduced processing time. 

![image](https://github.com/user-attachments/assets/a0c5289d-ad1c-48c1-8e51-500fff2086bf)

To run the code completely after all the optimisations took approximately 45uS

![Time for full code](https://github.com/user-attachments/assets/cf6171bf-f28f-4adf-a134-18a7516b18cb)

### Frequency Average

The frequency updates on average every 200uS with minor changes.
To allow these slight changes to be smoothed an averager was added.
This averager can be done in two ways, Hard coded average or weighed change.
In the code we can see a hard coded average where forty of the most recent frequencies are stored in an array and average. 
This helps keep a consistent frequency as the code runs and allows for smooth transitions.

There is also a weighted average which is commented out but works much the same where the incoming frequency has a limited effect on the overall value.
This keeps the value mostly isolated from single major increases while allowing changes to occur overtime.
Both works equally well while the weighted average would be a more optimal approach due to limited computation.

![image](https://github.com/user-attachments/assets/7c3b55a8-2c88-4775-87f0-92c127eb26d5)


### Phaseshift

A phase shift code was implemented to allow for a change in phase as desired.
This would be used to synchronise the input and output signal of the board.
this is done by reading the PA3 ADC and converting the 0-4096 units to 0-2pi radians which can later be added to the sinusoid calculator.
This shift is currently done manually but could be automated allowing for automatic phase alignment.

To manually adjust this phase, a potentiometer is twisted which adjusts the voltage entering PA3.

![image](https://github.com/user-attachments/assets/6d7fa169-28cb-426f-9906-a3deef056d51)

## Filter
The filter used in this project was a Butterworth low pass filter.
An FIR filter was tested but due to how this filter is implemented it took a lot more resources to compute for an adequate attenuation.

The filtered worked by shifting the old values of signal input and feedback and using them in the difference equation. 
This allowed for smoother and less noisy processing of the signal in the main loop.

![image](https://github.com/user-attachments/assets/ca0599b5-90c8-475e-8b03-ee70aa8cfc30)

## Sinusoid Generator
To start I created a function which created a sine wave which was scaled and offset using the information gotten from the main(). 
This worked well as the sine wave could be produced using sin() taken from math.h.
Scale and offset were gotten from equations using the max and min to find the middle between them and the distance from that middle to each.

![Slow](https://github.com/user-attachments/assets/26271acf-7726-4ba3-906b-6b0ac3890aee)

This way of calculating a sinewave  is very computationally expensive as it uses the Taylor series. 
Issues occurred as the sine wave is constantly calculated with new values.
To fix this, a look up table was implemented.
The lookup table allowed for quick calculations of the sinewave by using truncation.
The phase value was calculated and used to find the closest phase value for an 8 bit sine wave.

![FAST](https://github.com/user-attachments/assets/f19d6853-c330-42f7-b326-4cf4a6f864fc)

Due to the truncation, when frequency was changed at the maximum, there was an error that caused massive deviations in the sine wave.
To fix this the phase was made as accumulative, this allowed for softer transitions between frequencies. 
The phase shift was not an accumulated shift and so must be subtracted and readded to avoid major changes from a small shift.
An if state was used to normalize the phase to between 0 and 2pi or 0 and 360 degrees.

A 1.2 multiplier was also given to offset any losses caused by the filter at 50hz. 
This allowed for accurate reproduction of a sine wave with minimal cost.

## Buffer
A Buffer was used at multiple points throughout the coding process.
ADC, printf and DAC used buffers to avoid losses if the job couldn’t immediately be completed.
Due to optimizations in the code buffer 3 was unneeded as the DAC had enough time to complete its task immediately following the ADC in the systick.

The Buffer contained samples, start, end, count, and total size of buffer.
This allowed the buffer to track its contents and hold enough data to function correctly.
Buffer 1 was able to be reduced in size due to optimizations in the program by the end of the development.

![Setup](https://github.com/user-attachments/assets/a921a22c-f5cf-4eb9-8806-b16c7dbb7b2c)

Below shows the functions associated with the buffer and how data was treated when being fed into and extracted from the buffer.
The count keeps track of how much data is contained. 
The head is the most recent added data .
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

Below are the output from three input frequencies to show how the system reacts.
![TEK00002](https://github.com/user-attachments/assets/39962a51-e8ab-4cb4-af0e-1dd21134a7d4) 18hz input.

At 18hz we see an almost exact match with slight errors in scaling which can be fixed with software equations.
The above results highlight the precision of the sinusoid generator as frequency is less than 0.1hz off and phase has been matched.

![TEK00001](https://github.com/user-attachments/assets/3e73eb5f-88d0-4d39-802f-6810e4fc15ec) 95hz input.

At 95hz a slight attenuation is seen due to the filter applied to the samples.
A small error is once again seen of 0.2hz.
This error is minor and shows that the generator is accurate at high and low frequencies.

![TEK00000](https://github.com/user-attachments/assets/adf6e505-508c-42d5-b81c-aedb7fe4ce3f) 44hz input.

44hz is close to the 50hz expected as an input.
A minor error is seen of 0.15hz with a matched phase.
There is little attenuation seen at 44hz which matches expectations at the cut off frequency was designed to be 100hz.




## Future Work
Further Optimization: There may still be opportunities to optimize the ADC sampling process to reduce delays further, possibly implementing DMA (Direct Memory Access) to offload ADC and DAC processing and avoid additional complexity.

Dynamic Filtering: Introduce adaptive filtering to handle different types of noise or changing conditions in the input signal dynamically.

Increased Sampling Rate: With further hardware resources or optimizations, the system's sample rate could be increased to handle higher-frequency signals for more demanding applications.

Testing in Real-World Scenarios: The next step would involve testing this system in real-world power grid environments, where noisy signals and variable phases are common. This would allow for further refinement and validation of the approach.
