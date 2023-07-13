<p align="center">
  <img src="https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/logo.png" alt="Project logo" width="200">
</p>

# ec-stingy
The EC(stin)G(y) is an end-to-end design and implementation of a cheap, compact, electrocardiogram that can be connected to a computer via USB and used to monitor a patient's cardiac signal and heart rate over time.

This project can serve as a boilerplate for any device that measures the electrical activity of the heart and be easily adapted to meet the user's needs. By open-sourcing the design files and code, the project aims to make medical hardware and technology more accessible and reduce the barrier to entry for prototyping. 

## Directory Overview
Each folder corresponds to a discrete stage of development or component of the final product.

[`algorithm/`](algorithm/) - A simulation suite for prototyping digital signal filtering and peak detection.

[`firmware/`](firmware/) - Contains the program loaded into the MCU on-board the PCB.

[`spice/`](spice/) - Another simulation suite for prototyping the ECG analog front-end.

[`pcb/`](pcb/) - Schematic, footprint, and manufacturing files for the printed circuit board.

[`gui/`](gui/) - A desktop program to view the cardiac signal and heart rate readings over time.

[`docs/`](gui/) - Datasheets, README graphics, and other miscellanea.

## License

ec-stingy is licensed under the [MIT License](LICENSE). All files (source code, documentation, plots, and animations) are authored by me unless otherwise stated.