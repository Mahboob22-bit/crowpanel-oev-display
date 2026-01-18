#ifndef OJP_PARSER_H
#define OJP_PARSER_H

#include <Arduino.h>
#include <vector>
#include "TransportTypes.h"

class OjpParser {
public:
    // Parst die OJP XML Antwort und extrahiert Abfahrten
    static std::vector<Departure> parseResponse(const String& xmlContent);
    
    // Erstellt den XML Request Body für die OJP API
    static String buildRequestXml(const String& stationId, const String& requestorRef, int limit = 4);
    
    // Erstellt den XML Request Body für die Haltestellensuche (LocationInformationRequest)
    static String buildLocationSearchXml(const String& query, const String& requestorRef = "CrowPanel");
    
    // Parst die LocationInformationResponse und extrahiert Haltestellen
    static std::vector<StopSearchResult> parseLocationSearchResponse(const String& xmlContent);
    
    // Hilfsfunktion zum Parsen eines ISO 8601 Zeitstrings
    static time_t parseIsoTime(const char* isoTime);
};

#endif // OJP_PARSER_H


