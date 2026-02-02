#include "StringUtils.h"

String StringUtils::toASCII(const String& input) {
    String output = "";
    
    for (unsigned int i = 0; i < input.length(); i++) {
        unsigned char c = input[i];
        
        // Prüfe ob es ein UTF-8 Multi-Byte Zeichen ist
        if (c >= 0xC0 && c <= 0xDF && i + 1 < input.length()) {
            // 2-Byte UTF-8 Zeichen
            unsigned char c2 = input[i + 1];
            
            // Deutsche Umlaute (UTF-8)
            if (c == 0xC3) {
                switch (c2) {
                    case 0xA4: output += "ae"; break; // ä
                    case 0x84: output += "Ae"; break; // Ä
                    case 0xB6: output += "oe"; break; // ö
                    case 0x96: output += "Oe"; break; // Ö
                    case 0xBC: output += "ue"; break; // ü
                    case 0x9C: output += "Ue"; break; // Ü
                    case 0x9F: output += "ss"; break; // ß
                    case 0xA9: output += "e";  break; // é
                    case 0x89: output += "E";  break; // É
                    case 0xA8: output += "e";  break; // è
                    case 0x88: output += "E";  break; // È
                    case 0xAA: output += "e";  break; // ê
                    case 0x8A: output += "E";  break; // Ê
                    case 0xA0: output += "a";  break; // à
                    case 0x80: output += "A";  break; // À
                    default: 
                        // Unbekanntes Zeichen, nehme Original
                        output += (char)c;
                        output += (char)c2;
                        break;
                }
                i++; // Skip next byte
            } else {
                // Anderes 2-Byte Zeichen, behalte Original
                output += (char)c;
            }
        } else if (c >= 0x20 && c < 0x7F) {
            // Normales ASCII Zeichen (32-126)
            output += (char)c;
        } else if (c == 0x0A || c == 0x0D) {
            // Newline / Carriage Return
            output += (char)c;
        }
        // Alle anderen Zeichen werden ignoriert
    }
    
    return output;
}

String StringUtils::getStationNameOnly(const String& fullName) {
    // Finde das letzte Komma (für Fälle wie "Ort, Station, Detail")
    int commaIndex = fullName.lastIndexOf(',');
    
    if (commaIndex > 0 && commaIndex < fullName.length() - 1) {
        // Nimm den Teil NACH dem Komma (rechte Seite)
        String stationName = fullName.substring(commaIndex + 1);
        stationName.trim();
        return stationName;
    }
    
    // Kein Komma gefunden, gib den ganzen Namen zurück
    return fullName;
}
