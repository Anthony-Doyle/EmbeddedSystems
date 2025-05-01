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


### SinewaveTable
Creating a sinewave table allows for quick calculation of a sinewave rather then the computationally expensive taylor series which slowed down excucition of the code.
This allowed for more code to be used and allowed the systick handler to function without overrunning its sample rate.
There needed to be enough samples in the table to allow for a good resolution without having excess waste.
256 was used as with a sample rate of 100 per cycle 256 gave enough of a resolution while costing minimal resources.

![Table code](https://github.com/user-attachments/assets/375f0ccc-d60e-4950-9f01-10865c6b3ff3)
![Table](https://github.com/user-attachments/assets/452c7f46-4951-4134-851a-818696a7e488)


## Filter
The filter used in this project was a butterworth low pass filter.
An FIR filter was tested but due to how this filter is implemented it took alot more resources to compute for an aduquate attenuation.




## Interrupt
Systick was used to read and write at a given sample rate.
A sample rate of 5000hz as it allowed for 100 samples at the 50hz which is the hertz of the Irish power system.
This sample rate give us a systick interruption every 200uS due to the L432kc 80Mhz speed.
Within the Systick the ADC and DAC must be called to allow for consistant sampling aswell as a few incremental time based variable.
The Systick must be completeted before the next interrupt is called or there would be delays in the sample rate.


This worked mostly but at higher sample rates the system ran slow as the systick was called before the rest of the code would have completed leading to broken outputs as seen below
(PUT PICTURE IN OF SLOW AND FAST FS)
Slow FS lead to accurate outputs but poor resolution.


An external timer was used by DMA to allivate the process of systick interupting the main code to run the dac and adc 


## Buffer

## Results

