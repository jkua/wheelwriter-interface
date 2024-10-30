// Simple REST API class

#pragma once 

#include <string>
#include <map>

#include <Arduino.h>
#include <WiFiNINA.h>

using namespace std::string_literals;

namespace PicoRest {

class HttpRequest {
public:
  HttpRequest() : numHeaderLines(0), parseState(REQUEST), error(false) {}

  int parseChar(char c);
  void print();

  enum Method {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE
  } method;

  size_t numHeaderLines;
  std::string path;
  std::string host;
  std::string userAgent;
  std::string contentType;
  size_t contentLength;
  std::string content;

  enum ParseState {
    REQUEST,
    HEADER,
    CONTENT,
    DONE
  } parseState;

  bool error;

private:
  int parseLine(const std::string& line);
  int parseRequest(const std::string& line);
  int parseHeader(const std::string& line);

  std::string buffer;
};

class HttpResponse {
public:
  enum StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    LENGTH_REQUIRED = 411,
    CONTENT_TOO_LARGE = 413,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503,
  };

  HttpResponse() : status_(StatusCode::OK) {}
  HttpResponse(StatusCode status) : status_(status) {}

  StatusCode status_;

  static const std::map<StatusCode, std::string> statusStrings;

  std::string statusString() {
    return std::to_string(status_) + " "s + statusStrings.at(status_);
  }

  std::string header() {
    return "HTTP/1.1 "s + statusString() + "\r\n"s + "Content-type:text/html\r\n"s;
  }
  std::string body() {
    return "<h1>"s + statusString() + "</h1>"s;
  }
};

class PicoRestApi {
public:
  PicoRestApi(WiFiServer& server) : server_(server), status_(WL_IDLE_STATUS) {}
  int init();
  int connect(const char* ssid, const char* password);
  int listNetworks();
  void printWifiStatus();
  void printMacAddress();
  int processClient();
  virtual void handleGetRequest(WiFiClient& client, HttpRequest& request);
  virtual void handleHeadRequest(WiFiClient& client, HttpRequest& request);
  virtual void handlePostRequest(WiFiClient& client, HttpRequest& request);
  virtual void handlePutRequest(WiFiClient& client, HttpRequest& request);
  virtual void handleDeleteRequest(WiFiClient& client, HttpRequest& request);
  virtual void sendDefaultWebpage(WiFiClient& client);
  void sendGenericResponse(WiFiClient& client, HttpResponse::StatusCode status);
private:
  WiFiServer& server_;
  int status_;
  byte macAddress_[6];
  IPAddress ipAddress_;
};

} // namespace PicoRest