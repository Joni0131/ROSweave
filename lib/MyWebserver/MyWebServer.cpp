#include "MyWebServer.h"

MyWebServer::MyWebServer()
{
    // Constructor implementation
    this->port = 80;              // Default port
    this->ssid = "MyESP32AP";     // Default SSID
    this->password = ""; // Default password
}

MyWebServer::~MyWebServer()
{
    // Destructor implementation
}

void MyWebServer::startWifi()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(this->ssid, this->password);
    Serial.println("WiFi started in AP mode");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
}

void MyWebServer::startFS()
{
    if (!SPIFFS.begin())
    {
        Serial.println("Failed to start filesystem.");
    }
    else
    {
        Serial.println("Filesystem started.");
        // Check if the server folder exists
        if (!SPIFFS.exists("/index.html"))
        {
            Serial.println("Server not found.");
        }
        else
        {
            Serial.println("Server found.");
        }
    }
}

void MyWebServer::setup()
{
    startWifi();
    startFS();

    this->server = new WebServer(this->port);

    // Dynamically create handlers for all files in the /server folder
    createFileHandlers();

    // Add API endpoints for saving and loading configuration
    this->server->on("/api/save-config", HTTP_POST, [this]() {
        handleSaveConfig();
    });

    this->server->on("/api/read-config", HTTP_GET, [this]() {
        handleReadConfig();
    });

    this->server->begin();
    Serial.println("HTTP server started");
}

void MyWebServer::createFileHandlers()
{
    // List all files in the root directory
    File root = SPIFFS.open("/");
    if (!root || !root.isDirectory())
    {
        Serial.println("Failed to open root directory.");
        return;
    }

    // Create handler for root to /index.html
    this->server->on("/", HTTP_GET, [this]()
    {
        handleFileRequest("/index.html");
    });

    File file = root.openNextFile();
    while (file)
    {
        String filePath = "/" + String(file.name());
        Serial.println("Creating handler for: " + filePath);

        // Create a handler for each file
        this->server->on(filePath.c_str(), HTTP_GET, [this, filePath]()
        {
            handleFileRequest(filePath);
        });

        file = root.openNextFile();
    }
}

void MyWebServer::handleFileRequest(const String &filePath)
{
    Serial.println("Serving file: " + filePath);

    String contentType = getContentType(filePath);

    Serial.println("Trying opening file: " + filePath);
    Serial.println("Content type: " + contentType);
    Serial.println("File exists: " + String(SPIFFS.exists(filePath) ? "true" : "false"));

    if (SPIFFS.exists(filePath))
    {
        File file = SPIFFS.open(filePath, "r");
        this->server->streamFile(file, contentType);
        file.close();
    }
    else
    {
        this->server->send(404, "text/plain", "404: Not Found");
    }
}

String MyWebServer::getContentType(const String &uri)
{
    if (uri.endsWith(".html"))
        return "text/html";
    else if (uri.endsWith(".css"))
        return "text/css";
    else if (uri.endsWith(".js"))
        return "text/javascript";
    else if (uri.endsWith(".json"))
        return "application/json";
    else if (uri.endsWith(".png"))
        return "image/png";
    else if (uri.endsWith(".jpg") || uri.endsWith(".jpeg"))
        return "image/jpeg";
    else if (uri.endsWith(".ico"))
        return "image/x-icon";
    else if (uri.endsWith(".svg"))
        return "image/svg+xml";
    return "text/plain";
}

void MyWebServer::handleSaveConfig()
{
    if (this->server->hasArg("plain"))
    {
        String body = this->server->arg("plain");
        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            this->server->send(500, "text/plain", "Failed to open config file for writing");
            return;
        }
        configFile.print(body);
        configFile.close();
        this->server->send(200, "application/json", "{\"status\":\"success\"}");
        Serial.println("Configuration saved successfully.");
    }
    else
    {
        this->server->send(400, "text/plain", "Bad Request: No configuration data received");
    }
}

void MyWebServer::handleReadConfig()
{
    if (SPIFFS.exists("/config.json"))
    {
        File configFile = SPIFFS.open("/config.json", "r");
        if (!configFile)
        {
            this->server->send(500, "text/plain", "Failed to open config file for reading");
            return;
        }
        String configData = configFile.readString();
        configFile.close();
        this->server->send(200, "application/json", configData);
        Serial.println("Configuration sent successfully.");
    }
    else
    {
        this->server->send(404, "text/plain", "Configuration file not found");
    }
}

void MyWebServer::handle()
{
    this->server->handleClient();
}