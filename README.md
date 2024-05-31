# Monitoring-bpm-and-spo2

This codes were made for an Esp32-C3 (XIAO SEEED), using a digital pulseoximeter MAX30102.

## Contents of the repository:

### [Code for the Infant device](bpm_then_spo2_all.ino)
In this section you can find the complete code for the infant device (sender).
  > Calculates data and send it to caretaker device.

### Code for the Caretaker device
In this section you can find the complete code for the caretaker device (receptor).
  > Recieves data and manage outliers.<br />
  > Generates alarms according to certain parameters

### Code for the Analog device
In this section you can find the complete code for the analog device (pulse-oximeter).
  > Calculates bpm and spo2.

### R Code for statistical analysis of SpO2 and BPM values across 3 devices
This section is divided into 3 parts:<br />
*1. .csv data of SpO2 and BPM for the 20 people in the 3 different pulseoximeter devices*<br />
*2. R code for the analysis*<br />
  >Concatenates different .csv files<br />
  >Boxplots per participant including mean visualization<br />
  >Violin plots of the entire dataset<br />
  >ANOVA and post-hoc tests for the data<br />
  
*3. Images of different Results:*<br />
  >Boxplots<br />
  >Violin plots<br />
  >ANOVA<br />
  >Post-hoc test
