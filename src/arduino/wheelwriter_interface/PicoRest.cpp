// PicoRest - simple REST API class
// Copyright (c) 2024 John Kua <john@kua.fm>
//
#include <Arduino.h>
#include <WiFiNINA.h>

#include "PicoRest.h"

namespace PicoRest {

int HttpRequest::parseChar(char c) {
  switch (parseState) {
    case (REQUEST):
    case (HEADER): {
      if (c == '\n') {
        parseLine(buffer);
        numHeaderLines++;
        buffer.clear();
      }
      else {
        if (c != '\r') {
          buffer += c;
        }
      }
      return 1;
    }
    case (CONTENT): {
      content += c;
      if (content.length() == contentLength) {
        parseState = DONE;
      }
      return 1;
    } 
    case (DONE): {
      return 0;
    }
  }
}
void HttpRequest::print() {
  Serial.println("\nHTTP request");
  Serial.println("------------");
  Serial.print("--> Method: ");
  switch (method) {
    case GET: {
      Serial.println("GET");
      break;
    }
    case POST: {
      Serial.println("POST");
      break;
    }
    case PUT: {
      Serial.println("PUT");
      break;
    }
    case DELETE: {
      Serial.println("DELETE");
      break;
    }
  }
  Serial.print("--> Path: ");
  Serial.println(path.c_str());
  Serial.print("--> Host: ");
  Serial.println(host.c_str());
  Serial.print("--> User-Agent: ");
  Serial.println(userAgent.c_str());
  Serial.print("--> Content-Type: ");
  Serial.println(contentType.c_str());
  Serial.print("--> Content-Length: ");
  Serial.println(contentLength);
  Serial.print("--> Content: ");
  Serial.println(content.c_str());
}
int HttpRequest::parseLine(const std::string& line) {
  switch (parseState) {
    case (REQUEST): {
      return parseRequest(line);
    }
    case (HEADER): {
      return parseHeader(line);
    }
    default: {
      error = true;
      return 0;
    }
  }
}
int HttpRequest::parseRequest(const std::string& line) {
  if (line.find("GET") != std::string::npos) {
    method = GET;
  } else if (line.find("POST") != std::string::npos) {
    method = POST;
  } else if (line.find("PUT") != std::string::npos) {
    method = PUT;
  } else if (line.find("DELETE") != std::string::npos) {
    method = DELETE;
  } else {
    error = true;
    return 0;
  }
  size_t start = line.find(" ") + 1;
  size_t end = line.find(" ", start);
  path = line.substr(start, end - start);
  parseState = HEADER;
  return 1;
}
int HttpRequest::parseHeader(const std::string& line) {
  if (line.length() == 0) {
    if (contentLength != 0) {
      parseState = CONTENT;
    }
    else {
      parseState = DONE;
    }
    return 2;
  }
  if (line.find("Host") != std::string::npos) {
    size_t start = line.find(":") + 2;
    host = line.substr(start);
  } else if (line.find("User-Agent") != std::string::npos) {
    size_t start = line.find(":") + 2;
    userAgent = line.substr(start);
  } else if (line.find("Content-Type") != std::string::npos) {
    size_t start = line.find(":") + 2;
    contentType = line.substr(start);
  } else if (line.find("Content-Length") != std::string::npos) {
    size_t start = line.find(":") + 2;
    contentLength = std::stoi(line.substr(start));
  }
  return 1;
}

const std::map<HttpResponse::StatusCode, std::string> HttpResponse::statusStrings = { 
    { OK, "OK" },
    { BAD_REQUEST, "Bad Request" },
    { FORBIDDEN, "Forbidden" },
    { NOT_FOUND, "Not Found" },
    { METHOD_NOT_ALLOWED, "Method Not Allowed" },
    { LENGTH_REQUIRED, "Length Required" },
    { CONTENT_TOO_LARGE, "Content Too Large" },
    { INTERNAL_SERVER_ERROR, "Internal Server Error" },
    { NOT_IMPLEMENTED, "Not Implemented" },
    { SERVICE_UNAVAILABLE, "Service Unavailable" }
}; 

int PicoRestApi::init() {
  Serial.println("*** Starting WiFi");
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("--> Communication with WiFi module failed!");
    // Don't continue
    while (true);
  }

  WiFi.macAddress(macAddress_);
  Serial.print("--> MAC address: ");
  printMacAddress();
  Serial.write('\n');

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("--> Installed firmware: " + fv);
    Serial.print("--> Latest firmware: ");
    Serial.println(WIFI_FIRMWARE_LATEST_VERSION);
    Serial.println("--> *** Please upgrade the firmware!");
  }
}
int PicoRestApi::connect(const char* ssid, const char* password) {
  int retries = 3;
  for (int i = 0; i < retries; i++) {
    Serial.print("--> Attempting to connect to SSID: ");
    Serial.println(ssid);
    status_ = WiFi.begin(ssid, password);
    if (status_ == WL_CONNECTED) {
      break;
    }
    delay(3000);
  }
  if (status_ != WL_CONNECTED) {
    Serial.println("*** Failed to connect!");
    return 0;
  }
  server_.begin();
  printWifiStatus();
  return 1;
}
int PicoRestApi::listNetworks() {
  // scan for nearby networks:
  Serial.println("\n**** Scanning for networks...");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("--> ERROR: Couldn't get a WiFi connection");
  }

  // print the list of networks seen:
  Serial.print("Number of available networks: ");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    char buffer[256];
    sprintf(buffer, "%2d) %-32s | Signal: %4d dBm | Channel: %2d | Encryption: ", 
            thisNet,
            WiFi.SSID(thisNet),
            WiFi.RSSI(thisNet),
            WiFi.channel(thisNet)
            );
    Serial.print(buffer);
    byte encryption = WiFi.encryptionType(thisNet);
    switch (encryption) {
      case ENC_TYPE_TKIP: {
        Serial.println("TKIP (WPA)");
        break;
      }
      case ENC_TYPE_CCMP: {
        Serial.println("CCMP (WPA2)");
        break;
      }
      case ENC_TYPE_WEP: {
        Serial.println("WEP");
        break;
      }
      case ENC_TYPE_NONE: {
        Serial.println("None");
        break;
      }
      case ENC_TYPE_AUTO: {
        Serial.println("Auto");
        break;
      }
      default: {
        Serial.println("Unknown");
        break;
      }
    }
  }
  return numSsid;
}
void PicoRestApi::printWifiStatus() {
  Serial.print("--> SSID: ");
  Serial.println(WiFi.SSID());

  ipAddress_ = WiFi.localIP();
  Serial.print("    IP Address: ");
  Serial.println(ipAddress_);

  long rssi = WiFi.RSSI();
  Serial.print("    Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}
void PicoRestApi::printMacAddress() {
  for (int i = 5; i >= 0; i--) {
    Serial.print(macAddress_[i], HEX);
    if (i) {
      Serial.print(":");
    }
  }
}
int PicoRestApi::processClient() {
  WiFiClient client = server_.available();

  if (client) {
    Serial.println("\n*** New WiFi client\n");
    HttpRequest request;

    // Read the request from the client
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        // Serial.write(c);
        request.parseChar(c);
        if (request.parseState == HttpRequest::DONE) {
          break;
        }
      }
    }

    if (request.error) {
      Serial.println("--> Error parsing request");
      return 0;
    }
    if (request.contentLength != request.content.length()) {
      Serial.println("--> Content length mismatch! Expected " + String(request.contentLength) + " but got " + String(request.content.length()));
      return 0;
    }

    if ((request.method == HttpRequest::POST) || (request.method == HttpRequest::PUT)) {
      Serial.println();
    }
    Serial.println("--> Request parsed");
    request.print();

    switch (request.method) {
      case HttpRequest::GET: {
        handleGetRequest(client, request);
        break;
      }
      case HttpRequest::HEAD: {
        handleHeadRequest(client, request);
        break;
      }
      case HttpRequest::POST: {
        handlePostRequest(client, request);
        break;
      }
      case HttpRequest::PUT: {
        handlePutRequest(client, request);
        break;
      }
      case HttpRequest::DELETE: {
        handleDeleteRequest(client, request);
        break;
      }
      default: {
        sendGenericResponse(client, HttpResponse::StatusCode::NOT_IMPLEMENTED);
      }
    }

    // close the connection:
    client.stop();
    Serial.println("\nClient disconnected.");
    return 1;
  }
  return 0;
}
void PicoRestApi::handleGetRequest(WiFiClient& client, HttpRequest& request) {
  if (request.path == "/") {
    sendDefaultWebpage(client);
  }
  else {
    sendGenericResponse(client, HttpResponse::StatusCode::NOT_FOUND);
  }
}
void PicoRestApi::handleHeadRequest(WiFiClient& client, HttpRequest& request) {
  if (request.path == "/") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
  }
  else {
    sendGenericResponse(client, HttpResponse::StatusCode::NOT_FOUND);
  }
}
void PicoRestApi::handlePostRequest(WiFiClient& client, HttpRequest& request) {
  sendGenericResponse(client, HttpResponse::StatusCode::NOT_IMPLEMENTED);
}
void PicoRestApi::handlePutRequest(WiFiClient& client, HttpRequest& request) {
  sendGenericResponse(client, HttpResponse::StatusCode::NOT_IMPLEMENTED);
}
void PicoRestApi::handleDeleteRequest(WiFiClient& client, HttpRequest& request) {
  sendGenericResponse(client, HttpResponse::StatusCode::NOT_IMPLEMENTED);
}
void PicoRestApi::sendDefaultWebpage(WiFiClient& client) {
  // HTTP response header
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  // HTML content
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<body>");
  client.println("<h1>PicoRestApi</h1>");
  client.println("<p>Created by: <a href=https://github.com/jkua>John Kua</a></p>");
  client.println("<p><a href=https://github.com/jkua/pico_rest>GitHub</a></p>");
  client.println("</body>");
  client.println("</html>");
}
void PicoRestApi::sendGenericResponse(WiFiClient& client, HttpResponse::StatusCode status) {
  HttpResponse response(status);
  client.println(response.header().c_str());
  client.println(response.body().c_str());
}

} // namespace PicoRest
