// Tic-tac-toe served from an ESP32 hotspot.
// Join the WiFi, the game opens by itself, then ask somebody in the lobby to play.

#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "game.h"
#include "page.h"

const char *WIFI_NAME = "Play Tic-Tac-Toe";
const IPAddress PORTAL_IP(4, 3, 2, 1);  // easy to type if the game doesn't pop up
const int MAX_DEVICES = 8;              // up to four games at once

AsyncWebServer server(80);
DNSServer dns;
TicTacToe game;  // only touched from web server callbacks, which share one task
String portalHost, portalUrl;

uint32_t deviceId(AsyncWebServerRequest *request) {
  return (uint32_t)request->client()->remoteIP();
}

int intParam(AsyncWebServerRequest *request, const char *name) {
  return request->hasParam(name) ? request->getParam(name)->value().toInt() : -1;
}

void sendState(AsyncWebServerRequest *request) {
  int watch = intParam(request, "watch");
  char json[1280];
  game.toJson(deviceId(request), watch > 0 ? watch : 0, millis(), json, sizeof json);
  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

void sendToPortal(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(302);
  response->addHeader("Location", portalUrl.c_str());
  response->addHeader("Cache-Control", "no-store");
  request->send(response);
}

// Something a phone can ask for. It always gets the resulting state back.
void onAction(const char *path, ArRequestHandlerFunction action) {
  server.on(path, HTTP_POST, [action](AsyncWebServerRequest *request) {
    game.touch(deviceId(request), millis());
    action(request);
    sendState(request);
  });
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(PORTAL_IP, PORTAL_IP, IPAddress(255, 255, 255, 0));
  if (!WiFi.softAP(WIFI_NAME, NULL, 1, 0, MAX_DEVICES)) {
    Serial.println("Could not start the WiFi hotspot. Restarting...");
    delay(2000);
    ESP.restart();
  }
  portalHost = WiFi.softAPIP().toString();
  portalUrl = "http://" + portalHost + "/";

  // Every domain name leads here, which is what makes phones open the game.
  dns.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->host() != portalHost) return sendToPortal(request);
    AsyncWebServerResponse *response =
        request->beginResponse(200, "text/html", (const uint8_t *)PAGE, sizeof(PAGE) - 1);
    response->addHeader("Cache-Control", "no-store");
    request->send(response);
  });

  server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest *request) {
    game.touch(deviceId(request), millis());
    sendState(request);
  });
  onAction("/api/join", [](AsyncWebServerRequest *request) {
    game.join(deviceId(request), intParam(request, "avatar"), millis());
  });
  onAction("/api/leave", [](AsyncWebServerRequest *request) {
    game.leave(deviceId(request), millis());
  });
  onAction("/api/invite", [](AsyncWebServerRequest *request) {
    game.invite(deviceId(request), intParam(request, "player"));
  });
  onAction("/api/cancel", [](AsyncWebServerRequest *request) {
    game.cancelInvite(deviceId(request));
  });
  onAction("/api/decline", [](AsyncWebServerRequest *request) {
    game.decline(deviceId(request), intParam(request, "player"));
  });
  onAction("/api/move", [](AsyncWebServerRequest *request) {
    game.move(deviceId(request), intParam(request, "cell"), esp_random());
  });
  onAction("/api/rematch", [](AsyncWebServerRequest *request) {
    game.rematch(deviceId(request));
  });
  onAction("/api/taunt", [](AsyncWebServerRequest *request) {
    game.taunt(deviceId(request), intParam(request, "id"), millis());
  });
  onAction("/api/quit", [](AsyncWebServerRequest *request) {
    game.quit(deviceId(request));
  });

  // Includes the URLs phones and laptops use to check for a sign-in page.
  server.onNotFound(sendToPortal);
  server.begin();

  Serial.printf("Join the WiFi \"%s\". Game: %s\n", WIFI_NAME, portalUrl.c_str());
}

void loop() {
  dns.processNextRequest();
  delay(5);
}
