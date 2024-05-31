# Monitoring-bpm-and-spo2

This codes were made for an Esp32-C3 (XIAO SEEED), using a digital pulseoximeter MAX30102.

## Contents of the repository:

### [Code for the Infant device]()
Calculates data and send it to caretaker device.

### Code for the Caretaker device
Recieves data and manage outliers.<br />
Generates alarms.

### Code for the Analog device
Calculates bpm and spo2.

### R Code for statistical analysis of SpO2 and BPM values across 3 devices
  >Concatenates different .csv files
  >Boxplots per participant including mean visualization
  >Violin plots of the entire dataset
  >ANOVA and post-hoc tests for the data

[Lets go to BPM](bpm_then_spo2_all.ino)


