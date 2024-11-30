# VEdirect

## Library to parse input from Victron VEdirect devices

Keyword / value pairs to be captured from the VEdirect frame could be configured for each device so just data you are interested in is stored. Captured data could be queried from buffer as a JSON string holding all fields or individual keys could be read as string, int, float depending on source data format.

Values transmitted e.g. as integers in mV will be converted float according to the number of digital places configured for the keys to be captured.

**NOTE**  

1. Level of debugging output to serial console could be defined in top of VEdirect.cpp  
2. At the moment the library parses ASCII messages only. HEX messages are logged to console if debugging level is set to 2 or 3

Implementation is based on Victron's VEdirect protocol specification
[VE Direct Protocol-3.33.pdf](https://www.victronenergy.com/upload/documents/VE.Direct-Protocol-3.33.pdf)

## Library has been tested with the following devices

1. **Victron SmartSolar 75/15 MPTT charger**  
Connected to serial line (see example *serialParseTest.cpp*)

2. **Victron SmartSolar 75/15 MPTT charger**  
Using test data captured with logic analyzer (see example *parseStringTest.cpp*)

3. **Victron Phoenix 12/1200 inverter**  
Using test data captured with logic analyzer (see example *parseStringTest.cpp*)
