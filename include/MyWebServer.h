#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "SPIFFS.h"

#include <vector>
#include <FS.h> // Include SPIFFS filesystem library

class MyWebServer
{
private:
    /* data */
    int port;
    const char* ssid;
    const char* password;
    WebServer *server;

    void startWifi();
    void startFS();
    void createFileHandlers(); // Dynamically create handlers for files
    void handleFileRequest(const String &filePath); // Handle specific file requests
    String getContentType(const String &uri); 
    void handleNotFound();

    void handleSaveConfig(); // Handle saving configuration
    void handleReadConfig(); // Handle reading configuration

public:
    MyWebServer();
    ~MyWebServer();

    void setup();
    void handle();
};

#endif