# VEdirect

## Library to parse input from Victron VEdirect devices

Keyword / value pairs to be captured from the VEdirect frame could be configured for each device so just data you are interested in is stored. Captured data could be queried from buffer as a JSON string holding all fields or individual keys could be read as string, int, float depending on source data format.

Values transmitted e.g. as integers in mV will be converted float according to the number of digital places configured for the keys to be captured.

## library has been tested with the following devices

1. **Victron SmartSolar 75/15 MPTT charger** on serial line (see example *serialParseTest.cpp*)

2. **Victron SmartSolar 75/15 MPTT charger** test data captured with logic analyzer (see example *parseStringTest.cpp*)

3. **Victron Phoenix 12/1200 inverter** test data captured with logic analyzer (see example *parseStringTest.cpp*)