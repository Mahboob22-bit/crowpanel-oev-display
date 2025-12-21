#ifndef OJP_PARSER_H
#define OJP_PARSER_H

#include <Arduino.h>
#include <vector>
#include "TransportTypes.h"

class OjpParser {
public:
    // Parst die OJP XML Antwort und extrahiert Abfahrten
    static std::vector<Departure> parseResponse(const String& xmlContent);
    
    // Hilfsfunktion zum Parsen eines ISO 8601 Zeitstrings
    static time_t parseIsoTime(const char* isoTime);
};

#endif // OJP_PARSER_H


