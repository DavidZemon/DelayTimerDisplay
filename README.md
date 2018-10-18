Delay Timer Display
===================

Requirements
------------

* Default value for delay: 7.5 seconds
* Write delay to EEPROM to protect from power failure

### Output

* 1 x GPIO activated based on input (active high)
* WS2812 displaying red/green for active/inactive
* HD44780 controlled via serial (serial is a must for few pin connections)

### Input

* 1 x GPIO for activating relay
* 2 x GPIO, one each for increase and decrease delay in 0.1 second increments
