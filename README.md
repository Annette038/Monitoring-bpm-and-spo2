# Monitoring-bpm-and-spo2

This codes were made for an Esp32-C3 (XIAO SEEED), using a digital pulseoximeter MAX30102.

## Contents of the repository:

### [Code for the Infant device](InfantDevice.ino)
In this section you can find the complete code for the infant device (sender).
  > Calculates data and send it to caretaker device.

### [Code for the Caretaker device](CarerDevice.ino)
In this section you can find the complete code for the caretaker device (receptor).
  > Recieves data and manage outliers.<br />
  > Generates alarms according to certain parameters

### [Code for the Analog device](Analog.ino)
In this section you can find the complete code for the analog device (pulse-oximeter).
  > Calculates bpm and spo2.

### R Code for statistical analysis of SpO2 and BPM values across 3 devices
This section is divided into 3 parts:<br />

*1. .csv data of SpO2 and BPM for the 20 people in the 3 different pulseoximeter devices*<br />
  >[Find here the data of the 20 people](PulseOx)

*2. R code for the analysis*<br />
  >Concatenates different .csv files<br />
  >Boxplots per participant including mean visualization<br />
  >Violin plots of the entire dataset<br />
  >ANOVA and post-hoc tests for the data<br />
  >[Find here the entire code used](PulseOx/PulseOx.R)
  
*3. Images of different Results:*<br />
  >[Boxplots](PulseOx/Boxplots)<br />
  >[Violin plots](PulseOx/ViolinPlots)<br />
  >[ANOVA](PulseOx/ANOVA)<br />
  >[Post-hoc test](PulseOx/Post_hoc_tests)
