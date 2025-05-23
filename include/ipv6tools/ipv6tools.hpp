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
    unsigned short hextets[8] = {0};
    string IPv6String;
    string shortenedIPv6String;
    
    IPv6() = default;

    IPv6(unsigned short hextets[8]) { // NOLINT
        stringstream IPv6StringContainer;
        memcpy(this -> hextets, hextets, 16);
        for (int i=0; i<8; i++) {
            IPv6StringContainer << hex << hextets[i];
            if (i<7) IPv6StringContainer << ":";
        }
        this -> IPv6String = IPv6Sanitize(IPv6StringContainer.str());
        shortenedIPv6String = IPv6Truncate(this -> IPv6String);
    }

    IPv6(const string& stringArg) { // NOLINT
        unsigned short *hextetsToCopy = IPv6StringToHextets(stringArg);
        memcpy(this -> hextets, hextetsToCopy, 16);
        this -> IPv6String = IPv6Sanitize(stringArg);
        shortenedIPv6String = IPv6Truncate(this -> IPv6String);
        free(hextetsToCopy);
    }

    static string IPv6Sanitize(const string& stringArg) { // NOLINT
        int loopMaximum = 100;
        int currentLoopIteration = 0;

        if (stringArg.empty()) {
            return "0000:0000:0000:0000:0000:0000:0000:0000";
        }

        string stringToManipulate = stringArg;

        smatch zeroHextetMatch;
        regex IPv6ZeroHextetRegex("\\:0{1,3}\\:");

        smatch leadingZeroMatch;
        regex IPv6LeadingZeroRegex("\\:[0-9a-fA-F]{1,3}\\:");

        smatch beginningOfStringLeadingZeroMatch;
        regex beginningOfStringLeadingZeroRegex("^[0-9a-fA-F]{1,3}\\:");

        smatch endOfStringLeadingZeroMatch;
        regex endOfStringLeadingZeroRegex("\\:[0-9a-fA-F]{1,3}$");

        regex IPv6DoubleColonRegex("\\:\\:");

        smatch IPv6FormatMatch;
        regex IPv6FormatRegex(R"(^[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}\:[0-9a-fA-F]{1,4}$)");

        while (regex_search(stringToManipulate, beginningOfStringLeadingZeroMatch, beginningOfStringLeadingZeroRegex)) {
            stringToManipulate = "0" + stringToManipulate; // NOLINT
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        while (regex_search(stringToManipulate, zeroHextetMatch, IPv6ZeroHextetRegex)) {
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(zeroHextetMatch.position())) + ":0000:" + 
                                 stringToManipulate.substr(static_cast<size_t>(zeroHextetMatch.position()) + static_cast<size_t>(zeroHextetMatch.length()), 
                                 static_cast<size_t>(stringToManipulate.length()) - 1U);
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        while (regex_search(stringToManipulate, leadingZeroMatch, IPv6LeadingZeroRegex)) {
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(leadingZeroMatch.position())) + ":" + string(6U - static_cast<size_t>(leadingZeroMatch.length()), '0') + 
                                 stringToManipulate.substr(static_cast<size_t>(leadingZeroMatch.position()) + 1U, static_cast<size_t>(stringToManipulate.length()) - 1U);
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        while (regex_search(stringToManipulate, endOfStringLeadingZeroMatch, endOfStringLeadingZeroRegex)) {
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(endOfStringLeadingZeroMatch.position()) + 1U) + "0" + 
                                 stringToManipulate.substr(static_cast<size_t>(endOfStringLeadingZeroMatch.position()) + 1U, static_cast<size_t>(stringToManipulate.length())  - 1U);
            if (currentLoopIteration > loopMaximum) return "0000:0000:0000:0000:0000:0000:0000:0000";
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;

        if (smatch doubleColonMatch; regex_search(stringToManipulate, doubleColonMatch, IPv6DoubleColonRegex)) {
            string zeroBlockString;
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
            stringToManipulate = stringToManipulate.substr(0U, static_cast<size_t>(doubleColonMatch.position()) + 1) + zeroBlockString + 
                                 stringToManipulate.substr(static_cast<size_t>(doubleColonMatch.position()) + 1U, static_cast<size_t>(stringToManipulate.length()) - 1U);
        }

        if (!regex_search(stringToManipulate, IPv6FormatMatch, IPv6FormatRegex)) {
            return "0000:0000:0000:0000:0000:0000:0000:0000";
        }
        return stringToManipulate;
    }

    static string IPv6Truncate(const string& stringArg) {
        int loopMaximum = 100;
        int currentLoopIteration = 0;

        if (stringArg == "0000:0000:0000:0000:0000:0000:0000:0000") {
            return "::";
        }

        string sanitizedString = IPv6Sanitize(stringArg);
        string truncatedString = sanitizedString;
        string greediestMatch;

        smatch greedyZeroMatch;
        regex greedyZeroRegex(R"((\:[0:]+$)|(^[0:]+\:)|(\:[0:]+\:))"); // Match for double colons.

        smatch leadingZeroMatch;
        regex leadingZeroRegex("\\:[0]{1,3}(?!:)(?!$)"); // Zero preceded by a colon but not followed by a colon.

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

        while (regex_search(truncatedString, leadingZeroMatch, leadingZeroRegex)) {
            truncatedString = truncatedString.substr(0U, static_cast<size_t>(leadingZeroMatch.position()) + 1U) + truncatedString.substr(static_cast<size_t>(leadingZeroMatch.position()) + 
                              static_cast<size_t>(leadingZeroMatch.length()), static_cast<size_t>(truncatedString.length()) - 1U);
            if (currentLoopIteration > loopMaximum) break;
            currentLoopIteration++; 
        }
        currentLoopIteration = 0;
        sanitizedString = truncatedString;

        if (regex_search(sanitizedString, leadingZeroesAtBeginningMatch, leadingZeroesAtBeginningRegex)) {
            sanitizedString = sanitizedString.substr(0U, static_cast<size_t>(leadingZeroesAtBeginningMatch.position())) + 
                              sanitizedString.substr(static_cast<size_t>(leadingZeroesAtBeginningMatch.position()) + 
                              static_cast<size_t>(leadingZeroesAtBeginningMatch.length()), static_cast<size_t>(sanitizedString.length()) - 1U);
        }

        while (regex_search(sanitizedString, greedyZeroMatch, greedyZeroRegex)) { // Truncate the longest block of zeroes into a double colon.
            if (static_cast<int>(greedyZeroMatch[0].length()) > static_cast<int>(greediestMatch.length())) {
                greediestMatch = greedyZeroMatch[0];
            }
            sanitizedString = sanitizedString.substr(0U, static_cast<size_t>(greedyZeroMatch.position())) + ":" + sanitizedString.substr(static_cast<size_t>(greedyZeroMatch.position()) + 
                              static_cast<size_t>(greedyZeroMatch.length()), static_cast<size_t>(sanitizedString.length()) - 1U);
            if (currentLoopIteration > loopMaximum) break;
            currentLoopIteration++;
        }
        currentLoopIteration = 0;
        if (greediestMatch.empty()) {
            size_t index;
            // NOLINT
            index = truncatedString.find(greediestMatch);
            truncatedString = truncatedString.substr(0U, index) + "::" + truncatedString.substr(index + 
                              static_cast<size_t>(greediestMatch.length()), static_cast<size_t>(truncatedString.length()) - 1U);
        } // End of zero block truncation.
        while (regex_search(truncatedString, leadingZeroesAtBeginningMatch, leadingZeroesAtBeginningRegex)) {
            truncatedString = truncatedString.substr(1, truncatedString.length());
            if (currentLoopIteration > loopMaximum) break;
            currentLoopIteration++;
        }
        if (regex_search(truncatedString, cutOffZeroMatch, cutOffZeroRegex)) { // Account for very end's leading zero possibly being cut off due to truncation.
            truncatedString = truncatedString + "0";
        }
        if (regex_search(truncatedString, extraZeroAfterDoubleColonMatch, extraZeroAfterDoubleColonRegex)) {
            truncatedString = truncatedString.substr(0, truncatedString.length() - 1);
        }
        // if (regex_search(truncatedString, hangingZeroAtBeginningMatch, hangingZeroAtBeginningRegex) && !regex_search(truncatedString, doubleColonMatch, doubleColonRegex)) {
        //     truncatedString = ":" + truncatedString;
        // } else 
        if (regex_search(truncatedString, hangingZeroAtBeginningMatch, hangingZeroAtBeginningRegex) && truncatedString[0] != ':' && truncatedString[1] != ':') {
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
        auto *hextets = static_cast<unsigned short *>(calloc(8, sizeof(short)));
        for (int i=0; i<8; i++) {
            string currentString;
            while (stringArg[currentIndex] != ':' && static_cast<size_t>(currentIndex) < stringArg.length()) {
                currentString += stringArg[currentIndex];
                currentIndex++;
            }
            currentIndex++;
            hextets[i] = static_cast<unsigned short>(stoi("0x" + currentString, nullptr, 16));
        }
        return hextets;
    }

    static unsigned short *MACStringToHextets(const string& stringArg) {
        auto *hextets = static_cast<unsigned short *>(calloc(3, sizeof(short)));
        if (hextets == nullptr) {
            exit(1);
        }
        const regex MACRegex("^[0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}$");
        if (smatch MACMatch; !regex_search(stringArg, MACMatch, MACRegex)) {
            memset(hextets, 0, 3*sizeof(short));
            return hextets;
        }
        string calculationString;
        for (unsigned int i=0; i<stringArg.length(); i++) {
            if (i != 2 && i != 8 && i != 14) {
                calculationString += stringArg[i];
            }
        }
        size_t currentIndex = 0;
        for (unsigned int i=0; i<3; i++) {
            string currentString;
            while (calculationString[currentIndex] != ':' && calculationString[currentIndex] != '-' && static_cast<size_t>(currentIndex) < calculationString.length()) {
                currentString += calculationString[currentIndex];
                currentIndex++;
            }
            currentIndex++;
            hextets[i] = static_cast<unsigned short>(stoi("0x" + currentString, nullptr, 16));
        }
        return hextets;
    }

    static unsigned short *MACHextetsToEUIHextets(const unsigned short *MACHextets) {
        auto *EUIHextets = static_cast<unsigned short *>(calloc(4, sizeof(short)));
        EUIHextets[0U]             = static_cast<unsigned short>(MACHextets[0U] ^ static_cast<unsigned short>(0x0002));
        EUIHextets[1]              = static_cast<unsigned short>((MACHextets[1] & static_cast<unsigned short>(0xff00)) + static_cast<unsigned short>(0x00ff));
        EUIHextets[2]              = static_cast<unsigned short>((MACHextets[1] & static_cast<unsigned short>(0x00ff)) + static_cast<unsigned short>(0xfe00));
        EUIHextets[3]              = MACHextets[2];
        return EUIHextets;
    }

    static string MACStringToEUIString(const string& MACString) {
        const regex MACRegex("^[0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}[:-][0-9a-fA-F]{2}$");
        if (smatch MACMatch; !regex_search(MACString, MACMatch, MACRegex)) {
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

    static string MACHextetsToString(unsigned short *hextets) { // NOLINT
        stringstream MACString;
        const auto *macOctets = reinterpret_cast<unsigned char *>(hextets);
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[1]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[0]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[3]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[2]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[5]) << ":";
        MACString << hex << setfill('0') << setw(2) << static_cast<unsigned short>(macOctets[4]);
        return MACString.str();
    }

    [[nodiscard]] string type() const {
        if (
            (this -> hextets[0]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[1]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[2]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[3]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[4]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[5]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[6]                                         == static_cast<unsigned short>(0x0000)) &&
            ((this -> hextets[7] & static_cast<unsigned short>(0xfffe)) == static_cast<unsigned short>(0x0000))
        ) {
            return "loopback";
        }
        if (
            (this -> hextets[0]                                         == static_cast<unsigned short>(0xff02)) &&
            (this -> hextets[1]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[2]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[3]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[4]                                         == static_cast<unsigned short>(0x0000)) &&
            (this -> hextets[5]                                         == static_cast<unsigned short>(0x0001)) &&
            ((this -> hextets[6] & static_cast<unsigned short>(0xff00)) == static_cast<unsigned short>(0xff00))
        ) {
            return "solicited node multicast";
        }
        if (
            (this -> hextets[0] == static_cast<unsigned short>(0)) &&
            (this -> hextets[1] == static_cast<unsigned short>(0)) &&
            (this -> hextets[2] == static_cast<unsigned short>(0)) &&
            (this -> hextets[3] == static_cast<unsigned short>(0)) &&
            (this -> hextets[4] == static_cast<unsigned short>(0)) &&
            (this -> hextets[5] == static_cast<unsigned short>(0))
        ) {
            return "IPv4 backwards compatible";
        }
        if (
            this -> hextets[0] == static_cast<unsigned short>(0x2001) ||
            this -> hextets[0] == static_cast<unsigned short>(0x2002) ||
            this -> hextets[0] == static_cast<unsigned short>(0x3ffe)
        ) {
            return "aggregatable global unicast or anycast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0xffc0)) == static_cast<unsigned short>(0xfe80)) {
            return "link-local unicast or anycast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0xffc0)) == static_cast<unsigned short>(0xfec0)) {
            return "site-local unicast or anycast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0xff00)) == static_cast<unsigned short>(0xff00)) {
            return "assigned multicast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0xfe00)) == static_cast<unsigned short>(0xfc00)) {
            return "unique local unicast";
        }
        if ((this -> hextets[0] & static_cast<unsigned short>(0xe000)) == static_cast<unsigned short>(0x2000)) {
            return "generic global unicast";
        }
        return "unspecified";
    }
};