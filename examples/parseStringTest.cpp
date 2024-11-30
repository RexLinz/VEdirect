// sample code fragments used for testing parsers
// with input data captured from serial log files on
// Victron SmartSolar 75/15 MPTT charger and
// Victron Phoenix 12/1200 inverter

#include <Arduino.h>
#include <VEdirect.h>

// helping function for testing only
// parse String, aborting if block is complete
bool VEdirectParse(VEdirect &device, const String line)
{
#if VERBOSE >= 3
  Serial.print("\r\nVEdirect::parse ");
  Serial.println(line);
#endif
  bool done = false;
  for (int i=0; i<line.length(); i++)
  {
    done = device.parse(line[i]);
    if (done) 
    {
#if VERBOSE >= 3
      Serial.println("stopped parsing of line (message complete)");
#endif
      break;
    }
  }
  return done;
}

// Victron SmartSolar 75/15 MPTT charger
void SmartSolarTest(void)
{   
    Serial.println("testing with data recorded on SmartSolar 75/15 MPTT charger");
    // all fields found on this charger
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
    VEdirect SmartSolar(SmartSolarKeys, false);

    // packet from SmartSolar 75/15 MPPT charger
    VEdirectParse(SmartSolar, "abcd"); // some garbage to be ignored
    VEdirectParse(SmartSolar, ":A0102000543\n"); // asynchronous hex message reporting device state 0x0201 = 0x05 (float)
    VEdirectParse(SmartSolar, "\r\nPID\t0xA053");
    VEdirectParse(SmartSolar, "\r\nFW\t163");
    VEdirectParse(SmartSolar, "\r\nSER#\tHQ2144VVVT4");
    VEdirectParse(SmartSolar, "\r\nV\t13260");
    VEdirectParse(SmartSolar, "\r\nI\t1830");
    VEdirectParse(SmartSolar, "\r\nVPV\t33650");
    VEdirectParse(SmartSolar, "\r\nPPV\t26");
    VEdirectParse(SmartSolar, "\r\nCS\t3");
    VEdirectParse(SmartSolar, "\r\nMPPT\t2");
    VEdirectParse(SmartSolar, "\r\nOR\t0x00000000");
    VEdirectParse(SmartSolar, "\r\nERR\t0");
    VEdirectParse(SmartSolar, "\r\nLOAD\tON");
    VEdirectParse(SmartSolar, "\r\nIL\t0");
    VEdirectParse(SmartSolar, "\r\nH19\t2552");
    VEdirectParse(SmartSolar, "\r\nH20\t3");
    VEdirectParse(SmartSolar, "\r\nH21\t34");
    VEdirectParse(SmartSolar, "\r\nH22\t3");
    VEdirectParse(SmartSolar, "\r\nH23\t21");
    VEdirectParse(SmartSolar, "\r\nHSDS\t50");
    bool valid = VEdirectParse(SmartSolar, "\r\nChecksum\tf");

    if (valid)
    {
        // SmartSolar.printRaw(Serial); // print raw fields recorded
        Serial.print("Product ID (hex string)  = "); Serial.println(SmartSolar.readString("PID"));
        Serial.print("Product ID (uint)        = "); Serial.println(SmartSolar.readU32("PID"));
        Serial.print("PV voltage (float)       = "); Serial.println(SmartSolar.readFloat("VPV"));
        Serial.print("battery current (float)  = "); Serial.println(SmartSolar.readFloat("I"));
        Serial.print("MPPT tracker state (int) = "); Serial.println(SmartSolar.readInt("MPPT"));
        Serial.print("load relais (string)     = "); Serial.println(SmartSolar.readString("LOAD"));
        // Serial.println(SmartSolar.asJson(false));
    }
    else
        Serial.println("no valid frame received");
}

// Victron Phoenix 12/1200 inverter
void PhoenixTest(void)
{   
    Serial.println("testing with data recorded on Phoenix 12/1200 inverter");
    // all fields found on this inverter
    const VEdirect::VEkey PhoenixKeys[] = {
        {"PID",      -1}, // product ID, 16 bit hex
        {"FW",        2}, // firmWare, x.yy
        {"SER#",     -1}, // serial number, string
        {"MODE",      0}, // device mode
        {"CS",        0}, // state of operation
        {"AC_OUT_V",  2}, // output voltage, 1/100 V
        {"AC_OUT_I",  1}, // output current, mA
        {"AC_OUT_S",  0}, // output apparent power, VA
        {"V",         3}, // battery coltage, mV
        {"AR",       -1}, // alarm reason, bitfield
        {"WARN",     -1}, // warn reason, bitfield
        {"OR",       -1}, // off reason, 32 bit hex
        {"Checksum", -2}  // end of block
    };
    VEdirect PhoenixInverter(PhoenixKeys, false);

    // packet from Phoenix 12/1200 inverter
    VEdirectParse(PhoenixInverter, "abcd"); // some garbage to be ignored
    VEdirectParse(PhoenixInverter, "\r\nPID\t0xA2F1");  // start of frame
    VEdirectParse(PhoenixInverter, "\r\nFW\t0124");
    VEdirectParse(PhoenixInverter, "\r\nSER#\tHQ2152UFKWY");
    VEdirectParse(PhoenixInverter, "\r\nMODE\t2");
    VEdirectParse(PhoenixInverter, "\r\nCS\t9");
    VEdirectParse(PhoenixInverter, "\r\nAC_OUT_V\t23007");
    VEdirectParse(PhoenixInverter, "\r\nAC_OUT_I\t1");
    VEdirectParse(PhoenixInverter, "\r\nAC_OUT_S\t38");
    VEdirectParse(PhoenixInverter, "\r\nV\t13232");
    VEdirectParse(PhoenixInverter, "\r\nAR\t0");
    VEdirectParse(PhoenixInverter, "\r\nWARN\t0");
    VEdirectParse(PhoenixInverter, "\r\nOR\t0x00000000");
    VEdirectParse(PhoenixInverter, "\r\nChecksum\t\xB1some extra chars"); // checksum = end of frame, anything else must be ignored

//    PhoenixInverter.printRaw();

    Serial.print("AC output voltage (as string)  = "); Serial.println(PhoenixInverter.readString("AC_OUT_V"));
    Serial.print("AC output voltage (float)      = "); Serial.println(PhoenixInverter.readFloat("AC_OUT_V"));
    Serial.print("AC output current (float)      = "); Serial.println(PhoenixInverter.readFloat("AC_OUT_I"));
    Serial.print("AC output apparent power (int) = "); Serial.println(PhoenixInverter.readInt("AC_OUT_S"));
    Serial.println(PhoenixInverter.asJson(false));
}

void setup() 
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("===================");
    Serial.println("VEdirect parse test");

    Serial.println();
    Serial.println("parsing SmartSolar MPPT charger data from string");
    SmartSolarTest();

    Serial.println();
    Serial.println("parsing Phoenix inverter data from string");
    PhoenixTest();

    while (1)
        ; // stop program
}

void loop()
{

}