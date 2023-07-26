<p align="center">
  <img src="https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/logo.png" alt="Project logo" width="200">
</p>

# ec-stingy
The EC(stin)G(y) provides an end-to-end design and implementation of a cost-effective, compact electrocardiogram. It connects to a computer via USB, monitoring a patient's cardiac signal and heart rate over time.

This project can serve as a template for any device measuring the heart's electrical activity and can be easily adapted to the user's needs. By open-sourcing the design files and code, the project aims to make medical hardware and technology more accessible and reduce the barrier to entry for prototyping.

## Directory Overview
Each folder corresponds to a specific stage of development or a component of the final product.

[`algorithm/`](algorithm/) - A simulation suite for prototyping digital signal filtering and peak detection.

[`firmware/`](firmware/) - Contains the program loaded into the MCU on-board the PCB.

[`spice/`](spice/) - Another simulation suite for prototyping the ECG analog front-end circuitry.

[`pcb/`](pcb/) - Schematic, footprint, and manufacturing files for the printed circuit board.

[`gui/`](gui/) - A desktop program to view the cardiac signal and heart rate readings over time.

[`docs/`](docs/) - Datasheets, README graphics, cited research papers, and other miscellanea.

## License

ec-stingy is licensed under the [MIT License](LICENSE). Unless otherwise stated, the repository owner authors all content, including source code, schematics, documentation, plots, animations, etc.