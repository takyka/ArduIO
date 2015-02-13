# ArduIO
sketch, to control arduino pheripherals via serial (USB)

Device has an address listening on. It is stored in EEPROM at address 0.
Command format:
start of message "@" followed by device address,command,peripheral type,peripheral address, optional ":",data
Command has to be closed with newline character.
example commands and responses:
read digital input 2 on device 12
@12RD2
response:
@12D2:0
read analog0 on device 12
@12RA0
response:
@12A0:123
read counter0 (pin2)
@12RC0
response:
@12C0:123456
write analog 123 on pin 3 (pwm)
@12WA3:123
response:
@12A3:123
read cycletime in microseconds on counter1 (pin3) cycletime of the PWM initiated above
@12RP1
response:
@12P1:2040


Valid commands are:
R (read), W (write)
for read you can use peripherals:
D (digital pin 0-19  A0=14...A5=19 return value: 0-1)
A (analog pin A0-A7 return value:0-1023)
C (counter 0->pin2, 1->pin3 return value:0-max unsigned long)
E (eeprom address 0-511  N.B. 0 is the device address listening to! return value:0-255)
P (cycletime of pulses on counter 0&1 in microseconds, measured from rising edge to rising edge return value:0?-max unsigned long)

for write you can use:
D (digital output adresses as at read valid data:0-1)
A (3,5,6,9,10,11 PWM output valid data:0-255)
E (eeprom write address range like at read, valid data:0-255)
C (preset counter address 0-1 valid data:0-max long)
I (pin direction address like at digital inputs, valid data:I,O (input, output)


