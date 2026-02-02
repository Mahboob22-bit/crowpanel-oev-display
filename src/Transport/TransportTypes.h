#ifndef TRANSPORT_TYPES_H
#define TRANSPORT_TYPES_H

#include <Arduino.h>
#include <time.h>

struct Departure {
    String line;        // Liniennummer (z.B. "11")
    String direction;   // Zielort (z.B. "Auzelg")
    time_t departureTime; // Geplante Abfahrtszeit
    time_t estimatedTime; // Prognostizierte Abfahrtszeit (falls verfügbar)
    String type;        // Verkehrsmittel (Bus, Tram, Train, etc.)
    
    // Hilfsfunktion: Gibt die effektive Zeit zurück (Estimated falls vorhanden, sonst Planned)
    time_t getEffectiveTime() const {
        return (estimatedTime > 0) ? estimatedTime : departureTime;
    }
};

struct StopSearchResult {
    String id;                // z.B. "8503000"
    String name;              // z.B. "Zürich HB"
    String topographicPlace;  // z.B. "Zürich"
};

struct LineInfo {
    String line;        // z.B. "10"
    String direction;   // z.B. "Dornach"
    String type;        // bus, tram, train, metro, etc.
};

#endif // TRANSPORT_TYPES_H


