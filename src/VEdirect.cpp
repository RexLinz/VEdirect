#include "VEdirect.h"

// VERBOSE 0: no debugging output
// VERBOSE 1: just error messages
// VERBOSE 2: + unparsed names (e.g. for setup of new device)
// VERBOSE 3: + show parsing progress (trace level)

#define VERBOSE 2

VEdirect::VEdirect(const VEkey *VEkeys, bool retainValues) : 
    keys(VEkeys),
    retain(retainValues),
    nFrameErrors(0),
    nFramesOK(0),
    state(waitCR),
    valid(false)
{
    for (numKeys = 0; numKeys<MAX_KEYS; numKeys++)
    {
        if (keys[numKeys].digits == -2)
            break; // found end of block marker
    }
    if (numKeys < MAX_KEYS)
    {
        #if VERBOSE >= 2
            Serial.print("VE direct watching for ");
            Serial.print(numKeys);
            Serial.println(" keywords");
        #endif
    }
    else
    {
        numKeys = 0; // mark as not valid
#if VERBOSE >= 0
        Serial.println("keys specified not valid (not including trigger key)");
#endif                
    }
    values = new String[numKeys];
    tempValues = new String[numKeys];
}

void VEdirect::setRetain(bool retainValues) 
{ 
    retain = retainValues;
}

int VEdirect::findKey(const String name)
{
    int index;
    for (index = 0; index < numKeys; index++)
    { // search for known keyword
        if (name == keys[index].name)
            return index; // found
    };
    return -1; // not found
}

// assemble a line from input, parse on newline
// TODO exclude Victron HEX packets from parsing
bool VEdirect::parse(char c)
{
    if (c == ':') // a binary message can interrupt a text message at any time
    { 
#if VERBOSE >= 2
        Serial.print("\r\nbinary message '");
#endif
        state = binMessage; // ignore binary message, reset parser
    }
    switch(state)
    {
        case binMessage:
            if (c == '\n') // end of binary message, resart text parser 
            {
#if VERBOSE >= 2
                Serial.print("'\r");
#endif
                state = waitCR;
            }
#if VERBOSE >= 2
            Serial.print(c);    
#endif
            break;
        case waitCR:
            if (c == '\r') // a new line might start a new block of data (if it is the first one)
            {
                // clear temporary data
                for (int i=0; i<numKeys; i++)
                    tempValues[i] = "";
                chksum = c;
                state = waitLF;
#if VERBOSE >= 3
                Serial.println("VEdirect::parse starting block");
#endif
            }
            break;
        case waitLF: // every CR to be followed by LF
            chksum += c; // update checksum
            if (c != '\n')
            {
                nFrameErrors++; // increment error counter
                state = waitCR; // invalid, reset parser
            }
            else
            {
                state = getName;
                name = "";
            }
            break;
        case getName: // name is first part of each record in block 
            chksum += c; // update checksum
            if (c == '\t') // name completed, data or checksum to follow
            {
                if (name == keys[numKeys].name) // checksum 
                { // is checksum -> identifying end of block
                    state = getChksum;
                }
                else
                {
                    keyIndex = findKey(name);
                    if (keyIndex < 0)
                    { // name not found in list, ignore value
                        state = ignoreValue;
#if VERBOSE >= 2
                        Serial.print(name);
                        Serial.println(": ignoreed");
#endif
                    }
                    else 
                    { // found keyword we want to record the value in list
                        value = "";
                        state = getValue;
#if VERBOSE >= 3
                        Serial.print(name);
                        Serial.println(": recorded");
#endif
                    }
                }
            }
            else if (isPrintable(c))
                name += c; // name must not hold control characters
            else // reset parser
            {
                nFrameErrors++; // increment error counter
                state = waitCR; // anything else will reset parsing
#if VERBOSE >= 1
                Serial.println("name with invalid characters");
#endif
            }
            break;
        case getValue: // anything to the next CR should be part of the value
            chksum += c; // update checksum
            if (c == '\r')
            { // parameter value completed
                tempValues[keyIndex] = value; // save value
                state = waitLF; // we expect a LF next
#if VERBOSE >= 3
                Serial.print(keys[keyIndex].name);
                Serial.print(" = ");
                Serial.println(value);
#endif
            }
            else if (isPrintable(c))
                value += c; // assemble value
            else
            {
                nFrameErrors++; // increment error counter
                state = waitCR; // anything else will reset parsing
#if VERBOSE >= 1
                Serial.println("value with invalid characters");
#endif
            }
            break;
        case ignoreValue:
            chksum += c; // update checksum
            if (c == '\r')
                state = waitLF; // we expect a LF next
            else if (!isPrintable(c))
            {
                nFrameErrors++; // increment error counter
                state = waitCR; // anything else will reset parsing
#if VERBOSE >= 1
                Serial.println("value with invalid characters");
#endif
            }
            break;
        case getChksum: // name "checksum" is followed by a single value
            chksum += c; // update checksum
            bool tempValid = (chksum == 0); // must result in 0 over full block
#if VERBOSE >= 3
            Serial.print("checksum calculated ");
            Serial.println(chksum, HEX);
#endif
#if VERBOSE >= 1
            if (!tempValid)
                Serial.println("Checksum error");
#endif
            if (tempValid) // copy to public data
            {
                nFramesOK++; // increment frames OK counter
                for (int i=0; i<numKeys; i++)
                {
                    if (tempValues[i].length() > 0)
                        values[i] = tempValues[i]; // new data available
                    else if (!retain)
                        values[i] = ""; // clear existing data
                }
                valid = true;
            }
            else if (!retain)
            {
                for (int i=0; i<numKeys; i++)
                    values[i] = "";
                valid = false; // invalid data and not retaining
            }
            state = waitCR; // restart parsing
            return valid;
            break;
    }
    return false; // always return false if not completing a block
}

// non blocking parser from any input stream
// aborting if a valid frame has been read
bool VEdirect::parse(Stream& s)
{
    while (s.available())
    {
        bool done = parse(s.read());
        if (done)
            return true; // abort parsing after full and valid message has been received
    }
    return false;
}

// return frameing errors counter
uint VEdirect::numFrameErrors()
{
    return nFrameErrors;
}

// return counter of frames received OK
uint VEdirect::numFramesOK()
{
    return nFramesOK;
};

// return true if data in (public) buffer is complete and valid
bool VEdirect::dataValid()
{
    return valid;
}

// check if field name is available and has valid data, return ...
// -3 = value empty
// -2 = name not available
// -1 = data not valid
//  0... index to value
int VEdirect::hasField(const String name)
{
    if (!valid) // data valid?
    {   
#if VERBOSE >= 3
        Serial.println("data not valid");
#endif
        return -1;
    }
    int index = findKey(name);
    if (index < 0) // name available?
    {   
#if VERBOSE >= 3
        Serial.println("name not available");
#endif
        return -2; 
    }
    if (values[index].length() == 0) // value not empty?
    {
#if VERBOSE >= 3
        Serial.println("value empty");
#endif
        return -3;
    }
    return index;
}

String VEdirect::readString(const String name)
{
    int index = hasField(name);
    if (index < 0)
        return "";
    return values[index];
}

int VEdirect::readInt(const String name)
{
    int index = hasField(name);
    if (index < 0)
        return 0;
    if (keys[index].digits != 0)
    {
#if VERBOSE >= 3
        Serial.println("not an integer");
#endif
        return 0;
    }
    if (values[index].startsWith("0x"))
        return strtol(values[index].substring(2).c_str(), nullptr, 16);
    else
        return values[index].toInt();
}

uint32_t VEdirect::readU32(const String name)
{
    int index = hasField(name);
    if (index < 0)
        return 0;
    if (keys[index].digits != 0)
    {
#if VERBOSE >= 3
        Serial.println("not an integer");
#endif
        return 0;                
    }
    if (values[index].startsWith("0x"))
        return strtoul(values[index].substring(2).c_str(), nullptr, 16);
    else
        return values[index].toInt();
}

float VEdirect::readFloat(const String name)
{
    int index = hasField(name);
    if (index < 0)
        return NAN;
    if (keys[index].digits < 0)
    {
#if VERBOSE >= 3
        Serial.println("not a number");
#endif
        return NAN;
    }
    float value = values[index].toInt(); // read raw as int
    for (int i=0; i<keys[index].digits; i++)
    {
        value /= 10.0f; // respect number of decimals
    }
    return value;
}

// deal with undefined values depending on allFields
// retain = 1 -> return null
// retain = 0 -> exclude from string returned
String VEdirect::asJson(bool allFields)
{
    String jsonString = "";
    for (int i=0; i<numKeys; i++)
    {
        String jsonLine = "";
        if (values[i].length() > 0)
        { // value is available
            jsonLine += "\"" + keys[i].name + "\":";
            if (keys[i].digits < 0) // as string
                jsonLine += "\""  + values[i] + "\"";
            else if (keys[i].digits == 0) // integer
                jsonLine += values[i];
            else // floating point 
            {
                float value = readFloat(keys[i].name);
                jsonLine += String(value, keys[i].digits);
            }
        }
        else        
        {
#if VERBOSE >= 3
            Serial.print(keys[i].name);
            Serial.println(": no value available");
#endif
            if (allFields)
            { // mark as null
                jsonLine = "\"" + keys[i].name + "\":null";
            }
        }
        if ((jsonString.length() > 0) && (jsonLine.length() > 0))
            jsonString += ",\n"; // data is already existing, add as next field
        jsonString += jsonLine; // add actual data
    }
    return "{\n" + jsonString + "\n}";
}

bool VEdirect::printRaw(Stream &s)
{
    if (valid)
    {
        for (int i=0; i<numKeys; i++)
        {
            s.print(keys[i].name);
            s.print(" = ");
            s.println(values[i]);
        }
    }
  return valid;
}

VEdirect::AlarmWarnReasonBits VEdirect::AlarmReason(void)
{
    union 
    {
        int16_t reason;
        AlarmWarnReasonBits bits;
    } a;
    a.reason = readInt("AR");
    return a.bits;
}

VEdirect::AlarmWarnReasonBits VEdirect::WarnReason(void)
{
    union 
    {
        int16_t reason;
        AlarmWarnReasonBits bits;
    } a;
    a.reason = readInt("WARN");
    return a.bits;
}

VEdirect::OffReasonBits VEdirect::OffReason(void)
{
    union 
    {
        uint32_t reason;
        OffReasonBits bits;
    } a;
    a.reason = readU32("OR");
    return a.bits;
}
