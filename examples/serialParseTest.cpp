// sample code parsing VEdirect from serial input
// tested with Victron SmartSolar 75/15 MPTT charger

#include <Arduino.h>
#include <VEdirect.h>

// Victron SmartSolar MPPT charger or Phoenix inverter connection
// Serial parameters 19200,8,N,1
//         VEdirect <-> ESP32
// JST PH 2.0 / Wuerth 620004113322 WR-WTB 2.00 mm, 4 pins 
//  +---+
//  | 4 |  VCC (5V output, maximum load not defined)
//    3 |  TX   ->  UART0RX = 3 / UART2RX = 16 (via 10k series resistor or 1.8k / 3.3k voltage divider)
//    2 |  RX   <-  UART0TX = 1 / UART2TX = 17 (unused at the moment)
//  | 1 |  GND  --  GND
//  +---+

// keys for SmartSolar MPPT charger
const VEdirect::VEkey SmartSolarKeys[] = {
    {"PID",       0}, // product ID, 16 bit hex
    {"FW",        2}, // firmWare, x.yy
    {"SER#",     -1}, // serial number, string
    {"V",         3}, // battery coltage, mV
    {"I",         3}, // battery current, mA
    {"VPV",       3}, // panel voltage, mV
    {"PPV",       0}, // panel power, W
    {"CS",        0}, // charging state
    {"MPPT",      0}, // MPPT tracker state
    {"OR",        0}, // off reason, 32 bit hex
    {"ERR",       0}, // error code
    {"LOAD",     -1}, // load switch (ON/OFF)
    {"IL",        3}, // load current, mA    
    {"H19",       2}, // yield total, 1/100 kWh
    {"H20",       2}, // yield today, 1/100 kWh
    {"H21",       0}, // maximum power today, W
    {"H22",       2}, // yield yesterday, 1/100 kWh
    {"H23",       0}, // maximum power yesterday, W
    {"HSDS",      0}, // day sequence number (0...364)
    {"Checksum", -2}  // end of block
};

// parser object
VEdirect SmartSolar(SmartSolarKeys, false);

void setup() 
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("========================");
    Serial.println("ESP MQTT client starting");
    Serial.println();
    while (Serial.available())
        Serial.read(); // clear input

    Serial2.begin(19200); // VEdirect device
}

void loop() 
{
    // use input from console to trigger some binary communication to VEdirect device
    if (Serial.available()) 
    {
        char c = Serial.read();
        switch (c)
        { 
        // NOTE any request sent will keep Victron device talking binary for about 2 minutes
        case 'e': // read frame error counter
            Serial.println("  " + String(SmartSolar.numFrameErrors()) + " frames with errors");
            break;
        case 'f': // read total number of frames received
            Serial.println("  " + String(SmartSolar.numFramesOK()) + " frames received OK");
            break;
        case 'I': // binary message: query product ID
            Serial2.print(":451\n");
            break;
        case 'P': // binary message: send Ping request
            Serial2.print(":154\n"); 
            break;
        }
        Serial.println();
    }
    // parse SmartSolar and print results to Serial
    if (SmartSolar.parse(Serial2)) // parse input from VEdirect device
    {
        // SmartSolar.printRaw(Serial); // print raw fields recorded
        Serial.println("");
        Serial.print("Product ID (hex string)  = "); Serial.println(SmartSolar.readString("PID"));
        Serial.print("Product ID (uint)        = "); Serial.println(SmartSolar.readU32("PID"));
        Serial.print("PV voltage (float)       = "); Serial.println(SmartSolar.readFloat("VPV"));
        Serial.print("battery current (float)  = "); Serial.println(SmartSolar.readFloat("I"));
        Serial.print("MPPT tracker state (int) = "); Serial.println(SmartSolar.readInt("MPPT"));
        Serial.print("load relais (string)     = "); Serial.println(SmartSolar.readString("LOAD"));
        // Serial.println(SmartSolar.asJson(false)); // all captured fields as JSON string
    }
}
