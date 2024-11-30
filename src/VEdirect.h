#ifndef _VEDIRECT_H_
#define _VEDIRECT_H_

#include <Arduino.h>

class VEdirect 
{
public:
    // structure defining keywords to watch and data types
    // typically Victron product field names are all upper case (except "Checksum")
    // digits: 
    //  1... number of fractional digits
    //  0 = integer (no fractional digits)
    // -1 = String
    // -2 = end of block mark, name must be "Checksum"
    typedef struct {String name; int digits;} VEkey; 
    // initialize to use specified key names to record, keep older values if retainValues = true
    VEdirect(const VEkey *VEkeys, bool retainValues=true);
    void setRetain(bool retainValues);
    // parse functions return true if a full message has been successfully received
    bool parse(char c);    // single character
    bool parse(Stream &s); // non blocking read from selected stream, e.g. serial
    uint numFrameErrors(); // counter of framing errors
    uint numFramesOK();    // counter of frames received OK
    bool dataValid();      // return true if a valid block has been received
    // access to data once valid package is complete 
    int hasField(const String name);      // return name index if data available (data valid, name existing, value not empty)
    String readString(const String name); // read any value as string (raw format for floats)
    int readInt(const String name);       // read value as int, 0 if not valid
    uint32_t readU32(const String name);  // read hex value (e.g. 0x3df56ac8)
    float readFloat(const String name);   // read value as float, NAN if not valid
    // TODO add readHEX function uint32_t readHex(const String name); // read hexadecimal field as int32
    String asJson(bool allFields=false);  // return null for undefined fields if allFields is true, else skip them
    bool printRaw(Stream &s=Serial);      // print all values to stream as string, return dataValid condition
    // alarm reason (AR) and warning reason (WARN) bitfied
    typedef struct {
        bool lowVoltage              : 1; // 1
        bool highVoltage             : 1; // 2
        bool lowSOC                  : 1; // 4
        bool lowStarterVoltage       : 1; // 8
        bool highStarterVoltage      : 1; // 16
        bool lowTemperature          : 1; // 32
        bool highTemperature         : 1; // 64
        bool midVoltage              : 1; // 128
        bool overload                : 1; // 256
        bool DCripple                : 1; // 512
        bool lowVACout               : 1; // 1024
        bool highVACout              : 1; // 2048
        bool shortCircuit            : 1; // 4096
        bool BMSlockout              : 1; // 8192
    }  AlarmWarnReasonBits;
//    AlarmWarnReasonBits &AlarmWarnReason(int16_t &r) { return (AlarmWarnReasonBits&) r; }; // cast to bitfield
    AlarmWarnReasonBits AlarmReason(void); // return "AR" as bitfield 
    AlarmWarnReasonBits WarnReason(void);  // return "WARN" as bitfield
    // off reason (OR) bitfield
    typedef struct {
        bool noInputPower            : 1; // 1
        bool powerSwitchOff          : 1; // 2
        bool deviceSwitchedOff       : 1; // 4
        bool remoteInput             : 1; // 8
        bool protectionActive        : 1; // 16
        bool paygo                   : 1; // 32
        bool BMS                     : 1; // 64
        bool engineShutdownDetection : 1; // 128
        bool analyzingInputVoltage   : 1; // 256
    } OffReasonBits;
    OffReasonBits OffReason(void); // return "OR" as bitfield
private:
    bool retain;                          // retain values over blocks
    uint nFrameErrors;
    uint nFramesOK;
    // VEdirect specified max. 22 records per block, but data might be split into several blocks
    const int MAX_KEYS = 50;              // maximum number of keys accepted
    int numKeys;                          // number of keys to check
    const VEkey *keys;                    // pointer to key names/digits
    String *values;                       // data received
    bool valid;                           // data is valid?
    String *tempValues;                   // buffered data, copied to values if block is valid
    enum parserState {waitCR, waitLF, getName, getValue, ignoreValue, getChksum, binMessage} state; // state machine
    String name;                          // temporary field name, max. 9 characters
    String value;                         // temporary value, max. 33 characters
    uint8_t chksum;                       // updated while receiving a block
    int keyIndex;                         // used to store index while parsing name/value pairs
    int findKey(const String name);       // check a key is in list
};

#endif
