// -------------------------------------------------------------------------------------------------------------------------------------------
//  Table of Contents
// -------------------------------------------------------------------------------------------------------------------------------------------
//  1.  Includes and Type Definitions
//  2.  Global Variables
//  3.  ImGui Conditionals Preprocessing for Windows and Exports
//  4.  IP Class
//  5.  SubnetMask Class
//  6.  ChangingIP Class
//  7.  Subnet Class
//  8.  Functions for Client Code

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: Includes and Type Definitions
// -------------------------------------------------------------------------------------------------------------------------------------------

#pragma once
#include <iostream>
#include <string.h>
#include <cmath>
#include <regex>
#include <time.h>
#include <fstream>
using namespace std;

typedef union IPNumeric {
    uint32_t IP32;
    uint8_t octets[4];
} IPNumeric;

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: Global Variables
// -------------------------------------------------------------------------------------------------------------------------------------------

// IP cursors.
unsigned long long totalAddedToIP = 256; 

// Flags and vectors.
char **globalArgumentVector; 
string programName;
bool binaryFlag  = false;
bool debugFlag   = false;
bool reverseFlag = false;
ofstream exportFileStream;

// Other conditionals.
bool subnettingSuccessful = false; 

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: ImGui Conditionals Preprocessing for Windows and Exports
// -------------------------------------------------------------------------------------------------------------------------------------------

#ifdef IMGUI_API // If ImGui included, use ImGui::Text to render output. Otherwise, send output to cout.
extern const int totalNumberOfWindows;
extern bool windowsAreOpen[];

void nonExportOutput(const char *string) {
    ImGui::Text("%s", string);
}

void usage(const char *message) {
    ImGui::Text("%s", message);
}
#else
void nonExportOutput(const char *string) {
    cout << string << endl;
}

void usage(const char *message) {
    cout << message << endl; 
    exit(0);
}
#endif

void exportOutput(const char *string) {
    exportFileStream << string;
}

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: IP Class
// -------------------------------------------------------------------------------------------------------------------------------------------

class IP {
public:
    IPNumeric IPAddress;
    string IPString;
    string IPBinaryString;
    char IPClass;
    static const uint32_t classBStartBinary = (const uint32_t)(0b1U    << 31); // Binary maps for classful address resolution. Class A implicitly starts at 0.
    static const uint32_t classCStartBinary = (const uint32_t)(0b11U   << 30);
    static const uint32_t classDStartBinary = (const uint32_t)(0b111U  << 29);
    static const uint32_t classEStartBinary = (const uint32_t)(0b1111U << 28);
    bool invalidFormat = false;
    friend ostream &operator<<(ostream &stream, IP IPArg);

    IP() {} // Necessary for prototypes to work.

    IP(string stringArg) {
        if (isIPFormat(stringArg)) {
            this -> IPAddress = StringToIPInt(stringArg);
            this -> IPString = stringArg;
        } else if (isIPNumber(stringArg)) {
            this -> IPAddress.IP32 = static_cast<uint32_t>(stoi(stringArg));
            this -> IPString = intToIPString(this -> IPAddress);
        } else {
            this -> IPString = "0.0.0.0";
            this -> IPAddress.IP32 = 0;
            this -> invalidFormat = true;
        }
        this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
        setIPClass(this);
    }

    IP(uint32_t IPArg) {
        this -> IPAddress.IP32 = IPArg;
        this -> IPString = intToIPString(this -> IPAddress);
        this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
        setIPClass(this);
    }

    void setIPClass(IP *constructedObject) {
        if (constructedObject -> IPAddress.IP32 < classBStartBinary) {
            constructedObject -> IPClass = 'A';
        } else if (constructedObject -> IPAddress.IP32 < classCStartBinary) {
            constructedObject -> IPClass = 'B';
        } else if (constructedObject -> IPAddress.IP32 < classDStartBinary) {
            constructedObject -> IPClass = 'C';
        } else if (constructedObject -> IPAddress.IP32 < classEStartBinary) {
            constructedObject -> IPClass = 'D';
        } else {
            constructedObject -> IPClass = 'E';
        }
    }

    static bool isIPFormat(string testString) {
        smatch matches;
        regex IPPattern("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
        return regex_search(testString, matches, IPPattern);
    }

    static bool isCIDRMask(string testString) {
        smatch matches;
        regex CIDRPattern("^\\d{1,2}$");
        return regex_search(testString, matches, CIDRPattern) && (32 >= stoi(testString) && stoi(testString) >= 0);
    }

    static bool isIPNumber(string testString) {
        smatch matches;
        regex IPNumberPattern("^\\d{1,10}$");
        return regex_search(testString, matches, IPNumberPattern) && (testString == to_string(static_cast<uint32_t>(stoi(testString))));
    }

    static IPNumeric StringToIPInt(string stringArg) {
        IPNumeric returnValue;
        IPNumeric CIDRConversion;
        string operatingString;
        string currentOctet;
        if (isIPFormat(stringArg)) {
            operatingString = stringArg;
        } else if (isCIDRMask(stringArg)) {
            CIDRConversion.IP32 = ~static_cast<uint32_t>(((1<<(32-stoi(stringArg)))-1));
            operatingString = intToIPString(CIDRConversion);
        } else {
            returnValue = {0};
            return returnValue;
        }
        for (int i=3; i>=0; i--) {
            currentOctet = operatingString.substr(0, operatingString.find('.'));
            operatingString.erase(0, operatingString.find('.') + 1);
            returnValue.octets[i] = static_cast<uint8_t>(stoi(currentOctet));
        }
        return returnValue;
    }

    static string intToIPString(IPNumeric IPArg) {
        string returnString = "";
        for (int i=3; i>0; i--) {
            returnString.append(to_string(IPArg.octets[i]));
            returnString.append(".");
        }
        returnString.append(to_string(IPArg.octets[0]));
        return returnString;
    }

    template <typename T>
    static string toBinaryString(T numArg) {
        string returnString = "";
        int maxBitSize = sizeof(T) * 8 - 1;
        for (int i=maxBitSize; i>=0; i--) {
            if (numArg - (1<<i) >=0) {
                numArg = static_cast<T>((static_cast<int>(numArg) - (1<<i)));
                returnString.append("1");
            } else {
                returnString.append("0");
            }
        }
        return returnString;
    }

    static string toIPBinaryString(IPNumeric IPArg) {
        string returnString = "";
        for (int i=3; i>=0; i-=1) {
            returnString += toBinaryString<uint8_t>(IPArg.octets[i]);
            returnString += ".";
        }
        return returnString.erase(returnString.length() - 1);
    }

    friend ostream &operator<<(ostream &stream, IP IPArg) {
        if (binaryFlag) {
            return stream << IPArg.IPBinaryString;
        } else {
            return stream << IPArg.IPString;
        }
    }

    IP operator+(uint32_t operand) {
        return IP(this->IPAddress.IP32 + operand);
    }

    IP operator-(uint32_t operand) {
        return IP(this->IPAddress.IP32 - operand);
    }

    IP operator&(IP operand) {
        return IP(this->IPAddress.IP32 & operand.IPAddress.IP32);
    }

    void operator+=(uint32_t operand) {
        *this = (*this + operand);
    }

    void operator-=(uint32_t operand) {
        *this = (*this - operand);
    }

    void operator&=(uint32_t operand) {
        *this = (*this & operand);
    }

};

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: SubnetMask Class
// -------------------------------------------------------------------------------------------------------------------------------------------

class SubnetMask : public IP {
public:
    unsigned long long blockSize;
    unsigned long long numberOfSubnets;
    uint32_t hostBits;
    uint32_t networkBits;

    SubnetMask() {}
    
    SubnetMask(string stringArg) : IP(stringArg) {
        if (isCIDRMask(stringArg)) {
            this -> IPAddress.IP32 = CIDRToIPNumber(stoi(stringArg));
            this -> IPString = intToIPString(this -> IPAddress);
            this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
        } else if (!stringArg.compare("0.0.0.0")) {
            this -> IPAddress.IP32 = 0;
            this -> IPString = stringArg;
            this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
        }
        this -> hostBits = fetchHostBits(this -> IPAddress.IP32);
        if (this -> hostBits == 32) {
            this -> blockSize = 4294967296ULL;
        } else if (!invalidFormat) {
            this -> blockSize = 1ULL<<(this -> hostBits);
        } else {
            this -> blockSize = 0;
        }
        this -> networkBits = 32 - hostBits;
        this -> numberOfSubnets = 1ULL<<networkBits;
        if (!verifyMask(*this) && this->IPString.compare("0.0.0.0")) {
            *this = SubnetMask(getClosestNetworkBits(*this));
        }
    }

    SubnetMask(uint32_t CIDRMask) : IP(~static_cast<uint32_t>(((1ULL<<(32-CIDRMask)) - 1))) {
        this -> hostBits = 32 - CIDRMask;
        if (!invalidFormat) {
            this -> blockSize = 1ULL<<this -> hostBits;
            this -> networkBits = CIDRMask;
            this -> numberOfSubnets = 1ULL<<networkBits;
        } else {
            this -> blockSize = 0;
            this -> networkBits = 0;
            this -> numberOfSubnets = 1;
        }
    }

    static uint32_t CIDRToIPNumber(int CIDRNumber) {
        if (CIDRNumber == 0) {
            return 0;
        }
        return ~static_cast<uint32_t>((1 << (32 - CIDRNumber)) - 1);
    }

    static uint32_t fetchHostBits(uint32_t IPNumber) {
        if (IPNumber == 0) {
            return 32;
        }
        uint32_t IPComplement = static_cast<uint32_t>((~IPNumber) + 1);
        for (uint32_t i=0; i<=32; i++) {
            if (static_cast<uint32_t>(1<<i) == IPComplement) {
                return i;
            }
        }
        return 0;
    }

    static bool verifyMask(SubnetMask netArg) {
        if (SubnetMask(netArg.networkBits).IPString.compare(netArg.IPString)) {
            return false;
        }
        return true;
    }

    static uint32_t getClosestNetworkBits(SubnetMask netArg) {
        uint32_t currentIP32 = netArg.IPAddress.IP32;
        uint32_t IP32Exp = static_cast<uint32_t>(1<<31);
        uint32_t bitshiftOperand = 30;
        while (IP32Exp < currentIP32) {
            IP32Exp += 1 << bitshiftOperand;
            bitshiftOperand--;
        }
        return 32U - fetchHostBits(IP32Exp);
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: ChangingIP Class
// -------------------------------------------------------------------------------------------------------------------------------------------

class ChangingIP {
public:
    string IPString;
    string IPBinaryString;
    SubnetMask netMask;
    friend ostream &operator<<(ostream &stream, IP IPArg);

    ChangingIP() {}
    
    ChangingIP(IP IPAddress, SubnetMask netMask) {
        this -> IPString = getChangingIPString(IPAddress, netMask);
        this -> IPBinaryString = getChangingIPBinaryString(IPAddress, netMask);
        this -> netMask = netMask;
    }

    friend ostream &operator<<(ostream &stream, ChangingIP ChangingIPArg) {
        if (binaryFlag) {
            return stream << ChangingIPArg.IPBinaryString;
        } else {
            return stream << ChangingIPArg.IPString;
        }
    }

    string getChangingIPString(IP IPArg, SubnetMask netMaskArg) {
        string returnString = "";
        int changingOctets = getChangingOctets(netMaskArg);
        for (int i=3; i>=changingOctets; i--) {
            returnString.append(to_string(IPArg.IPAddress.octets[i]));
            returnString.append(".");
        }
        for (int i=0; i<changingOctets; i++) {
            returnString.append("x.");
        }
        returnString.erase(returnString.length() - 1);
        return returnString;
    }

    static string getChangingIPBinaryString(IP IPArg, SubnetMask netMaskArg) {
        string returnString = "";
        uint32_t changingBits = netMaskArg.hostBits;
        for (int i=3; i>=0; i--) {
            returnString += IP::toBinaryString(IPArg.IPAddress.octets[i]);
        }
        for (uint32_t i=0; i<static_cast<uint32_t>(returnString.length()); i++) {
            if (i>=32U-changingBits && returnString[static_cast<size_t>(i)] != '.') {
                returnString[static_cast<size_t>(i)] = 'x';
            }
        }
        for (int i=1; i<=3; i++) {
            returnString.insert(returnString.begin() + 8*i+(i-1), '.'); // New indices each iteration must account for periods being added to string.
        }
        return returnString;
    }

    int getChangingOctets(SubnetMask netMaskArg) {
        return static_cast<int>(ceil(netMaskArg.hostBits / 8.0)); // Division requires float argument to return float.
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: Subnet Class
// -------------------------------------------------------------------------------------------------------------------------------------------

class Subnet {
public:
    IP networkIP;
    IP startIP;
    IP endIP;
    IP broadcastIP;
    SubnetMask netMask;
    friend ostream &operator<<(ostream &stream, Subnet subnetArg);

    Subnet() {}

    Subnet(IP IPAddress, SubnetMask netMask) {
        this -> networkIP = IPAddress & netMask;
        this -> startIP = networkIP + 1;
        this -> broadcastIP = networkIP + static_cast<uint32_t>(netMask.blockSize - 1);
        this -> endIP = broadcastIP - 1;
        this -> netMask = netMask;
    }

    static string subnetToString(Subnet subnetArg) {
        switch (subnetArg.netMask.blockSize) {
            case 1: return subnetArg.networkIP.IPString + "/" + to_string(subnetArg.netMask.networkBits);
            case 2: return subnetArg.networkIP.IPString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" + subnetArg.networkIP.IPString + " - " + subnetArg.broadcastIP.IPString;
            default: return subnetArg.networkIP.IPString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" + subnetArg.startIP.IPString + " - " 
                    + subnetArg.endIP.IPString + "\n\t" + subnetArg.broadcastIP.IPString + " broadcast";
        }
    }

    static string subnetToBinaryString(Subnet subnetArg) {
        switch (subnetArg.netMask.blockSize) {
            case 1: return subnetArg.networkIP.IPBinaryString + "/" + to_string(subnetArg.netMask.networkBits);
            case 2: return subnetArg.networkIP.IPBinaryString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" 
                + subnetArg.networkIP.IPBinaryString + " - " + subnetArg.broadcastIP.IPBinaryString;
            default: return subnetArg.networkIP.IPBinaryString + "/" + to_string(subnetArg.netMask.networkBits) + "\n\t" 
                + subnetArg.startIP.IPBinaryString + " - " + subnetArg.endIP.IPBinaryString + "\n\t" + subnetArg.broadcastIP.IPBinaryString 
                + " broadcast";
        }
    }

    friend ostream &operator<<(ostream &stream, Subnet subnetArg) {
        if (binaryFlag) {
            return stream << subnetToBinaryString(subnetArg);
        } else {
            return stream << subnetToString(subnetArg);
        }
    }
};

// -------------------------------------------------------------------------------------------------------------------------------------------
// SECTION: Functions for Client Code
// -------------------------------------------------------------------------------------------------------------------------------------------

void VLSM(IP IPAddr, SubnetMask netMask1, SubnetMask netMask2, bool exportFlag) {
    void (*VLSMOutput)(const char *string);
    if (exportFlag) {
        VLSMOutput = &exportOutput;
    } else {
        VLSMOutput = &nonExportOutput;
    }
    if (IPAddr.invalidFormat || netMask1.invalidFormat || netMask2.invalidFormat) {
        usage("Please provide a valid format for the IP.");
        subnettingSuccessful = false;
        return;
    } else {
        subnettingSuccessful = true;
    }
    if (netMask1.blockSize < netMask2.blockSize) {
        SubnetMask swapMask = netMask1;
        netMask1 = netMask2;
        netMask2 = swapMask;
    }
    uint32_t networkMagnitudeDifference = netMask1.hostBits - netMask2.hostBits;
    unsigned long long int totalSubnetsToGenerate = 1ULL<<networkMagnitudeDifference;
    unsigned long long int subnetsToGenerate;
    #ifdef IMGUI_API // Limit generated subnets in graphical interface to 256, but not in CLI mode.
    if (exportFlag) {
        subnetsToGenerate = totalSubnetsToGenerate;
    } else {
        subnetsToGenerate = (totalSubnetsToGenerate < 256) ? totalSubnetsToGenerate : 256;
    }
    #else
        subnetsToGenerate = totalSubnetsToGenerate;
    #endif
    IP localIPCopy = (IPAddr & netMask1) + (static_cast<uint32_t>(totalAddedToIP - 256) * static_cast<uint32_t>(netMask2.blockSize));
    IP veryFirstIP = IPAddr & netMask1;
    IP veryLastIP = (IPAddr & netMask1) + static_cast<uint32_t>((totalSubnetsToGenerate * netMask2.blockSize) - 1);
    string netMask1StringToUse;
    string netMask2StringToUse;
    string startingIPToUse;
    string changingIPToUse;
    string (*subnetStringConversionFunction)(Subnet);
    if (binaryFlag) {
        subnetStringConversionFunction = &Subnet::subnetToBinaryString;
        changingIPToUse = ChangingIP(localIPCopy, netMask1).IPBinaryString;
        startingIPToUse = veryFirstIP.IPBinaryString;
        netMask1StringToUse = netMask1.IPBinaryString;
        netMask2StringToUse = netMask2.IPBinaryString;
    } else {
        subnetStringConversionFunction = &Subnet::subnetToString;
        changingIPToUse = ChangingIP(localIPCopy, netMask1).IPString;
        startingIPToUse = veryFirstIP.IPString;
        netMask1StringToUse = netMask1.IPString;
        netMask2StringToUse = netMask2.IPString;
    }
    if (debugFlag) {VLSMOutput(("Added total in header: " + to_string(totalAddedToIP)).c_str());}
    if (exportFlag && debugFlag) {
        VLSMOutput("\n");
    }
    VLSMOutput((to_string(totalSubnetsToGenerate) + " Subnet(s) Total, " + to_string(netMask2.blockSize) + " IP(s) Per Subnet").c_str());
    if (exportFlag) {
        VLSMOutput("\n");
    }
    if (netMask1.blockSize != netMask2.blockSize) {
        VLSMOutput((netMask1StringToUse + "[/" + to_string(netMask1.networkBits) + "] -> " + netMask2StringToUse + "[/" + to_string(netMask2.networkBits) + "]\n" +
            startingIPToUse + "/" + to_string(netMask1.networkBits) + " -> " + changingIPToUse + "/" + to_string(netMask2.networkBits)).c_str());
    } else {
        VLSMOutput((netMask1StringToUse + "[/" + to_string(netMask1.networkBits) + "]").c_str());
    }
    if (exportFlag) {
        VLSMOutput("\n");
    }
    VLSMOutput("-------------------------------------------------------------------");
    if (exportFlag) {
        VLSMOutput("\n");
    }
    if (reverseFlag) {
        localIPCopy += static_cast<uint32_t>(netMask2.blockSize) * static_cast<uint32_t>(subnetsToGenerate - 1);
        for (uint32_t i=0; i<subnetsToGenerate; i++) {
            #ifdef IMGUI_API
            if (!windowsAreOpen[0]) return;
            #endif
            VLSMOutput((subnetStringConversionFunction(Subnet(localIPCopy, netMask2))).c_str());
            if (exportFlag) {
                VLSMOutput("\n");
            }
            localIPCopy -= static_cast<uint32_t>(netMask2.blockSize);
        }
    } else {
        for (uint32_t i=0; i<subnetsToGenerate; i++) {
            #ifdef IMGUI_API
            if (!windowsAreOpen[0]) return;
            #endif
            VLSMOutput((subnetStringConversionFunction(Subnet(localIPCopy, netMask2))).c_str());
            if (exportFlag) {
                VLSMOutput("\n");
            }
            localIPCopy += static_cast<uint32_t>(netMask2.blockSize);
        }
    }
}

void timedVLSM(IP IPAddr, SubnetMask netMask1, SubnetMask netMask2, bool exportFlag) {
    clock_t startingClock, endingClock;
    startingClock = clock();
    VLSM(IPAddr, netMask1, netMask2, exportFlag);
    endingClock = clock();
    double timeTotal = static_cast<double>(endingClock - startingClock) / CLOCKS_PER_SEC;
    if (debugFlag && exportFlag) {
        exportOutput((to_string(timeTotal) + " seconds to run").c_str());
    } else if (debugFlag) {
        nonExportOutput((to_string(timeTotal) + " seconds to run").c_str());
    }
}