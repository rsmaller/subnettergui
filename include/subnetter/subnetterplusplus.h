#pragma once
#include <iostream>
#include <string.h>
#include <cmath>
#include <regex>
#include <time.h>
#include <fstream>

using namespace std;
unsigned long long totalAddedToIP = 256;

typedef union IPNumeric {
    unsigned int IP32;
    unsigned char octets[4];
} IPNumeric;

char **globalArgumentVector;
string programName;
bool binaryFlag = false;
bool debugFlag = false;
bool reverseFlag = false;
extern ofstream exportFileStream;
extern const int totalNumberOfWindows;
extern bool windowsAreOpen[];

#ifdef IMGUI_API // If ImGui included, use ImGui::Text to render output. Otherwise, send output to cout
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


class IP {
public:
    IPNumeric IPAddress;
    string IPString;
    string IPBinaryString;
    bool invalidFormat = false;
    friend ostream &operator<<(ostream &stream, IP IPArg);

    IP(string stringArg) {
        if (isIPFormat(stringArg)) {
            this -> IPAddress = StringToIPInt(stringArg);
            this -> IPString = stringArg;
        } else if (isIPNumber(stringArg)) {
            this -> IPAddress.IP32 = (unsigned int)stoi(stringArg);
            this -> IPString = intToIPString(this -> IPAddress);
        } else {
            this -> IPString = "0.0.0.0";
            this -> IPAddress.IP32 = 0;
            this -> invalidFormat = true;
        }
        this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
    }

    IP(unsigned int IPArg) {
        this -> IPAddress.IP32 = IPArg;
        this -> IPString = intToIPString(this -> IPAddress);
        this -> IPBinaryString = toIPBinaryString(this -> IPAddress);
    }

    IP() {} // necessary for prototypes to work.

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
        return regex_search(testString, matches, IPNumberPattern) && (testString == to_string((unsigned int)stoi(testString)));
    }

    static IPNumeric StringToIPInt(string stringArg) {
        IPNumeric returnValue;
        IPNumeric CIDRConversion;
        string operatingString;
        string currentOctet;
        if (isIPFormat(stringArg)) {
            operatingString = stringArg;
        } else if (isCIDRMask(stringArg)) {
            CIDRConversion.IP32 = ~((1<<(32-stoi(stringArg)))-1);
            operatingString = intToIPString(CIDRConversion);
        } else {
            returnValue = {0};
            return returnValue;
        }
        for (int i=3; i>=0; i--) {
            currentOctet = operatingString.substr(0, operatingString.find('.'));
            operatingString.erase(0, operatingString.find('.') + 1);
            returnValue.octets[i] = (unsigned char)stoi(currentOctet);
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
                numArg = (T)((int)numArg - (1<<i));
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
            returnString += toBinaryString<unsigned char>(IPArg.octets[i]);
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

    IP operator+(unsigned int operand) {
        return IP(this->IPAddress.IP32 + operand);
    }

    IP operator-(unsigned int operand) {
        return IP(this->IPAddress.IP32 - operand);
    }

    IP operator&(IP operand) {
        return IP(this->IPAddress.IP32 & operand.IPAddress.IP32);
    }

    void operator+=(unsigned int operand) {
        *this = (*this + operand);
    }

    void operator-=(unsigned int operand) {
        *this = (*this - operand);
    }

    void operator&=(unsigned int operand) {
        *this = (*this & operand);
    }

};

class SubnetMask : public IP {
public:
    unsigned long long blockSize;
    unsigned long long numberOfSubnets;
    unsigned int hostBits;
    unsigned int networkBits;

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
        this -> hostBits = fetchHostBits((int)this -> IPAddress.IP32);
        if (this -> hostBits == 32) {
            this -> blockSize = 4294967296ULL;
        } else if (!invalidFormat) {
            this -> blockSize = 1ULL<<(unsigned long long)(this -> hostBits);
        } else {
            this -> blockSize = 0;
        }
        this -> networkBits = 32 - hostBits;
        this -> numberOfSubnets = 1ULL<<networkBits;
        if (!verifyMask(*this) && this->IPString.compare("0.0.0.0")) {
            *this = SubnetMask(getClosestNetworkBits(*this));
        }
    }

    SubnetMask(int CIDRMask) : IP(~((1ULL<<(32-CIDRMask)) - 1)) {
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

    SubnetMask() {}

    static int CIDRToIPNumber(int CIDRNumber) {
        if (CIDRNumber == 0) {
            return 0;
        }
        return ~((1 << (32 - CIDRNumber)) - 1);
    }

    static int fetchHostBits(int IPNumber) {
        if (IPNumber == 0) {
            return 32;
        }
        int IPComplement = (int)(~IPNumber) + 1;
        for (int i=0; i<=32; i++) {
            if (1<<i == IPComplement) {
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

    static int getClosestNetworkBits(SubnetMask netArg) {
        int currentIP32 = netArg.IPAddress.IP32;
        int IP32Exp = 1<<31;
        int bitshiftOperand = 30;
        while (IP32Exp < currentIP32) {
            IP32Exp += 1 << bitshiftOperand;
            bitshiftOperand--;
        }
        return 32 - fetchHostBits(IP32Exp);
    }
};

class ChangingIP {
public:
    string IPString;
    string IPBinaryString;
    SubnetMask netMask;
    friend ostream &operator<<(ostream &stream, IP IPArg);

    ChangingIP(IP IPAddress, SubnetMask netMask) {
        this -> IPString = getChangingIPString(IPAddress, netMask);
        this -> IPBinaryString = getChangingIPBinaryString(IPAddress, netMask);
        this -> netMask = netMask;
    }

    ChangingIP() {}

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
        int changingBits = netMaskArg.hostBits;
        for (int i=3; i>=0; i--) {
            returnString += IP::toBinaryString(IPArg.IPAddress.octets[i]);
        }
        for (int i=0; i<(int)returnString.length(); i++) {
            if (i>=32-changingBits && returnString[(size_t)i] != '.') {
                returnString[(size_t)i] = 'x';
            }
        }
        for (int i=1; i<=3; i++) {
            returnString.insert(returnString.begin() + 8*i+(i-1), '.'); // new indices each iteration must account for periods being added to string.
        }
        return returnString;
    }

    int getChangingOctets(SubnetMask netMaskArg) {
        return (int)ceil(netMaskArg.hostBits / 8.0); // division requires float argument to return float.
    }
};

class Subnet {
public:

    IP networkIP;
    IP startIP;
    IP endIP;
    IP broadcastIP;
    SubnetMask netMask;
    friend ostream &operator<<(ostream &stream, Subnet subnetArg);

    Subnet(IP IPAddress, SubnetMask netMask) {
        this -> networkIP = IPAddress & netMask;
        this -> startIP = networkIP + 1;
        this -> broadcastIP = networkIP + (unsigned int)(netMask.blockSize - 1);
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

    friend ostream &operator<<(ostream &stream, Subnet subnetArg) { // change what this does when using binary argument?
        if (binaryFlag) {
            return stream << subnetToBinaryString(subnetArg);
        } else {
            return stream << subnetToString(subnetArg);
        }
    }
};

void exportOutput(const char *string) {
    exportFileStream << string;
}

void VLSM(IP IPAddr, SubnetMask netMask1, SubnetMask netMask2, bool exportFlag) {
    void (*VLSMOutput)(const char *string);
    if (exportFlag) {
        VLSMOutput = &exportOutput;
    } else {
        VLSMOutput = &nonExportOutput;
    }
    if (IPAddr.invalidFormat || netMask1.invalidFormat || netMask2.invalidFormat) {
        usage("Please provide a valid format for the IP.");
        return;
    }
    if (netMask1.blockSize < netMask2.blockSize) {
        SubnetMask swapMask = netMask1;
        netMask1 = netMask2;
        netMask2 = swapMask;
    }
    int networkMagnitudeDifference = netMask1.hostBits - netMask2.hostBits;
    unsigned long long int totalSubnetsToGenerate = 1ULL<<networkMagnitudeDifference;
    unsigned long long int subnetsToGenerate;
    #ifdef IMGUI_API // Limit generated subnets in graphical interface to 256 but not in CLI
    if(exportFlag) {
        subnetsToGenerate = totalSubnetsToGenerate;
    } else {
        subnetsToGenerate = (totalSubnetsToGenerate < 256) ? totalSubnetsToGenerate : 256;
    }
    #else
        subnetsToGenerate = totalSubnetsToGenerate;
    #endif
    IP localIPCopy = (IPAddr & netMask1) + ((unsigned int)(totalAddedToIP - 256) * (unsigned int)netMask2.blockSize);
    IP veryFirstIP = IPAddr & netMask1;
    IP veryLastIP = (IPAddr & netMask1) + (unsigned int)(((unsigned long long)totalSubnetsToGenerate * netMask2.blockSize) - 1);
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
    VLSMOutput((to_string(totalSubnetsToGenerate) + " Subnet(s) Total, " + to_string(netMask2.blockSize) + " IP(s) Per Subnet\n").c_str());
    if (netMask1.blockSize != netMask2.blockSize) {
        VLSMOutput((netMask1StringToUse + "[/" + to_string(netMask1.networkBits) + "] -> " + netMask2StringToUse + "[/" + to_string(netMask2.networkBits) + "]\n" +
            startingIPToUse + "/" + to_string(netMask1.networkBits) + " -> " + changingIPToUse + "/" + to_string(netMask2.networkBits)).c_str());
    } else {
        VLSMOutput((netMask1StringToUse + "[/" + to_string(netMask1.networkBits) + "]").c_str());
    }
    if (exportFlag) {
        VLSMOutput("\n");
    }
    VLSMOutput("-------------------------------------------------------------------\n");
    if (reverseFlag) {
        localIPCopy += (unsigned int)netMask2.blockSize * ((unsigned int)subnetsToGenerate - 1);
        for (int i=0; i<subnetsToGenerate; i++) {
            if (!windowsAreOpen[0]) return;
            VLSMOutput((subnetStringConversionFunction(Subnet(localIPCopy, netMask2)) + "\n").c_str());
            localIPCopy -= (unsigned int)netMask2.blockSize;
        }
    } else {
        for (int i=0; i<subnetsToGenerate; i++) {
            if (!windowsAreOpen[0]) return;
            VLSMOutput((subnetStringConversionFunction(Subnet(localIPCopy, netMask2)) + "\n").c_str());
            localIPCopy += (unsigned int)netMask2.blockSize;
        }
    }
}

void timedVLSM(IP IPAddr, SubnetMask netMask1, SubnetMask netMask2, bool exportFlag) {
    clock_t startingClock, endingClock;
    startingClock = clock();
    VLSM(IPAddr, netMask1, netMask2, exportFlag);
    endingClock = clock();
    double timeTotal = (double)(endingClock - startingClock) / CLOCKS_PER_SEC;
    if (debugFlag && exportFlag) {
        exportOutput((to_string(timeTotal) + " seconds to run").c_str());
        exportFileStream.close();
    } else if (debugFlag) {
        nonExportOutput((to_string(timeTotal) + " seconds to run").c_str());
    }
}