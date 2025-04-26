This projects objective is to create a filter that can read in a signal from a power line, Filter it, phase shift where nessiscary and output the signal.
DAC, ADC, Buffers will be used throughout the process.

Issue caused by systemclock being 4Mhz not 80Mhz as expected which cahnged my frequency calculations, debugger was neeeded to realied and fix the issue

phase shift done using peteniometer to determine how leading or lagging the phase is

L432kc set to run at 80MHz or 12.5ns between executions, to take 50000 sample per second, a sample is taken every 1600 ticks 20us
To calculate frequency we take 1/(num_of_samples*time_per_sample)

Project works with resolution issue, DMA must be implemented to allow for increased resolution without systick taking up all the resources of the cpu 
> This is a quote
# Specialist embedded systems project
## Table of contents

## Introduction
The specialist embedded system pick was a sinusoidal generator that would read an excisting noicing and broken sinusoid and repecated as a clean source.
The phase should be able to be adjusted to allow for voltage control in a power grid.
There should be a high frequency filter to remove noice from the signal.
DAC, ADC should be used to read and write a signal allowing for easy storage of units and modification.
DMA should be used to allow for faster excicution** of code while read and write will be done without slowing the main() code.
Buffers should be used to store the read units, the filtered units and printf() characters.

By the completion of the project, a signal should be able to be replicated with the same freqency, maximums, minimums, and an adjustable phase of 180 -> -180 degrees.
This project will be built around the idea of a power system signal cleaning 


## Matlab

## Filter

## Interrupt

## Buffer

## Results

