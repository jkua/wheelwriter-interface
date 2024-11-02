// Wheelwriter REST API class
// Copyright (c) 2024 John Kua <john@kua.fm>
//
#pragma once 

#include <Arduino.h>
#include <WiFiNINA.h>

#include "PicoRest.h"
#include "Wheelwriter.h"
#include "utility.h"


class WheelwriterRestApi : public PicoRest::PicoRestApi {
public:
  WheelwriterRestApi(WiFiServer& server, wheelwriter::Wheelwriter& typewriter) : PicoRest::PicoRestApi(server), typewriter_(typewriter) {}
  void handlePostRequest(WiFiClient& client, PicoRest::HttpRequest& request) override {
    if (request.path == "/bufferTest") {
      ParameterString parameters(request.content);
      uint16_t numChars = parameters.getParameterInt(0, 10);
      uint8_t charsPerLine = parameters.getParameterInt(1, 80);
      typewriter_.bufferTest(numChars, charsPerLine);
      sendGenericResponse(client, PicoRest::HttpResponse::StatusCode::OK);
    }
    else if (request.path == "/characterTest") {
      wheelwriter::ww_typestyle typestyle = wheelwriter::TYPESTYLE_NORMAL;
      if (request.content == "bold") {
        typestyle = wheelwriter::TYPESTYLE_BOLD;
      }
      else if (request.content == "underline") {
        typestyle = wheelwriter::TYPESTYLE_UNDERLINE;
      }
      typewriter_.characterTest(typestyle);
      sendGenericResponse(client, PicoRest::HttpResponse::StatusCode::OK);
    }
    else if (request.path == "/circleTest") {
      typewriter_.circleTest();
      sendGenericResponse(client, PicoRest::HttpResponse::StatusCode::OK);
    }
    else if (request.path == "/printwheelSample") {
      ParameterString parameters(request.content);
      uint8_t plusPosition = parameters.getParameterInt(0, 0x3b);
      uint8_t underscorePosition = parameters.getParameterInt(1, 0x4f);
      typewriter_.printwheelSample(plusPosition, underscorePosition);
      sendGenericResponse(client, PicoRest::HttpResponse::StatusCode::OK);
    }
    else if (request.path == "/query") {
      std::string json;
      typewriter_.queryToJson(json);
      PicoRest::HttpResponse response(PicoRest::HttpResponse::StatusCode::OK);
      client.println(response.header().c_str());
      client.println(json.c_str());
    }
    else if (request.path == "/readLine") {
      ParameterString parameters(request.content);
      int timeout = parameters.getParameterInt(0, 0);
      std::string line;
      typewriter_.readLine(line, timeout);
      PicoRest::HttpResponse response(PicoRest::HttpResponse::StatusCode::OK);
      client.println(response.header().c_str());
      client.println(line.c_str());
    }
    else if (request.path == "/type") {
      typewriter_.readFlush();
      typewriter_.setSpaceForWheel();
      // typewriter_.setKeyboard(keyboard);
      typewriter_.setLeftMargin();

      typewriter_.typeStream.reset();
      typewriter_.typeStream.setUseCaratAsControl(true);

      for (int i = 0; i < request.content.length(); i++) {
        if (!(typewriter_.typeStream << request.content[i])) {
          break;
        }
      }
      sendGenericResponse(client, PicoRest::HttpResponse::StatusCode::OK);
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
