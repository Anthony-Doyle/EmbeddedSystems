%% Setup
% producing basic varibles for code

clc; clear;close all;
fs=200000       %1000 hz sample rate
fc=7500;         %break frequency
t=0:1/fs:1-1/fs  %1 Seconds long

n=2; %system order
Wn= fc/(fs/2);  %Break frequency normalised between 0 and fs/2

Signal_Input=cos(2*pi*t*(30000)) %t is the amount of cycles per second, 2t is 2 cycles ect
%Signal_Input=square(x,50);
plot(t,Signal_Input)   % plots the sinusoid
% Filter
%Using a basic low pass filter, the signal will be filtered to get the expected output from a sinusoid input. 
%This will allow replication on a board and a way of comparing the outputs.

% Signal process using matlab functions
% [Signal_Output,d]=lowpass(Signal_Input,0.6)
% figure(2)
% plot(t,Signal_Output)

% butterworth filter
hold on;
[b,a]=butter(n,Wn,'low')
Signal_Output2=filter(b,a,Signal_Input)
plot(t,Signal_Output2)
ylim([-1 1])
xlim([200/(30000),202/(30000)])
legend

freqz(b,a,[],fs)
xscale("log")
