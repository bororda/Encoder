# Soldering Iron
A Soldering iron controlled with Arduino via Encoder project.

# Blueprint:
Please check the [Materials](https://github.com/bororda/Encoder/tree/master/Materials) for more details.

# Parts used:
* DC-DC adapter
* MOSFET
* Arduino Nano v3.0
* 7 segment 3 digit display
* Encoder
* Comparator on LM358
* A few resistors/ chip sockets/ other trash...

# Specification:
* Input current: 19V, 3A (laptop's power adapter)
* Temp min 200 C: 
* Temp max 480 C: 
* Time 0-300 C: 

# Manual:
* CW turn adds aprox. 4 degrees C per tick
* CCW turn decreases the temperature by aprox. 4 degrees C per tick
* Press + Hold + CW turn adds aprox. 20 degrees C per tick
* Press + Hold + CCW turn decreases the temperature by aprox. 20 degrees C per tick
* Long press & hold (0.8 sec) activates temperature boost for 15 sec (OFF at the moment). The display shows the boost time left.
