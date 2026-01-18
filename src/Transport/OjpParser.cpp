#include "OjpParser.h"
#include <tinyxml2.h>
#include <time.h>

using namespace tinyxml2;

String OjpParser::buildRequestXml(const String& stationId, const String& requestorRef, int limit) {
    // Aktuelle Zeit für Request (in UTC)
    time_t now;
    time(&now);
    struct tm* timeinfo = gmtime(&now);
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    
    // OJP 2.0 Format (für Endpoint /ojp20)
    String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml += "<OJP xmlns=\"http://www.vdv.de/ojp\" xmlns:siri=\"http://www.siri.org.uk/siri\" version=\"2.0\">";
    xml += "<OJPRequest>";
    xml += "<siri:ServiceRequest>";
    xml += "<siri:ServiceRequestContext><siri:Language>de</siri:Language></siri:ServiceRequestContext>";
    xml += "<siri:RequestTimestamp>" + String(timeStr) + "</siri:RequestTimestamp>";
    xml += "<siri:RequestorRef>" + requestorRef + "</siri:RequestorRef>";
    xml += "<OJPStopEventRequest>";
    xml += "<siri:RequestTimestamp>" + String(timeStr) + "</siri:RequestTimestamp>";
    xml += "<siri:MessageIdentifier>StopEvent1</siri:MessageIdentifier>";
    xml += "<Location>";
    xml += "<PlaceRef>";
    xml += "<siri:StopPointRef>" + stationId + "</siri:StopPointRef>";
    xml += "<Name><Text>Station</Text></Name>";
    xml += "</PlaceRef>";
    xml += "</Location>";
    xml += "<Params>";
    xml += "<NumberOfResults>" + String(limit) + "</NumberOfResults>";
    xml += "<StopEventType>departure</StopEventType>";
    xml += "<IncludePreviousCalls>false</IncludePreviousCalls>";
    xml += "<IncludeOnwardCalls>false</IncludeOnwardCalls>";
    xml += "<UseRealtimeData>full</UseRealtimeData>";
    xml += "</Params>";
    xml += "</OJPStopEventRequest>";
    xml += "</siri:ServiceRequest>";
    xml += "</OJPRequest>";
    xml += "</OJP>";
    
    return xml;
}

String OjpParser::buildLocationSearchXml(const String& query, const String& requestorRef) {
    // Aktuelle Zeit für Request (in UTC)
    time_t now;
    time(&now);
    struct tm* timeinfo = gmtime(&now);
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    
    // OJP 2.0 Format für LocationInformationRequest
    String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml += "<OJP xmlns=\"http://www.vdv.de/ojp\" xmlns:siri=\"http://www.siri.org.uk/siri\" version=\"2.0\">";
    xml += "<OJPRequest>";
    xml += "<siri:ServiceRequest>";
    xml += "<siri:ServiceRequestContext><siri:Language>de</siri:Language></siri:ServiceRequestContext>";
    xml += "<siri:RequestTimestamp>" + String(timeStr) + "</siri:RequestTimestamp>";
    xml += "<siri:RequestorRef>" + requestorRef + "</siri:RequestorRef>";
    xml += "<OJPLocationInformationRequest>";
    xml += "<siri:RequestTimestamp>" + String(timeStr) + "</siri:RequestTimestamp>";
    xml += "<siri:MessageIdentifier>LocationSearch1</siri:MessageIdentifier>";
    xml += "<InitialInput>";
    xml += "<Name>" + query + "</Name>";
    xml += "</InitialInput>";
    xml += "<Restrictions>";
    xml += "<Type>stop</Type>";
    xml += "<NumberOfResults>10</NumberOfResults>";
    xml += "<IncludePtModes>true</IncludePtModes>";
    xml += "</Restrictions>";
    xml += "</OJPLocationInformationRequest>";
    xml += "</siri:ServiceRequest>";
    xml += "</OJPRequest>";
    xml += "</OJP>";
    
    return xml;
}

std::vector<Departure> OjpParser::parseResponse(const String& xmlContent) {
    std::vector<Departure> departures;
    XMLDocument doc;
    
    // TinyXML2 erwartet char*, String muss gecastet werden
    // Wir nutzen c_str(), was read-only ist. Parse() modifiziert den String normalerweise nicht,
    // aber sicherheitshalber prüfen wir den Rückgabewert.
    XMLError err = doc.Parse(xmlContent.c_str());
    if (err != XML_SUCCESS) {
        Serial.printf("XML Parse Error: %d\n", err);
        return departures;
    }

    // Navigiere durch die OJP Struktur
    // <OJP> -> <OJPResponse> -> <ServiceDelivery> -> <OJPStopEventDelivery> -> <StopEventResult>
    
    XMLElement* root = doc.FirstChildElement("siri:OJP");
    // Fallback falls Namespaces im Root anders gehandhabt werden oder fehlen
    if (!root) root = doc.FirstChildElement("OJP"); 
    
    if (!root) {
        Serial.println("OJP Root not found");
        return departures;
    }

    XMLElement* response = root->FirstChildElement("siri:OJPResponse");
    if (!response) response = root->FirstChildElement("OJPResponse");
    
    if (!response) {
        Serial.println("OJPResponse not found");
        return departures;
    }

    XMLElement* serviceDelivery = response->FirstChildElement("siri:ServiceDelivery");
    if (!serviceDelivery) serviceDelivery = response->FirstChildElement("ServiceDelivery");

    if (!serviceDelivery) {
        Serial.println("ServiceDelivery not found");
        return departures;
    }

    XMLElement* stopEventDelivery = serviceDelivery->FirstChildElement("ojp:OJPStopEventDelivery");
    if (!stopEventDelivery) stopEventDelivery = serviceDelivery->FirstChildElement("OJPStopEventDelivery");
    
    if (!stopEventDelivery) {
        Serial.println("OJPStopEventDelivery not found");
        return departures;
    }

    // Iteriere über alle StopEventResult Elemente
    XMLElement* stopEventResult = stopEventDelivery->FirstChildElement("ojp:StopEventResult");
    if (!stopEventResult) stopEventResult = stopEventDelivery->FirstChildElement("StopEventResult");

    while (stopEventResult) {
        XMLElement* stopEvent = stopEventResult->FirstChildElement("ojp:StopEvent");
        if (!stopEvent) stopEvent = stopEventResult->FirstChildElement("StopEvent");

        if (stopEvent) {
            Departure dep;
            
            // ===== OJP 2.0 Struktur =====
            // StopEvent
            //   ├── ThisCall
            //   │   └── CallAtStop
            //   │       └── ServiceDeparture (Zeiten hier!)
            //   └── Service (Linien-Info hier, NICHT in ServiceDeparture!)
            
            // 1. Zeiten aus ThisCall/CallAtStop/ServiceDeparture
            XMLElement* thisCall = stopEvent->FirstChildElement("ojp:ThisCall");
            if (!thisCall) thisCall = stopEvent->FirstChildElement("ThisCall");
            
            if (thisCall) {
                // OJP 2.0: CallAtStop statt direkt ServiceDeparture
                XMLElement* callAtStop = thisCall->FirstChildElement("ojp:CallAtStop");
                if (!callAtStop) callAtStop = thisCall->FirstChildElement("CallAtStop");
                
                XMLElement* serviceDeparture = NULL;
                if (callAtStop) {
                    serviceDeparture = callAtStop->FirstChildElement("ojp:ServiceDeparture");
                    if (!serviceDeparture) serviceDeparture = callAtStop->FirstChildElement("ServiceDeparture");
                }
                // Fallback für andere Strukturen
                if (!serviceDeparture) {
                    serviceDeparture = thisCall->FirstChildElement("ojp:ServiceDeparture");
                    if (!serviceDeparture) serviceDeparture = thisCall->FirstChildElement("ServiceDeparture");
                }
                
                if (serviceDeparture) {
                    XMLElement* timeElem = serviceDeparture->FirstChildElement("ojp:TimetabledTime");
                    if (!timeElem) timeElem = serviceDeparture->FirstChildElement("TimetabledTime");
                    if (timeElem && timeElem->GetText()) {
                        dep.departureTime = parseIsoTime(timeElem->GetText());
                    }
                    
                    XMLElement* estTimeElem = serviceDeparture->FirstChildElement("ojp:EstimatedTime");
                    if (!estTimeElem) estTimeElem = serviceDeparture->FirstChildElement("EstimatedTime");
                    if (estTimeElem && estTimeElem->GetText()) {
                        dep.estimatedTime = parseIsoTime(estTimeElem->GetText());
                    } else {
                        dep.estimatedTime = 0;
                    }
                }
            }
            
            // 2. Service-Info direkt unter StopEvent (NICHT in ServiceDeparture!)
            XMLElement* service = stopEvent->FirstChildElement("ojp:Service");
            if (!service) service = stopEvent->FirstChildElement("Service");
            
            if (service) {
                // Linienname: PublishedServiceName (nicht PublishedLineName!)
                XMLElement* psn = service->FirstChildElement("ojp:PublishedServiceName");
                if (!psn) psn = service->FirstChildElement("PublishedServiceName");
                if (psn) {
                    XMLElement* textElem = psn->FirstChildElement("ojp:Text");
                    if (!textElem) textElem = psn->FirstChildElement("Text");
                    if (textElem && textElem->GetText()) {
                        dep.line = textElem->GetText();
                    } else if (psn->GetText()) {
                        dep.line = psn->GetText();
                    }
                }
                
                // Ziel: DestinationText -> Text
                XMLElement* destText = service->FirstChildElement("ojp:DestinationText");
                if (!destText) destText = service->FirstChildElement("DestinationText");
                if (destText) {
                    XMLElement* textElem = destText->FirstChildElement("ojp:Text");
                    if (!textElem) textElem = destText->FirstChildElement("Text");
                    if (textElem && textElem->GetText()) {
                        dep.direction = textElem->GetText();
                    } else if (destText->GetText()) {
                        dep.direction = destText->GetText();
                    }
                }
                
                // Verkehrsmittel: Mode -> PtMode
                XMLElement* modeElem = service->FirstChildElement("ojp:Mode");
                if (!modeElem) modeElem = service->FirstChildElement("Mode");
                if (modeElem) {
                    XMLElement* ptMode = modeElem->FirstChildElement("ojp:PtMode");
                    if (!ptMode) ptMode = modeElem->FirstChildElement("PtMode");
                    if (ptMode && ptMode->GetText()) {
                        dep.type = ptMode->GetText();
                    }
                }
            }
            
            // Nur hinzufügen wenn wir mindestens Abfahrtszeit haben
            if (dep.departureTime > 0) {
                departures.push_back(dep);
            }
        }
        
        // Nächstes StopEventResult
        XMLElement* nextResult = stopEventResult->NextSiblingElement("ojp:StopEventResult");
        if (!nextResult) nextResult = stopEventResult->NextSiblingElement("StopEventResult");
        stopEventResult = nextResult;
    }

    return departures;
}

std::vector<StopSearchResult> OjpParser::parseLocationSearchResponse(const String& xmlContent) {
    std::vector<StopSearchResult> results;
    XMLDocument doc;
    
    XMLError err = doc.Parse(xmlContent.c_str());
    if (err != XML_SUCCESS) {
        Serial.printf("XML Parse Error: %d\n", err);
        return results;
    }
    
    // Navigiere durch die OJP Struktur
    // <OJP> -> <OJPResponse> -> <ServiceDelivery> -> <OJPLocationInformationDelivery> -> <PlaceResult>
    
    XMLElement* root = doc.FirstChildElement("siri:OJP");
    if (!root) root = doc.FirstChildElement("OJP");
    
    if (!root) {
        Serial.println("OJP Root not found");
        return results;
    }
    
    XMLElement* response = root->FirstChildElement("siri:OJPResponse");
    if (!response) response = root->FirstChildElement("OJPResponse");
    
    if (!response) {
        Serial.println("OJPResponse not found");
        return results;
    }
    
    XMLElement* serviceDelivery = response->FirstChildElement("siri:ServiceDelivery");
    if (!serviceDelivery) serviceDelivery = response->FirstChildElement("ServiceDelivery");
    
    if (!serviceDelivery) {
        Serial.println("ServiceDelivery not found");
        return results;
    }
    
    XMLElement* locationDelivery = serviceDelivery->FirstChildElement("ojp:OJPLocationInformationDelivery");
    if (!locationDelivery) locationDelivery = serviceDelivery->FirstChildElement("OJPLocationInformationDelivery");
    
    if (!locationDelivery) {
        Serial.println("OJPLocationInformationDelivery not found");
        return results;
    }
    
    // Iteriere über alle PlaceResult Elemente
    XMLElement* placeResult = locationDelivery->FirstChildElement("ojp:PlaceResult");
    if (!placeResult) placeResult = locationDelivery->FirstChildElement("PlaceResult");
    
    while (placeResult) {
        XMLElement* place = placeResult->FirstChildElement("ojp:Place");
        if (!place) place = placeResult->FirstChildElement("Place");
        
        if (place) {
            XMLElement* stopPlace = place->FirstChildElement("ojp:StopPlace");
            if (!stopPlace) stopPlace = place->FirstChildElement("StopPlace");
            
            if (stopPlace) {
                StopSearchResult result;
                
                // StopPlaceRef (die ID die wir brauchen)
                XMLElement* refElem = stopPlace->FirstChildElement("ojp:StopPlaceRef");
                if (!refElem) refElem = stopPlace->FirstChildElement("StopPlaceRef");
                if (refElem && refElem->GetText()) {
                    result.id = refElem->GetText();
                }
                
                // StopPlaceName -> Text
                XMLElement* nameElem = stopPlace->FirstChildElement("ojp:StopPlaceName");
                if (!nameElem) nameElem = stopPlace->FirstChildElement("StopPlaceName");
                if (nameElem) {
                    XMLElement* textElem = nameElem->FirstChildElement("ojp:Text");
                    if (!textElem) textElem = nameElem->FirstChildElement("Text");
                    if (textElem && textElem->GetText()) {
                        result.name = textElem->GetText();
                    }
                }
                
                // TopographicPlaceName -> Text (optional)
                XMLElement* topoElem = stopPlace->FirstChildElement("ojp:TopographicPlaceName");
                if (!topoElem) topoElem = stopPlace->FirstChildElement("TopographicPlaceName");
                if (topoElem) {
                    XMLElement* textElem = topoElem->FirstChildElement("ojp:Text");
                    if (!textElem) textElem = topoElem->FirstChildElement("Text");
                    if (textElem && textElem->GetText()) {
                        result.topographicPlace = textElem->GetText();
                    }
                }
                
                // Nur hinzufügen wenn wir mindestens ID und Name haben
                if (result.id.length() > 0 && result.name.length() > 0) {
                    results.push_back(result);
                }
            }
        }
        
        // Nächstes PlaceResult
        XMLElement* nextResult = placeResult->NextSiblingElement("ojp:PlaceResult");
        if (!nextResult) nextResult = placeResult->NextSiblingElement("PlaceResult");
        placeResult = nextResult;
    }
    
    return results;
}

time_t OjpParser::parseIsoTime(const char* isoTime) {
    // Format: 2024-12-20T14:30:00Z (UTC) oder 2024-12-20T15:30:00+01:00 (mit Offset)
    struct tm tm = {0};
    
    int year, month, day, hour, minute, second;
    char tzSign = 0;
    int tzHour = 0, tzMin = 0;
    
    // Versuche Format mit Zeitzone zu parsen
    int parsed = sscanf(isoTime, "%d-%d-%dT%d:%d:%d%c%d:%d", 
                        &year, &month, &day, &hour, &minute, &second, 
                        &tzSign, &tzHour, &tzMin);
    
    if (parsed >= 6) {
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = 0;
        
        // Berechne den Offset der Zeit im String
        int inputOffsetSeconds = 0;
        if (parsed >= 7) {
            if (tzSign == 'Z') {
                inputOffsetSeconds = 0; // UTC
            } else if (tzSign == '+') {
                inputOffsetSeconds = tzHour * 3600 + tzMin * 60;
            } else if (tzSign == '-') {
                inputOffsetSeconds = -(tzHour * 3600 + tzMin * 60);
            }
        }
        
        // Berechne lokalen Offset durch Vergleich von localtime und gmtime
        time_t now = time(NULL);
        struct tm localTm, utcTm;
        localtime_r(&now, &localTm);
        gmtime_r(&now, &utcTm);
        
        // Lokaler Offset = lokale Zeit - UTC Zeit (in Sekunden)
        int localOffset = (localTm.tm_hour - utcTm.tm_hour) * 3600 +
                          (localTm.tm_min - utcTm.tm_min) * 60;
        // Korrektur für Tageswechsel
        if (localTm.tm_mday != utcTm.tm_mday) {
            if (localTm.tm_mday > utcTm.tm_mday || 
                (localTm.tm_mday == 1 && utcTm.tm_mday > 1)) {
                localOffset += 24 * 3600;
            } else {
                localOffset -= 24 * 3600;
            }
        }
        
        // mktime interpretiert tm als lokale Zeit
        time_t result = mktime(&tm);
        
        // Korrektur: Die Zeit im String hat inputOffsetSeconds
        // mktime hat localOffset angenommen
        // Also müssen wir die Differenz korrigieren
        result += (localOffset - inputOffsetSeconds);
        
        return result;
    }
    return 0;
}


