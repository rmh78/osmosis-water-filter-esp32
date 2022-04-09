#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <main_macros.h>

class Backend
{
private:
    String restService = "http://ubuntu-pi4:8081/filter-events/";
public:
    bool connectWiFi();
    void disconnectWiFi();
    void createFilterEvent(int startWeight, int endWeight, int duration, int ppm);
    void getFilterEvent(long id);
};

bool Backend::connectWiFi()
{
    Serial.println("### connecting to WiFi..");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    for (int i=0; i<10 && !WiFi.isConnected(); i++)
    {
        delay(1000);
        Serial.println("connecting...");
    }

    if (WiFi.isConnected())
    {
        Serial.println("connected to WiFi");
        Serial.print("ip: ");
        Serial.println(WiFi.localIP());
        Serial.print("gateway ip: ");
        Serial.println(WiFi.gatewayIP());
        return true;
    }

    Serial.println("NOT connected to WiFi");
    return false;
}

void Backend::disconnectWiFi()
{
    Serial.println("### disconnecting from WiFi..");
    WiFi.disconnect();
}

void Backend::createFilterEvent(int startWeight, int endWeight, int duration, int ppm)
{
    Serial.println("### create filter event");

    HTTPClient http;
    http.begin(restService);
    http.addHeader("Content-Type", "application/json");

    // prepare JSON
    DynamicJsonDocument doc(2048);
    doc["startWeight"] = startWeight;
    doc["endWeight"] = endWeight;
    doc["duration"] = duration;
    doc["ppm"] = ppm;

    // serialize JSON
    String json;
    serializeJson(doc, json);
    Serial.println(json);

    int httpCode = http.POST(json);
    Serial.print("http response code: ");
    Serial.println(httpCode);
    if (httpCode != 201)
    {
        Serial.println("error on HTTP request");
    }
    http.end();
}

void Backend::getFilterEvent(long id)
{
    Serial.println("### get filter event for id: " + (String)id);

    HTTPClient http;
    http.begin(restService + (String)id);
    int httpCode = http.GET();

    Serial.print("http response code: ");
    Serial.println(httpCode);

    if (httpCode == 200)
    {
        String payload = http.getString();
        Serial.print("payload: ");
        Serial.println(payload);

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print("deserialize json failed: ");
            Serial.println(error.f_str());
        }
        else
        {
            long id = doc["id"];
            int startWeight = doc["startWeight"];
            int endWeight = doc["endWeight"];
            int duration = doc["duration"];
            int ppm = doc["ppm"];
        }
    }
    else
    {
        Serial.println("error on HTTP request");
    }

    http.end();
}