#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <Arduino.h>

class StringUtils {
public:
    // Konvertiert deutsche Umlaute zu ASCII-Äquivalenten
    // ä -> ae, ö -> oe, ü -> ue, ß -> ss, etc.
    static String toASCII(const String& input);
    
    // Extrahiert nur den Stationsnamen (Teil nach dem Komma)
    // "Zürich, Bucheggplatz" -> "Bucheggplatz"
    static String getStationNameOnly(const String& fullName);
};

#endif // STRING_UTILS_H
