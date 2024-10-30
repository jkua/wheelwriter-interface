// Wheelwriter REST API class

#pragma once 

#include <Arduino.h>
#include <WiFiNINA.h>

#include "PicoRest.h"
#include "Wheelwriter.h"

class WheelwriterRestApi : public PicoRest::PicoRestApi {
public:
  WheelwriterRestApi(WiFiServer& server, wheelwriter::Wheelwriter& typewriter) : PicoRest::PicoRestApi(server), typewriter_(typewriter) {}
  void handlePostRequest(WiFiClient& client, PicoRest::HttpRequest& request) override {
    if (request.path == "/type") {
      for (int i = 0; i < request.content.length(); i++) {
        if (request.content[i] == 0x0a) {
          typewriter_.carriageReturn();
          typewriter_.lineFeed();
        }
        else {
          typewriter_.typeAscii(request.content[i]);
        }
      }
    }
    else {
      sendGenericResponse(client, PicoRest::HttpResponse::StatusCode::NOT_FOUND);
    }
  }
  void sendDefaultWebpage(WiFiClient& client) override {
    // HTTP response header
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    // HTML content
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<body>");
    client.println("<h1>Wheelwriter Interface Board</h1>");
    client.println("<p>Created by: <a href=https://github.com/jkua>John Kua</a></p>");
    client.println("<p><a href=https://github.com/jkua/wheelwriter-interface>GitHub</a></p>");
    client.println("</body>");
    client.println("</html>");
  }
  
private:
  wheelwriter::Wheelwriter& typewriter_;
  
};
