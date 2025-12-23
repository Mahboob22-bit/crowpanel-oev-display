#include "WebConfigModule.h"
#include "../Logger/Logger.h"
#include <ESPmDNS.h>

WebConfigModule::WebConfigModule() : server(80), configStore(NULL), wifiManager(NULL) {}

void WebConfigModule::begin(ConfigStore* config, WifiManager* wifi) {
    this->configStore = config;
    this->wifiManager = wifi;
    
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
    // Static Files
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
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
    
    // CORS Headers für Entwicklung (optional, aber hilfreich)
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
}

void WebConfigModule::handleStatus(AsyncWebServerRequest *request) {
    JsonDocument doc;
    
    doc["ip"] = wifiManager->getIpAddress();
    doc["state"] = (int)wifiManager->getState();
    doc["heap"] = ESP.getFreeHeap();
    
    // Config Status
    doc["configured"] = configStore->hasWifiConfig();
    
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
    
    // Wifi
    if (doc["ssid"].is<const char*>() && doc["password"].is<const char*>()) {
        configStore->setWifiCredentials(doc["ssid"], doc["password"]);
    }
    
    // Station
    if (doc["station"].is<JsonObject>()) {
        JsonObject st = doc["station"];
        configStore->setStation(st["name"], st["id"]);
    }
    
    // Lines
    if (doc["line1"].is<JsonObject>()) {
        JsonObject l1 = doc["line1"];
        configStore->setLine1(l1["name"], l1["dir"]);
    }
    
    if (doc["line2"].is<JsonObject>()) {
        JsonObject l2 = doc["line2"];
        configStore->setLine2(l2["name"], l2["dir"]);
    }
    
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Saved. Restarting...\"}");
    
    // Verzögerter Neustart
    delay(1000);
    ESP.restart();
}
