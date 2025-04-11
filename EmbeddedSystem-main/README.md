# Getting started with embedded systems project
## Table of contents
[Introduction](https://github.com/AntKing12/EmbeddedSystem#introduction)

[Matlab](https://github.com/AntKing12/EmbeddedSystem#matlab)

[Filter Function](https://github.com/AntKing12/EmbeddedSystem#filter-function)

[Circuit Diagram](https://github.com/AntKing12/EmbeddedSystem#circuit-diagram)

[Port Issue](https://github.com/AntKing12/EmbeddedSystem?tab=readme-ov-file#port-issue)

[Systick Interrupt](https://github.com/AntKing12/EmbeddedSystem#systick-interupt)

[Buffer](https://github.com/AntKing12/EmbeddedSystem#buffer)

[Results](https://github.com/AntKing12/EmbeddedSystem#results)
## Introduction
The scope of this project was to create a low pass filter using digital signal processing.

An L432KC microprocessor would be used to read in a signal, filter it using a difference equation and output the filtered signal.

Within this project an ADC, DAC, interrups and buffer are used to create a digital low pass filter.

Digital signal processing is the technique of using equations to alter signals rather than components such as capacitors.
This allows for greater accuracy and repeatability as the system will always act the same no matter where its implemented while components which can vary under different environmental conditions such as temperature.

## Matlab
MATLAB was used to create a filter and signal.
Within MATLAB there is a function "butter()" which allows for Butterworth filters to be easily created.
This function requires three inputs to work, system order, normalised break frequency, type of filter.
Type of filter determines what is done.
A "low" was put into this spot to make a low pass filter.
A second-order filter was chosen, as it is the most commonly used.
Normalised break frequency represents where the system would see a 3dB attentuaion in voltage.
It's normalised by picking a freuency you want to start attenuaing,fc and putting it over half the sample frequency,fs.

fc/(fs/2)

This will give a number somewhere between 1 and 0. 
The fc in this project was 7500hz or 0.075 when normalised.

Using the above values in MATLAB you input "[b,a] = butter(2, 0.075, "low"); ".
b and a would contain the required values needed for the difference equation to filter the signal.

A signal was created using the "cos" function in MATLAB.
The frequency was adjusted to allow for testing the filter at different input frequencies.

## Filter Function

For ease of use the existing code "Analog pass through Systick" was sampled to allow for use of the systick interupt and the DAC/ADC functions rather then rewriting them.

The b and a values were put into a difference equation.
These equations determine the output based on new and old information, with delays and feedback.

y[0]*a[0] = x[0]*b[0]+x[1]*b[1]+...x[n]*b[n] - y[1]*a[1]-...y[n]*a[n].

where y[0] is the output and x[0] is the input.
x[1] and onwards represent previous values that are held to be used in future processing.
y[1] and onwards represent previous outputs that are fed back into the equation.


![image](https://github.com/user-attachments/assets/34952c4e-3701-4905-b192-ded071ac9886)

![Screenshot 2025-03-22 153510](https://github.com/user-attachments/assets/2820b059-3966-4d76-a4c2-d208965e6768)

A for loop was used to shift the data from one position to the next whenever a new input was being read.

## Circuit Diagram

![image](https://github.com/user-attachments/assets/097424a9-c8eb-4d34-9c6a-510b326082a2)

The circuit diagram is minimal due to this being a digital process with just a signal input and output being required.
The power is also connected with all grounds going back to the microprocessor.

![image](https://github.com/user-attachments/assets/99fc52a4-e186-4c1d-90fc-ae8e527c847b)

Pa_1 and pa_4 were chosen due to their analog functions being ADC and DAC.
Pa_0 was orignally meant to be the ADC but broke during testing and had to be adjusted.

## Port issue
While working on the code an issue became apparent as no output was being read.
Testing had to be done to see where the error was occuring.

A constant was applied to the dac to see if an output was read.
Changing the constant changed the value read on the oscilloscope showing that the DAC was working an outputting.

The oscilloscope read that a signal was applied to the input showing the function generator worked as expected.

The orignial analogue pass through systick code was uploaded onto the L432kc to ensure that the changes to the code weren't affecting the pass through.

The orignal code still did not work.
By changing the code to use pa_1 which has a ADC on channel 6 rather then chanel 5 to determine if pa_0 was broken.
A signal was then observed passing through the microprocessor.
This change was passed onto the filtering code and the signal was now filtering correctly confirming that pa_0 was not reading a signal.

## Systick Interrupt 
The systick interrupt uses the system clock to determine when an interrupt is called.
By including this it allows for an accurate clock/timer.

![image](https://github.com/user-attachments/assets/75bf76a0-2229-456a-a1a6-a7f3cb6474f1)

![image](https://github.com/user-attachments/assets/4abbb48e-199e-4ccb-9833-6d39ac458c1e)

In this example the systemcoreclock is being used to create an interupt 200000 times a second.
This creates an interupt every 5 microseconds which samples the incoming signal and stores it to be processed.

Using an interrupt is important here as it ensures that the sample rate is kept at a constant speed which is important to a filter

## Buffer
A buffer is a way of storing information until it is ready to be processed. 
This is important because if a processer is busy it may not process the new information being given which would affect the output.

![init buffer](https://github.com/user-attachments/assets/b50de65c-7757-4f80-9345-b57d8ea175a3)

![process buffer](https://github.com/user-attachments/assets/33cc8f1e-0775-4905-a601-a3fce7e04791)

![build buffer](https://github.com/user-attachments/assets/a208a804-b50c-467b-b8ca-d3e6feb581f9)

In the above example we see the uses of the buffer.
It is initialised to have a tail, head and count which helps keep track of the contents.
There are 64 slots available in this buffer which equates to 330 micro seconds worth of buffering

When the code is running the buffer is being processed where the tail of the buffer is put through the filter and output.
This is done continuously until the buffer is empty.

When the systick inturupt is called a sample is taken and stored in the buffer.
This ensures that no matter what the microprocessor is currently doing the data will be read when needed.


## Results
Testing was done using oscilliscopes and comparing the output of the filter to the matlab model to assess whether real-world results matched the expected behaviour.
### Sine wave input
#### 200hz
<img src="https://github.com/user-attachments/assets/74140883-4965-469e-bb5a-e402e2281356" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/b760b9ac-640e-4411-952f-20b69b66a8cd" alt="200hz sin" width="400" height="300">

#### 3000hz
<img src="https://github.com/user-attachments/assets/5d56a05d-b9f7-4bab-9f72-a4ddf948c17c" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/4467433a-c3e3-4490-8680-bdd46c48d49b" alt="200hz sin" width="400" height="300">

#### 7500hz
<img src="https://github.com/user-attachments/assets/fb7171ec-423d-45fd-b645-c9b81e24b1c3" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/fb63e6f8-f5e0-425c-af71-da7c83489a56" alt="200hz sin" width="400" height="300">

#### 30000hz
<img src="https://github.com/user-attachments/assets/8d88d68a-72e3-4823-bc3a-1864000cc694" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/a4128e41-e7a8-4ac5-a40c-e4cb69b0d540" alt="200hz sin" width="400" height="300">

### Square wave input

#### 200hz
<img src="https://github.com/user-attachments/assets/9158efab-5920-47d1-8470-fe2042c9ea22" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/81ace64d-d1ea-4dd0-86d9-4b455247dd5d" alt="200hz sin" width="400" height="300">

#### 2000hz
<img src="https://github.com/user-attachments/assets/e2edb72d-b7b0-459f-947e-15a603c89497" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/b833e632-36cd-493b-b53c-95419a001da4" alt="200hz sin" width="400" height="300">

#### 7500hz
<img src="https://github.com/user-attachments/assets/a7810ef4-8ecf-47eb-b458-22a3aa488239" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/f92557a0-1982-4439-a2b4-b019f60f1934" alt="200hz sin" width="400" height="300">

#### 20000hz
<img src="https://github.com/user-attachments/assets/83b37305-81a8-4520-98d2-aa6961712c2c" alt="200hz sin" width="400" height="300"> <img src="https://github.com/user-attachments/assets/67deacc3-7bdb-40ff-ba47-e4201a4e94e2" alt="200hz sin" width="400" height="300">

### Discussion
It can be seen that there is a big similarity between the model and the real output. some small varitiy exists. 
This may be due to real world errors in sampling a signal aswell as not getting exact signals with noise and frequency variations.

