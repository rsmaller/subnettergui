#pragma once
#include <string>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <regex>
#include <cstdlib>
#include <sstream>
#define IPv6UTILS
using namespace std;

class IPv6 {
public:
    unsigned short hextets[8];
    string IPv6String;
    string shortenedIPv6String;
    
    IPv6() {}

    IPv6(unsigned short hextets[8]) { 
        stringstream IPv6StringContainer;
        memcpy(this -> hextets, hextets, 16);
        for (int i=0; i<8; i++) {
            IPv6StringContainer << hex << hextets[i];
            if (i<7) IPv6StringContainer << ":";
        }
        this -> IPv6String = IPv6Sanitize(IPv6StringContainer.str());
        shortenedIPv6String = IPv6Truncate(this -> IPv6String);
    }

    IPv6(string stringArg) {
        unsigned short *hextetsToCopy = IPv6StringToHextets(stringArg);
        memcpy(this -> hextets, hextetsToCopy, 16);
        this -> IPv6String = IPv6Sanitize(stringArg);
        shortenedIPv6String = IPv6Truncate(this -> IPv6String);
        free(hextetsToCopy);
    }

    static string IPv6Sanitize(string stringArg) {
        int loopMaximum = 100;
        int currentLoopIteration = 0;

        if (!stringArg.compare("")) {
            return "0000:0000:0000:0000:0000:0000:0000:0000";
        }

        string stringToManipulate = stringArg;
        string zeroBlockString = "";

        smatch zeroHextetMatch;
        regex IPv6ZeroHextetRegex("\\:0{1,3}\\:");

        smatch leadingZeroMatch;
        regex IPv6LeadingZeroRegex("\\:[0-9a-fA-F]{1,3}\\:");

        smatch beginningOfStringLeadingZeroMatch;
        regex beginningOfStringLeadingZeroRegex("^[0-9a-fA-F]{1,3}\\:");

        smatch endOfStringLeadingZeroMatch;
        regex endOfStringLeadingZeroRegex("\\:[0-9a-fA-F]{1,3}$");

        smatch doubleColonMatch;
        regex IPv6DoubleColonRegex("\\:\\:");

        smatch IPv6FormatMatch;
        regex IPv6FormatRegex("^[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}$");

        while (regex_search(stringToManipulate, beginningOfStringLeadingZeroMatch, beginningOfStringLeadingZeroRegex)) {
            stringToManipulate = "0" + stringToManipulate;
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        while (regex_search(stringToManipulate, zeroHextetMatch, IPv6ZeroHextetRegex)) {
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(zeroHextetMatch.position())) + ":0000:" + stringToManipulate.substr(static_cast<size_t>(zeroHextetMatch.position()) + static_cast<size_t>(zeroHextetMatch.length()), static_cast<size_t>(stringToManipulate.length()) - 1U);
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        while (regex_search(stringToManipulate, leadingZeroMatch, IPv6LeadingZeroRegex)) {
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(leadingZeroMatch.position())) + ":" + string(6U - static_cast<size_t>(leadingZeroMatch.length()), '0') + stringToManipulate.substr(static_cast<size_t>(leadingZeroMatch.position()) + 1U, static_cast<size_t>(stringToManipulate.length()) - 1U);
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        while (regex_search(stringToManipulate, endOfStringLeadingZeroMatch, endOfStringLeadingZeroRegex)) {
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(endOfStringLeadingZeroMatch.position()) + 1U) + "0" + stringToManipulate.substr(static_cast<size_t>(endOfStringLeadingZeroMatch.position()) + 1U, static_cast<size_t>(stringToManipulate.length())  - 1U);
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;

        if (regex_search(stringToManipulate, doubleColonMatch, IPv6DoubleColonRegex)) {
            int lengthDifference = 39 - static_cast<int>(stringToManipulate.length());
            int numberOfColonsToAdd = (lengthDifference / 5);
            int numberOfZeroesToAdd = lengthDifference - numberOfColonsToAdd;
            while (numberOfZeroesToAdd) {
                for (int i=0; i<4; i++) {
                    zeroBlockString += "0";
                }
                if (numberOfColonsToAdd) {
                    zeroBlockString += ":";
                    numberOfColonsToAdd--;
                }
                numberOfZeroesToAdd -= 4;
                if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
                currentLoopIteration++; 
            }
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(doubleColonMatch.position()) + 1) + zeroBlockString + stringToManipulate.substr(static_cast<size_t>(doubleColonMatch.position()) + 1U, static_cast<size_t>(stringToManipulate.length()) - 1U);
        }

        if (!regex_search(stringToManipulate, IPv6FormatMatch, IPv6FormatRegex)) {
            return "0000:0000:0000:0000:0000:0000:0000:0000";
        }
        return stringToManipulate;
    }

    static string IPv6Truncate(string stringArg) {
        int loopMaximum = 100;
        int currentLoopIteration = 0;

        if (!stringArg.compare("0000:0000:0000:0000:0000:0000:0000:0000")) {
            return "::";
        }

        string sanitizedString = IPv6Sanitize(stringArg);
        string truncatedString = sanitizedString;
        string greediestMatch = "";

        smatch greedyZeroMatch;
        regex greedyZeroRegex("(\\:[0:]+$)|(^[0:]+\\:)|(\\:[0:]+\\:)"); // THIS is the correct way to match for double colons.

        smatch leadingZeroMatch;
        regex leadingZeroRegex("\\:[0]{1,3}(?!:)(?!$)"); // zero preceded by a colon but not followed by a colon

        smatch cutOffZeroMatch;
        regex cutOffZeroRegex("[0-9a-fA-F]\\:$");

        smatch extraZeroAfterDoubleColonMatch;
        regex extraZeroAfterDoubleColonRegex("\\:{2}0$");

        smatch leadingZeroesAtBeginningMatch;
        regex leadingZeroesAtBeginningRegex("^0{1,3}");

        smatch hangingZeroAtBeginningMatch;
        regex hangingZeroAtBeginningRegex("^\\:");

        smatch doubleColonMatch;
        regex doubleColonRegex("\\:\\:");

        size_t index;

        while (regex_search(truncatedString, leadingZeroMatch, leadingZeroRegex)) {
            truncatedString = truncatedString.substr(0U, static_cast<size_t>(leadingZeroMatch.position()) + 1U) + truncatedString.substr(static_cast<size_t>(leadingZeroMatch.position()) + static_cast<size_t>(leadingZeroMatch.length()), static_cast<size_t>(truncatedString.length()) - 1U);
            if (currentLoopIteration > loopMaximum) break;
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        sanitizedString = truncatedString;

        if (regex_search(sanitizedString, leadingZeroesAtBeginningMatch, leadingZeroesAtBeginningRegex)) {
            sanitizedString = sanitizedString.substr(0U, static_cast<size_t>(leadingZeroesAtBeginningMatch.position())) + sanitizedString.substr(static_cast<size_t>(leadingZeroesAtBeginningMatch.position()) + static_cast<size_t>(leadingZeroesAtBeginningMatch.length()), static_cast<size_t>(sanitizedString.length()) - 1U);
        }

        while (regex_search(sanitizedString, greedyZeroMatch, greedyZeroRegex)) { // truncate the longest block of zeroes into a double colon
            if (static_cast<int>(greedyZeroMatch[0].length()) > static_cast<int>(greediestMatch.length())) {
                greediestMatch = greedyZeroMatch[0];
            }
            sanitizedString = sanitizedString.substr(0U, static_cast<size_t>(greedyZeroMatch.position())) + ":" + sanitizedString.substr(static_cast<size_t>(greedyZeroMatch.position()) + static_cast<size_t>(greedyZeroMatch.length()), static_cast<size_t>(sanitizedString.length()) - 1U);
            if (currentLoopIteration > loopMaximum) break;
            currentLoopIteration++;
        }
        currentLoopIteration = 0;
        currentLoopIteration = 0;
        if (greediestMatch.compare("")) {
            index = truncatedString.find(greediestMatch);
            truncatedString = truncatedString.substr(0U, index) + "::" + truncatedString.substr(index + static_cast<size_t>(greediestMatch.length()), static_cast<size_t>(truncatedString.length()) - 1U);
        } // end of zero block truncation
        while (regex_search(truncatedString, leadingZeroesAtBeginningMatch, leadingZeroesAtBeginningRegex)) {
            truncatedString = truncatedString.substr(1, truncatedString.length());
            if (currentLoopIteration > loopMaximum) break;
            currentLoopIteration++;
        }
        if (regex_search(truncatedString, cutOffZeroMatch, cutOffZeroRegex)) { // account for very end's leading zero possibly being cut off due to truncation
            truncatedString = truncatedString + "0";
        }
        if (regex_search(truncatedString, extraZeroAfterDoubleColonMatch, extraZeroAfterDoubleColonRegex)) {
            truncatedString = truncatedString.substr(0, truncatedString.length() - 1);
        }
        if (regex_search(truncatedString, hangingZeroAtBeginningMatch, hangingZeroAtBeginningRegex) && !regex_search(truncatedString, doubleColonMatch, doubleColonRegex)) {
            // truncatedString = ":" + truncatedString;
        } else if (regex_search(truncatedString, hangingZeroAtBeginningMatch, hangingZeroAtBeginningRegex) && truncatedString[0] != ':' && truncatedString[1] != ':') {
            truncatedString = "0" + truncatedString;
        }
        if (truncatedString[0] == ':' && truncatedString[1] != ':') {
            truncatedString = "0" + truncatedString;
        }
        return truncatedString;
    }

    static unsigned short *IPv6StringToHextets(string stringArg) {
        stringArg = IPv6Sanitize(stringArg);
        size_t currentIndex = 0;
        string currentString = "";
        unsigned short *hextets = static_cast<unsigned short *>(calloc(8, sizeof(short)));
        for (int i=0; i<8; i++) {
            currentString = "";
            while (stringArg[currentIndex] != ':' && static_cast<size_t>(currentIndex) < stringArg.length()) {
                currentString += stringArg[currentIndex];
                currentIndex++;
            }
            currentIndex++;
            hextets[i] = static_cast<unsigned short>(stoi("0x" + currentString, nullptr, 16));
        }
        return hextets;
    }

    static unsigned short *MACStringToHextets(string stringArg) {
        unsigned short *hextets = static_cast<unsigned short *>(calloc(3, sizeof(short)));
        smatch MACMatch;
        regex MACRegex("^[0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}$");
        if (!regex_search(stringArg, MACMatch, MACRegex)) {
            memset(hextets, 0, 3*sizeof(short));
            return hextets;
        }
        string calculationString = "";
        for (unsigned int i=0; i<stringArg.length(); i++) {
            if (i != 2 && i != 8 && i != 14) {
                calculationString += stringArg[i];
            }
        }
        size_t currentIndex = 0;
        string currentString = "";
        for (unsigned int i=0; i<3; i++) {
            currentString = "";
            while (calculationString[currentIndex] != ':' && calculationString[currentIndex] != '-' && static_cast<size_t>(currentIndex) < calculationString.length()) {
                currentString += calculationString[currentIndex];
                currentIndex++;
            }
            currentIndex++;
            hextets[i] = static_cast<unsigned short>(stoi("0x" + currentString, nullptr, 16));
        }
        return hextets;
    }

    static unsigned short *MACHextetsToEUIHextets(unsigned short *MACHextets) {
        unsigned short *EUIHextets = static_cast<unsigned short *>(calloc(4, sizeof(short)));
        EUIHextets[0U] = static_cast<unsigned short>(MACHextets[0U] ^ static_cast<unsigned short>(0b00000010));
        EUIHextets[1] = static_cast<unsigned short>((MACHextets[1] & static_cast<unsigned short>(0xff00)) + static_cast<unsigned short>(0x00ff));
        EUIHextets[2] = static_cast<unsigned short>((MACHextets[1] & static_cast<unsigned short>(0x00ff)) + static_cast<unsigned short>(0xfe00));
        EUIHextets[3] = MACHextets[2];
        return EUIHextets;
    }

    static string MACStringToEUIString(string MACString) {
        smatch MACMatch;
        regex MACRegex("^[0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}$");
        if (!regex_search(MACString, MACMatch, MACRegex)) {
            return "0000:00ff:fe00:0000";
        }
        stringstream EUIString;
        unsigned short *MACHextets = MACStringToHextets(MACString);
        unsigned short *EUIHextets = MACHextetsToEUIHextets(MACHextets);
        free(MACHextets);
        for (int i=0; i<4; i++) {
            EUIString << hex << setfill('0') << setw(4) << EUIHextets[i];
            EUIString << ":";
        }
        free(EUIHextets);
        return EUIString.str().substr(0, EUIString.str().length() - 1);
    }

    static string MACHextetsToString(unsigned short *hextets) {
        stringstream MACString;
        unsigned char *macOctets = reinterpret_cast<unsigned char *>(hextets);
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[1]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[0]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[3]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[2]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[5]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[4]);
        return MACString.str();
    }

    string type() {

        if (
            ((this -> hextets[0] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[1] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[2] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[3] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[4] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[5] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[6] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[7] & static_cast<unsigned short>(0xfffe)) == static_cast<unsigned short>(0))
        ) {
            return "loopback";
        }

        if (
            ((this -> hextets[0] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0xff02)) &&
            ((this -> hextets[1] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[2] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[3] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[4] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[5] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0x0001)) &&
            ((this -> hextets[6] & static_cast<unsigned short>(0xff00)) == static_cast<unsigned short>(0xff00))
        ) {
            return "solicited node multicast";
        }

        if (
            ((this -> hextets[0] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[1] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[2] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[3] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[4] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0)) &&
            ((this -> hextets[5] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0))
        ) {
            return "IPv4 backwards compatible";
        }


        if (
            (this -> hextets[0] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0x2001) ||
            (this -> hextets[0] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0x2002) ||
            (this -> hextets[0] & static_cast<unsigned short>(0xffff)) == static_cast<unsigned short>(0x3ffe)
        ) {
            return "aggregatable global unicast or anycast";
        }

        if ((this -> hextets[0] & static_cast<unsigned short>(0b1111111111000000)) == static_cast<unsigned short>(0xfe80)) {
            return "link-local unicast or anycast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0b1111111111000000)) == static_cast<unsigned short>(0xfec0)) {
            return "site-local unicast or anycast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0b1111111100000000)) == static_cast<unsigned short>(0xff00)) {
            return "assigned multicast";
        }
        
        if ((this -> hextets[0] & static_cast<unsigned short>(0b1111111000000000)) == static_cast<unsigned short>(0xfc00)) {
            return "unique local unicast";
        }
        

        if ((this -> hextets[0] & static_cast<unsigned short>(0b1110000000000000)) == static_cast<unsigned short>(0x2000)) {
            return "generic global unicast";
        }
        return "unspecified";
    }
};