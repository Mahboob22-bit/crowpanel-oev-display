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

#endif // TRANSPORT_TYPES_H


