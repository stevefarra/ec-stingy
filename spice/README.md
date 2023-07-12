# SPICE simulations
## File overview
[`ecgsyn.m`](spice/ecgsyn_m), [`derivsecgsyn.m`](spice/derivsecgsyn.m) - These are MATLAB scripts sourced from [ECGSYN](https://physionet.org/content/ecgsyn/1.0.0/), a realistic ECG waveform generator open-sourced under the GNU General Public License and used in this project by `gen_ecg.m`.

[`gen_ecg.m`](spice/gen_ecg.m) - An Octave script for generating an ECG waveform suitable for transient analysis in LTspice.

[`AD8232.cir`](spice/AD8232.cir) - An LTspice model for the AD8232 chip, which is the analog front-end of choice for this project.

[`AD8232.asy`](spice/AD8232.asy) - An LTspice symbol for the AD8232.

[`AD8232_cm.asc`](spice/AD8232_cm.asc) - An LTspice schematic for the AD8232 in the cardiac monitor configuration specified in the datasheet. This file contains 

Plot of the simulated signal being input to the AD8232:
![Raw ECG](../docs/visuals/ecg_raw.png)
Plot of the output:
![Output ECG](../docs/visuals/ecg_out.png)
And the bode plot:
![SPICE bode plot](../docs/visuals/spice_bode.png)
