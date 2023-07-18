# Firmware
## Directory overview
[`Core/`](Core/): Contains source, header, and startup files for this STM32 project. All user-written code in this project is contained within [`Core/Src/main.c`](Core/Src/main.c).

[`Drivers/`](Drivers/): Contains device-specific drivers and hardware abstraction layer (HAL) files required for this project's chosen STM32 microcontroller.

[`.mxproject`](.mxproject), [`firmware.ioc`](firmware.ioc): Used by STM32CubeIDE to specify configuration information for this project (pin assignments, clock settings, peripheral configurations, etc.).

## Chip selection

The STM32 family of microcontrollers crosses off several boxes that lead to its use in this project. In comparison to its competitors/alternatives, it especially has a leg up in (i.e. where it really shines is) the comprehensive suite of development tools, extensive documentation, and vast range of devices. The must-haves for this project were

- Two timers; one that triggers every sample period and another to flash an LED upon QRS detection
- An ADC with at least 12-bit resolution to capture the AD8232 output signal
- A dedicated FPU (floating point unit) for real-time signal processing
- A communication protocol to interface with the computer
- ~5 kB of RAM

The STM32F334R8 satisfies all of these requirements, is well-priced, and also had an in-stock development board that was ready to order.

## Chip configuration


## External libraries

## Prototyping setup

<div align="center">

| Name | Description |
|---|---|
| [AD8232-EVALZ](https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/eval-ad8232.html) | AD8232 evaluation board |
| [NUCLEO-F334R8](https://www.st.com/en/evaluation-tools/nucleo-f334r8.html) | STM32 Nucleo-64 development board |
| [BOB-11570](https://www.sparkfun.com/products/11570) | SparkFun TRRS 3.5mm jack breakout |
| [LD1117-3.3 TO-220](https://www.st.com/resource/en/datasheet/ld1117.pdf) | 3.3 V linear voltage regulator |
| [CAB-12970](https://www.sparkfun.com/products/12970) | SparkFun 3.5 mm jack to 3-electrode cable |
| [3M 2560](https://www.3m.com/3M/en_US/medical-us/red-dot-ecg-electrodes/) | 3M Red Dot monitoring electrode pads |

</div>

## Idea graveyard
