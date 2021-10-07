//#include <AsyncElegantOTA.h>
#include "Networking.h"

const char* host = "esp32";



AsyncWebServer server(80);

//AsyncWebSocket ws("/ws");
//AsyncEventSource events("/events");
Settings* settings = Settings::getInstance();
Screen* screen = Screen::getInstance();

[[noreturn]] void Networking::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
//    Serial.print("IP address: ");
//    Serial.println(IPAddress(info.ap_staipassigned.ip.addr));
//    Serial.println(WiFi.localIP());

//  UpdaterClass updater;
//  updater.checkForUpdate();


}

[[noreturn]] void Networking::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{

    Serial.print("IP address: ");
    Serial.println(IPAddress(info.ap_staipassigned.ip.addr));

    if (!MDNS.begin(host)) { //http://esp32.local
        Serial.println("Error setting up MDNS responder!");
        while (1) {
            delay(100);
        }
    }

    Serial.print("mDNS responder started, hostname: ");
    Serial.println("http://esp32.local");

    serverSetup();

}

int Networking::connectWiFi(int wait, const char* ssid, const char* pass) {

    if(WiFi.status() == WL_CONNECTED)
        return WiFi.status();

    WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);

    Serial.print("Connecting to WiFi network: \"");
    Serial.print(ssid);
    Serial.println("\"");

    WiFi.begin(ssid, pass);
}

boolean Networking::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}


String processor(const String& var){
//    Serial.println(var);
    char c[10];
    if(var == "offsetX"){
        return itoa(settings->offsetX, c, 10);
    } else if(var == "offsetY") {
        return itoa(settings->offsetY, c, 10);
    } else if(var == "ellipseA") {
        return itoa(settings->ellipseA, c, 10);
    } else if(var == "ellipseB") {
        return itoa(settings->ellipseB, c, 10);
    } else if(var == "needleCenterRadius") {
        return itoa(settings->needleCenterRadius, c, 10);
    } else if(var == "needleCenterOffset") {
        return itoa(settings->needleCenterOffset, c, 10);
    } else if(var == "needleLength") {
        return itoa(settings->needleLength, c, 10);
    } else if(var == "needleBottomWidth") {
        return itoa(settings->needleBottomWidth, c, 10);
    } else if(var == "needleTopWidth") {
        return itoa(settings->needleTopWidth, c, 10);

    } else if(var == "input_table") {

        std::stringstream ss;
        ss << "<table>\n<tr>";
        for(int i=0; i<6; i++) {
            if(i<4)
                ss << "<td>ADS1115_" << i << "</td>";
            else
                ss << "<td>ADC" << i << "</td>";
        }
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>Włącz<input type='checkbox' id='input_" << i << "_en' " << (settings->input[i].enable ? "checked" : "") << "></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>R<input value='" << settings->input[i].r << "' type='number' step='0.1' id='input_" << i << "_rballance'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>Typ<select id='input_" << i << "_type'><option value='0' " << (settings->input[i].type ? "" : "selected") << ">Liniowe</option><option value='1'" << (settings->input[i].type ? "selected" : "")  << ">Log</option></select></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>Beta<input value='" << settings->input[i].beta << "' type='number' step='0.1' id='input_" << i << "_beta'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>R25<input value='" << settings->input[i].r25 << "' type='number' step='0.1' id='input_" << i << "_r25'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>Rmin<input value='" << settings->input[i].rmin << "' type='number' step='0.1' id='input_" << i << "_rmin'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>Rmaks<input value='" << settings->input[i].rmax << "' type='number' step='0.1' id='input_" << i << "_rmax'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>Maks. wartość<input value='" << settings->input[i].maxVal << "' type='number' step='0.1' id='input_" << i << "_max_val'></td>";
        ss << "</tr>\n<table>";
        return String(ss.str().c_str());
    }
    return String("2137");
}

void Networking::serverSetup() {
    TaskHandle_t handle;
    Serial.print("server setup task: ");
    Serial.println(
            xTaskCreatePinnedToCore(serverSetupTask,
                "setupServer",
                16*1024,
                NULL,
                1,
                &handle,
                1) ? "started" : "failed");
}


/* setup function */
void Networking::serverSetupTask(void * pvParameters) {


    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Serial.print(".");
    }


    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/settings.html", String(), false, processor);
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    server.on("/settingsSet", HTTP_POST, [](AsyncWebServerRequest *request){
//        setSettings();
//        while(screen->isBusy) {
//
//        }
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
                Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());

                std::string str = p->name().c_str();

                if(str.find("input") != std::string::npos) {
                    int i = str.at(6) - 48;
                    std::string str2 = str.substr(8);

                    if(str2.compare("en") == 0)
                        settings->input[i].enable = atoi(p->value().c_str());
                    else if(str2.compare("rballance") == 0)
                        settings->input[i].r = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("type") == 0)
                        settings->input[i].type = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("beta") == 0)
                        settings->input[i].beta = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("r25") == 0)
                        settings->input[i].r25 = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("rmin") == 0)
                        settings->input[i].rmin = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("rmax") == 0)
                        settings->input[i].rmax = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("max_val") == 0)
                        settings->input[i].maxVal = strtof(p->value().c_str(), NULL);

                }

                if(p->name()=="offsetX")
                    settings->offsetX = atoi(p->value().c_str());
                else if(p->name()=="offsetY")
                    settings->offsetY = atoi(p->value().c_str());
                else if(p->name()=="ellipseA")
                    settings->ellipseA = atoi(p->value().c_str());
                else if(p->name()=="ellipseB")
                    settings->ellipseB = atoi(p->value().c_str());
                else if(p->name()=="needleCenterRadius")
                    settings->needleCenterRadius = atoi(p->value().c_str());
                else if(p->name()=="needleCenterOffset")
                    settings->needleCenterOffset = atoi(p->value().c_str());
                else if(p->name()=="needleLength")
                    settings->needleLength = atoi(p->value().c_str());
                else if(p->name()=="needleBottomWidth")
                    settings->needleBottomWidth = atoi(p->value().c_str());
                else if(p->name()=="needleTopWidth")
                    settings->needleTopWidth = atoi(p->value().c_str());


            }
        }

        settings->save();
//        screen->reset();
        screen->shallWeReset = true;
//        request->send(SPIFFS, "/settings.html", String(), false, processor);
    });

//  server.on("/settingsSet", HTTP_POST, []() {
//    setSettings();
//  });

//    AsyncElegantOTA.begin(&server);

    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
        settings->loadDefault();
    });

  server.begin();

  Serial.println("Server started");

  vTaskDelete(NULL);
}

