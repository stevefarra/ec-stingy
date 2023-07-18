# SPICE simulations
## File overview
[`ecgsyn.m`](spice/ecgsyn_m), [`derivsecgsyn.m`](spice/derivsecgsyn.m) - These are MATLAB scripts sourced from [ECGSYN](https://physionet.org/content/ecgsyn/1.0.0/), a realistic ECG waveform generator open-sourced under the GNU General Public License and used in this project by `gen_ecg.m`.

[`gen_ecg.m`](spice/gen_ecg.m) - An Octave script for generating an ECG waveform suitable for transient analysis in LTspice.

[`AD8232.cir`](spice/AD8232.cir) - An LTspice model for the AD8232 chip, which is the analog front-end of choice for this project.

[`AD8232.asy`](spice/AD8232.asy) - An LTspice symbol for the AD8232.

[`AD8232_cm.asc`](spice/AD8232_cm.asc) - An LTspice schematic for the AD8232 in the cardiac monitor configuration specified in the datasheet. This file contains simulation commands for both frequency and transient analyses.

We've chosen to use an integrated chip, the AD8232, to interface with our MCU and provide it with a signal suitable for its ADC. Fortunately, its datasheet provides us with several configurations depending on our application, and the cardiac monitor design requires minimal modification to be used in our own project. 

The component values pertaining to the filter characteristics might be re-worked in the future, so in this section we'll provide analysis and situation that would make future modification easier.



The amplitude of an ECG signal is typically about ~1 mV, which is a resolution that most at-home oscilloscopes can't reach

Plot of the simulated signal being input to the AD8232:
![Raw ECG](../docs/visuals/ecg_raw.png)

Plot of the output:
![Output ECG](../docs/visuals/ecg_out.png)
The filtering has managed to remove the baseline wandering while preserving the shape of the P and T waves and amplifying the signal by a factor of roughly 10³. 

And the bode plot:
![SPICE bode plot](../docs/visuals/spice_bode.png)

## Idea Graveyard
**Build an analog front-end from scratch**: The idea of building the required circuitry using discrete ICs was considered, but the bill of materials just for core functionality -- an in-amp, op-amps, and switches -- quickly exceeded the current price for an AD8232, not to mention the jump in complexity. Barring future supply chain issues, this approach was quickly deemed unnecessary.

**Widen the filter passband**: Finding literature on best practices for ECG signal conditioning is surprisingly difficult. One of the few resources I did find was [an article from MEDTEQ](https://www.medteq.net/article/2017/4/1/ecg-filters), whose recommendations align pretty closely with the datasheet. They provide a diagnostic passband for motionless, low-noise environments of 0.05 - 150 Hz. The lowered high pass cut-off prevents ST segment distoration as [this 2012 paper](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3388307/) points out. The narrower pass band was opted for to make the device more tolerant to noise, but if one wants to use this wider band they should consider another circuit topology that requires cheaper components to reach this lower high pass cut-off or make use of the digital domain.