# Firmware
This directory houses an STM32CubeIDE project. It targets the STM32F334R8 board and runs a slightly modified version of [`algorithm/algorithm.c`](../algorithm/algorithm.c). You can immediately compile, flash, and run it with the IDE once the board is connected.
## Directory overview
[`Core/`](Core/): Contains source, header, and startup files for this STM32 project. All user-written code and documentation in this project is contained within [`Core/Src/main.c`](Core/Src/main.c).

[`Drivers/`](Drivers/): Contains device-specific drivers and hardware abstraction layer (HAL) files.

[`.mxproject`](.mxproject), [`firmware.ioc`](firmware.ioc): Used by STM32CubeMX to specify configuration information for this project (pin assignments, clock settings, peripheral configurations, etc.).

## The Right Chip for Your Salsa: Choosing a microcontroller

The STM32 family of microcontrollers was chosen for this project due to its comprehensive suite of development tools, extensive documentation, and vast range of devices, giving it a significant advantage over its competitors. The must-haves for this project are

- Two timers; one that triggers every sample period and another to flash an LED upon QRS detection
- An ADC with at least 12-bit resolution to capture the AD8232 output signal
- A dedicated FPU (floating point unit) for real-time signal processing
- A communication protocol to interface with a serial port, for debugging purposes and the GUI (this project uses USART)
- ~5 kB of RAM, largely used by signal buffers

The STM32F334R8 satisfies all of these requirements, is well-priced, and also had an in-stock development board that was ready to order.

## Minimalist Marvel: Assembling a barebones prototype for firmware testing

To develop and test the firmware on a real-time sampled signal, from electrode pads to the serial port, we used several development and breakout boards:

<div align="center">

| Part Number | Description |
|---|---|
| [AD8232-EVALZ](https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad8232.html) | AD8232 evaluation board |
| [NUCLEO-F334R8](https://www.st.com/en/evaluation-tools/nucleo-f334r8.html) | STM32 Nucleo-64 development board |
| [BOB-11570](https://www.sparkfun.com/products/11570) | SparkFun TRRS 3.5mm jack breakout |
| [LD1117-3.3 TO-220](https://www.st.com/resource/en/datasheet/ld1117.pdf) | 3.3 V linear voltage regulator |
| [CAB-12970](https://www.sparkfun.com/products/12970) | SparkFun 3.5 mm jack to 3-electrode cable |
| [3M 2560](https://www.3m.com/3M/en_US/medical-us/red-dot-ecg-electrodes/) | 3M Red Dot monitoring electrode pads |

</div>

These parts can be connected using only jumper wires and a breadboard in the following manner:

<div align="center">

![Prototype setup](/docs/visuals/prototype_setup.png)

</div>

Not included in this diagram is the electrode cable attached to the audio jack breakout board, and a simple 9V battery snap connector attached to a Duracell CopperTop which powers the regulator. The Nucleo development board connects to a PC using a micro-USB cable. 

## The idea graveyard

**More safety guardrails**: An important notice is included in the datasheet for the AD8232 evaluation board:

> For safety reasons, Analog Devices, Inc., recommends evaluating the device with an electrocardiogram (ECG) signal source  instead of a live subject. Although the LA, RA, and RL electrode  terminals are equipped with current-limiting resistors (R3, R4, and R5), these resistors are not a comprehensive patient protection system. The resistors may not protect against supply line transients or leakage currents coming through power and acquisition systems. The user is fully responsible for understanding and applying all the safety guidelines and regulations that apply to medical equipment.

The author of this repository felt daring enough to use the aforementioned current-limiting resistors and a battery power supply as his insurance against any potentially dangerous occurences, but an arbitrary waveform generator could be used in place of a live subject, although it does not allow for use of the right leg drive electrode or the leads-out detection functionality of the AD8232.

**Optimize the choice of MCU**: The STM32F334R8 microcontroller, being more powerful than this project requires, leaves many of its peripherals and pins unused. Reducing manufacturing cost might involve a more optimized choice of MCU.