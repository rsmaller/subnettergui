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
        string stringToManipulate = stringArg;
        string zeroBlockString = "";

        smatch zeroHextetMatch;
        regex IPv6ZeroHextetRegex("\\:0\\:");

        smatch leadingZeroMatch;
        regex IPv6LeadingZeroRegex("\\:[1-9a-fA-F]{1,3}\\:");

        smatch doubleColonMatch;
        regex IPv6DoubleColonRegex("\\:\\:");

        smatch IPv6FormatMatch;
        regex IPv6FormatRegex("^[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}\\:[0-9a-fA-F]{1,4}$");

        while (regex_search(stringToManipulate, zeroHextetMatch, IPv6ZeroHextetRegex)) {
            stringToManipulate = stringToManipulate.substr(0, zeroHextetMatch.position()) + ":0000:" + stringToManipulate.substr(zeroHextetMatch.position() + zeroHextetMatch.length(), stringToManipulate.length() - 1);
        }

        while (regex_search(stringToManipulate, leadingZeroMatch, IPv6LeadingZeroRegex)) {
            stringToManipulate = stringToManipulate.substr(0, leadingZeroMatch.position()) + ":" + string(6 - leadingZeroMatch.length(), '0') + stringToManipulate.substr(leadingZeroMatch.position() + 1, stringToManipulate.length() - 1);
        }

        if (regex_search(stringToManipulate, doubleColonMatch, IPv6DoubleColonRegex)) {
            int lengthDifference = 39 - (int)stringToManipulate.length();
            int numberOfZeroesToAdd = lengthDifference - (lengthDifference % 4);
            int numberOfColonsToAdd = numberOfZeroesToAdd / 4 - 1;
            while (numberOfZeroesToAdd) {
                for (int i=0; i<4; i++) {
                    zeroBlockString += "0";
                }
                if (numberOfColonsToAdd) {
                    zeroBlockString += ":";
                    numberOfColonsToAdd--;
                }
                numberOfZeroesToAdd -= 4;
            }
            stringToManipulate = stringToManipulate.substr(0, doubleColonMatch.position() + 1) + zeroBlockString + stringToManipulate.substr(doubleColonMatch.position() + 1, stringToManipulate.length() - 1);
        }

        if (!regex_search(stringToManipulate, IPv6FormatMatch, IPv6FormatRegex)) {
            return "0000:0000:0000:0000:0000:0000:0000:0000";
        }
        return stringToManipulate;
    }

    static string IPv6Truncate(string stringArg) {
        if (!stringArg.compare("0000:0000:0000:0000:0000:0000:0000:0000")) {
            return "::";
        }

        string sanitizedString = IPv6Sanitize(stringArg);
        string truncatedString = sanitizedString;
        string greediestMatch = "";

        smatch greedyZeroMatch;
        regex greedyZeroRegex("\\:[0:]+\\:");

        smatch leadingZeroMatch;
        regex leadingZeroRegex("\\:[0]{1,3}(?!:)(?!$)"); // zero preceded by a colon but not followed by a colon

        int index;

        while (regex_search(truncatedString, leadingZeroMatch, leadingZeroRegex)) {
            truncatedString = truncatedString.substr(0, leadingZeroMatch.position()+1) + truncatedString.substr(leadingZeroMatch.position() + leadingZeroMatch.length(), truncatedString.length() - 1);
        }
        sanitizedString = truncatedString;

        while (regex_search(sanitizedString, greedyZeroMatch, greedyZeroRegex)) { // truncate the longest block of zeroes into a double colon
            if ((int)greedyZeroMatch[0].length() > (int)greediestMatch.length()) {
                greediestMatch = greedyZeroMatch[0];
            }
            sanitizedString = sanitizedString.substr(0, greedyZeroMatch.position()) + ":" + sanitizedString.substr(greedyZeroMatch.position() + greedyZeroMatch.length(), sanitizedString.length() - 1);
        }
        if (greediestMatch.compare("")) {
            index = (int)truncatedString.find(greediestMatch);
            truncatedString = truncatedString.substr(0, index) + "::" + truncatedString.substr(index + greediestMatch.length(), truncatedString.length() - 1);
        } // end of zero block truncation
        return truncatedString;
    }

    static unsigned short *IPv6StringToHextets(string stringArg) {
        stringArg = IPv6Sanitize(stringArg);
        int currentIndex = 0;
        string currentString = "";
        unsigned short *hextets = (unsigned short *)calloc(8, 2);
        for (int i=0; i<8; i++) {
            currentString = "";
            while (stringArg[currentIndex] != ':' && (size_t)currentIndex < stringArg.length()) {
                currentString += stringArg[currentIndex];
                currentIndex++;
            }
            currentIndex++;
            hextets[i] = (unsigned short)stoi("0x" + currentString, nullptr, 16);
        }
        return hextets;
    }
};