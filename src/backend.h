#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <main_macros.h>

TimerHandle_t wifiReconnectTimer;

void connectToWifi()
{
    Serial.println("Trying to Reconnect");
    WiFi.disconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void handleWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
    //Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event)
    {
    case SYSTEM_EVENT_STA_CONNECTED:
        Serial.println("SYSTEM_EVENT_STA_CONNECTED");
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("SYSTEM_EVENT_STA_GOT_IP");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Gateway IP: ");
        Serial.println(WiFi.gatewayIP());
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.print("SYSTEM_EVENT_STA_DISCONNECTED, Reason: ");
        Serial.println(info.disconnected.reason);
        if (!xTimerIsTimerActive(wifiReconnectTimer))
        {
            xTimerStart(wifiReconnectTimer, 0);
        }
        break;
    }
}

class Backend
{
private:
    String restService = "http://ubuntu-pi4:8081/filter-events/";

public:
    void init();
    void reconnect();
    void createFilterEvent(int startWeight, int endWeight, int duration, int ppm);
    void getFilterEvent(long id);
};

void Backend::init()
{
    Serial.println("### connecting to WiFi..");

    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(10000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    WiFi.mode(WIFI_STA);
    WiFi.onEvent(handleWiFiEvent);
    connectToWifi();
    /*
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    */
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