#include "WebConfigModule.h"
#include "../Logger/Logger.h"
#include <ESPmDNS.h>

WebConfigModule::WebConfigModule() : server(80), configStore(NULL), wifiManager(NULL), transportModule(NULL) {}

void WebConfigModule::begin(ConfigStore* config, WifiManager* wifi, TransportModule* transport) {
    this->configStore = config;
    this->wifiManager = wifi;
    this->transportModule = transport;
    
    if(!LittleFS.begin(true)){
        Logger::error("WEB", "An Error has occurred while mounting LittleFS");
        return;
    }
    
    setupRoutes();
    server.begin();
    Logger::info("WEB", "Web Server started");

    // Start mDNS
    if (MDNS.begin("crowpanel")) {
        Logger::info("WEB", "mDNS responder started: http://crowpanel.local");
        MDNS.addService("http", "tcp", 80);
    } else {
        Logger::error("WEB", "Error starting mDNS responder!");
    }
}

void WebConfigModule::setupRoutes() {
    // CORS Headers für Entwicklung (optional, aber hilfreich)
    // WICHTIG: Muss VOR den Routes gesetzt werden!
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    
    // API Routes MÜSSEN vor serveStatic definiert werden!
    
    // API: Status
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleStatus(request);
    });
    
    // API: Scan Start
    server.on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleScan(request);
    });

    // API: Scan Results
    server.on("/api/scan-results", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleScanResults(request);
    });
    
    // API: Config Save (POST)
    // Wir nutzen den Body Handler für JSON Payload
    server.on("/api/config", HTTP_POST, 
        [](AsyncWebServerRequest *request) { /* Response handled in Body Handler */ },
        NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            this->handleConfigSave(request, data, len, index, total);
        }
    );

    // API: Factory Reset (POST)
    server.on("/api/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Logger::info("WEB", "Factory Reset requested via Web");
        this->configStore->resetToFactory();
        request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Resetting...\"}");
        delay(1000);
        ESP.restart();
    });

    // API: Stop Search (GET)
    server.on("/api/stops/search", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handleStopSearch(request);
    });
    
    // Static Files - MUSS am Ende stehen, da "/" alles matched
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
}

void WebConfigModule::handleStatus(AsyncWebServerRequest *request) {
    JsonDocument doc;
    
    doc["ip"] = wifiManager->getIpAddress();
    doc["state"] = (int)wifiManager->getState();
    doc["heap"] = ESP.getFreeHeap();
    
    // Config Status
    doc["configured"] = configStore->hasWifiConfig();
    
    // Current station config
    StationConfig station = configStore->getStation();
    doc["station"]["name"] = station.name;
    doc["station"]["id"] = station.id;
    
    // Current line configs
    LineConfig line1 = configStore->getLine1();
    doc["line1"]["name"] = line1.name;
    doc["line1"]["dir"] = line1.direction;
    
    LineConfig line2 = configStore->getLine2();
    doc["line2"]["name"] = line2.name;
    doc["line2"]["dir"] = line2.direction;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void WebConfigModule::handleScan(AsyncWebServerRequest *request) {
    int scanStatus = WiFi.scanComplete();
    
    if (scanStatus == -1) {
        // Scan läuft bereits
        Logger::info("WEB", "Scan already running");
        request->send(200, "application/json", "{\"status\":\"running\",\"message\":\"Scan already in progress\"}");
    } else {
        // Neuen Scan starten (async = true)
        Logger::info("WEB", "Starting async WiFi scan...");
        WiFi.scanNetworks(true); 
        request->send(202, "application/json", "{\"status\":\"started\",\"message\":\"Scan started\"}");
    }
}

void WebConfigModule::handleScanResults(AsyncWebServerRequest *request) {
    int n = WiFi.scanComplete();
    JsonDocument doc;

    if (n == -2) {
        // Scan fehlgeschlagen
        doc["status"] = "failed";
        doc["message"] = "Scan failed";
        Logger::error("WEB", "WiFi Scan failed");
        
        String response;
        serializeJson(doc, response);
        request->send(500, "application/json", response);
        
    } else if (n == -1) {
        // Scan läuft noch
        doc["status"] = "running";
        doc["message"] = "Scan in progress";
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
        
    } else {
        // Scan fertig
        doc["status"] = "complete";
        doc["count"] = n;
        JsonArray networks = doc["networks"].to<JsonArray>();
        
        for (int i = 0; i < n; ++i) {
            JsonObject net = networks.add<JsonObject>();
            net["ssid"] = WiFi.SSID(i);
            net["rssi"] = WiFi.RSSI(i);
            net["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
        
        Logger::printf("WEB", "Scan complete, found %d networks", n);
        
        // Scan-Ergebnisse löschen, um Speicher freizugeben
        WiFi.scanDelete();
    }
}

void WebConfigModule::handleConfigSave(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Einfaches Handling: Wir nehmen an, dass das JSON in einen Chunk passt.
    // Für größere Payloads müsste man puffern.
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);
    
    if (error) {
        Logger::error("WEB", "JSON Parsing failed");
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
        return;
    }
    
    Logger::info("WEB", "Received new config");
    
    // Wifi - nur speichern wenn SSID nicht leer ist (Passwort kann leer sein für offene Netzwerke)
    if (doc["ssid"].is<const char*>()) {
        String ssid = doc["ssid"].as<String>();
        if (ssid.length() > 0) {
            String password = doc["password"].is<const char*>() ? doc["password"].as<String>() : "";
            configStore->setWifiCredentials(ssid, password);
            Logger::printf("WEB", "WiFi credentials updated: %s", ssid.c_str());
        }
    }
    
    // Station - nur speichern wenn ID nicht leer ist
    if (doc["station"].is<JsonObject>()) {
        JsonObject st = doc["station"];
        String stId = st["id"].as<String>();
        if (stId.length() > 0) {
            configStore->setStation(st["name"], stId);
            Logger::printf("WEB", "Station updated: %s", st["name"].as<String>().c_str());
        }
    }
    
    // Lines - nur speichern wenn Name nicht leer ist
    if (doc["line1"].is<JsonObject>()) {
        JsonObject l1 = doc["line1"];
        String l1Name = l1["name"].as<String>();
        if (l1Name.length() > 0) {
            configStore->setLine1(l1Name, l1["dir"]);
        }
    }
    
    if (doc["line2"].is<JsonObject>()) {
        JsonObject l2 = doc["line2"];
        String l2Name = l2["name"].as<String>();
        if (l2Name.length() > 0) {
            configStore->setLine2(l2Name, l2["dir"]);
        }
    }
    
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Saved. Restarting...\"}");
    
    // Verzögerter Neustart
    delay(1000);
    ESP.restart();
}

void WebConfigModule::handleStopSearch(AsyncWebServerRequest *request) {
    if (!request->hasParam("q")) {
        request->send(400, "application/json", "{\"error\":\"Missing query parameter 'q'\"}");
        return;
    }
    
    String query = request->getParam("q")->value();
    
    if (query.length() < 2) {
        request->send(400, "application/json", "{\"error\":\"Query too short (min 2 chars)\"}");
        return;
    }
    
    if (!transportModule) {
        request->send(500, "application/json", "{\"error\":\"TransportModule not available\"}");
        return;
    }
    
    Logger::printf("WEB", "Stop search request: %s", query.c_str());
    
    std::vector<StopSearchResult> stops = transportModule->searchStops(query);
    
    JsonDocument doc;
    JsonArray results = doc["results"].to<JsonArray>();
    
    for (const auto& stop : stops) {
        JsonObject obj = results.add<JsonObject>();
        obj["id"] = stop.id;
        obj["name"] = stop.name;
        obj["location"] = stop.topographicPlace;
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
