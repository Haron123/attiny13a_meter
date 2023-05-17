# attiny13a_meter  
Using the attiny13a to measure current  
  
This Was just a random project idea, the current measuring is not that precise ~(0.3ma) from my tests.  
Its also not intended for larger currents or voltages, it measures with the vcc voltage of the micro so you cant go past 5.5V   
The tiny runs at 9.6mhz 
  
It measures the vcc voltage it receives by itself during startup and then starts measuring current based on that and the   
voltage drop of the shunt resistor  
  
##  PIN SETUP
  PB4 as INPUT, has to be 1/10th of VCC this can be done via a voltage Divider  
	PB2 as INPUT, has to be connected via a shunt Resistor to VCC (This also determines SHUNT value)  
	PB1, PB3, PB0 Are Free to use (In this example used for a 4 Digit Display via I2C)    
  
  I quickly sketches how my circuit for this example code looks :
  ![alt text](https://i.imgur.com/a6SrCUI.png)
  
