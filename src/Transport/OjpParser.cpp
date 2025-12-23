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
    
    // Einfacher String-Builder für XML Request
    String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml += "<OJP xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"http://www.siri.org.uk/siri\" version=\"1.0\" xmlns:ojp=\"http://www.vdv.de/ojp\" xsi:schemaLocation=\"http://www.siri.org.uk/siri ../ojp-xsd-v1.0/OJP.xsd\">";
    xml += "<OJPRequest>";
    xml += "<ServiceRequest>";
    xml += "<RequestTimestamp>" + String(timeStr) + "</RequestTimestamp>";
    xml += "<RequestorRef>" + requestorRef + "</RequestorRef>";
    xml += "<ojp:OJPStopEventRequest>";
    xml += "<RequestTimestamp>" + String(timeStr) + "</RequestTimestamp>";
    xml += "<ojp:Location>";
    xml += "<ojp:PlaceRef>";
    xml += "<ojp:StopPlaceRef>" + stationId + "</ojp:StopPlaceRef>";
    xml += "<ojp:LocationName>";
    xml += "<ojp:Text>Station</ojp:Text>"; 
    xml += "</ojp:LocationName>";
    xml += "</ojp:PlaceRef>";
    xml += "<ojp:DepArrTime>" + String(timeStr) + "</ojp:DepArrTime>";
    xml += "</ojp:Location>";
    xml += "<ojp:Params>";
    xml += "<ojp:NumberOfResults>" + String(limit) + "</ojp:NumberOfResults>";
    xml += "<ojp:StopEventType>departure</ojp:StopEventType>";
    xml += "<ojp:IncludeRealtimeData>true</ojp:IncludeRealtimeData>";
    xml += "</ojp:Params>";
    xml += "</ojp:OJPStopEventRequest>";
    xml += "</ServiceRequest>";
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
            XMLElement* service = stopEvent->FirstChildElement("ojp:ThisCall")->FirstChildElement("ojp:ServiceDeparture")->FirstChildElement("ojp:Service");
            if (!service) service = stopEvent->FirstChildElement("ThisCall")->FirstChildElement("ServiceDeparture")->FirstChildElement("Service");
            
            XMLElement* journey = stopEvent->FirstChildElement("ojp:Service"); // Manchmal direkt unter Service? Nein, Struktur ist komplexer.
            // Korrekte Struktur OJP 2.0 StopEvent:
            // StopEvent -> ThisCall -> ServiceDeparture
            
            XMLElement* thisCall = stopEvent->FirstChildElement("ojp:ThisCall");
            if (!thisCall) thisCall = stopEvent->FirstChildElement("ThisCall");
            
            if (thisCall) {
                 XMLElement* serviceDeparture = thisCall->FirstChildElement("ojp:ServiceDeparture");
                 if (!serviceDeparture) serviceDeparture = thisCall->FirstChildElement("ServiceDeparture");
                 
                 if (serviceDeparture) {
                     Departure dep;
                     
                     // Zeiten
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

                     // Service Infos
                     XMLElement* serviceInfo = serviceDeparture->FirstChildElement("ojp:Service");
                     if (!serviceInfo) serviceInfo = serviceDeparture->FirstChildElement("Service");
                     
                     if (serviceInfo) {
                         XMLElement* lineElem = serviceInfo->FirstChildElement("ojp:PublishedLineName");
                         if (!lineElem) lineElem = serviceInfo->FirstChildElement("PublishedLineName")->FirstChildElement("ojp:Text"); // OJP Struktur ist oft verschachtelt mit Text
                         // Manchmal ist PublishedLineName direkt TextContainer oder hat Text Child
                         
                         // In OJP 2.0 ist PublishedLineName -> Text
                         if (!lineElem) {
                             XMLElement* pln = serviceInfo->FirstChildElement("ojp:PublishedLineName");
                             if (!pln) pln = serviceInfo->FirstChildElement("PublishedLineName");
                             if (pln) lineElem = pln->FirstChildElement("ojp:Text");
                             if (!lineElem && pln) lineElem = pln->FirstChildElement("Text");
                         }
                         
                         if (lineElem && lineElem->GetText()) {
                             dep.line = lineElem->GetText();
                         }

                         XMLElement* dirElem = serviceInfo->FirstChildElement("ojp:DestinationText");
                         if (!dirElem) dirElem = serviceInfo->FirstChildElement("DestinationText");
                         // Auch hier oft -> Text
                         if (dirElem) {
                             XMLElement* textElem = dirElem->FirstChildElement("ojp:Text");
                             if (!textElem) textElem = dirElem->FirstChildElement("Text");
                             if (textElem && textElem->GetText()) {
                                 dep.direction = textElem->GetText();
                             } else if (dirElem->GetText()) {
                                 dep.direction = dirElem->GetText();
                             }
                         }
                         
                         XMLElement* modeElem = serviceInfo->FirstChildElement("ojp:Mode");
                         if (!modeElem) modeElem = serviceInfo->FirstChildElement("Mode");
                         if (modeElem) {
                             XMLElement* ptMode = modeElem->FirstChildElement("ojp:PtMode");
                             if (!ptMode) ptMode = modeElem->FirstChildElement("PtMode");
                             if (ptMode && ptMode->GetText()) {
                                 dep.type = ptMode->GetText();
                             }
                         }
                     }
                     
                     departures.push_back(dep);
                 }
            }
        }
        
        stopEventResult = stopEventResult->NextSiblingElement("ojp:StopEventResult");
        if (!stopEventResult) stopEventResult = stopEventResult->NextSiblingElement("StopEventResult");
    }

    return departures;
}

time_t OjpParser::parseIsoTime(const char* isoTime) {
    // Format: 2024-12-20T14:30:00Z oder mit Offset
    struct tm tm = {0};
    
    // Einfaches Parsing (sscanf)
    int year, month, day, hour, minute, second;
    if (sscanf(isoTime, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second) >= 6) {
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1; // Auto detect
        
        return mktime(&tm);
    }
    return 0;
}


