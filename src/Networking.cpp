//#include <AsyncElegantOTA.h>
#include "Networking.h"

const char* host = "esp32";



AsyncWebServer server(80);

//AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");
Settings* settings = Settings::getInstance();
Screen* screen = Screen::getInstance();

void Networking::sendEvent(const char * event, std::string str) {
    events.send(str.c_str(), event, millis());
}

void Networking::f(std::string str) {
    events.send(str.c_str(), "myevent", millis());
}


[[noreturn]] void Networking::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Log.logf("Connected to %s\n", WiFi.SSID().c_str());
//    Log.log(WiFi.SSID());
//    Log.log("IP address: ");
//    Log.log(IPAddress(info.ap_staipassigned.ip.addr));
//    Log.log(WiFi.localIP());

//  UpdaterClass updater;
//  updater.checkForUpdate();


}

[[noreturn]] void Networking::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Log.logf("IP address: %s\n", IPAddress(info.ap_staipassigned.ip.addr).toString().c_str());
//    Log.log(IPAddress(info.ap_staipassigned.ip.addr).toString());

    if (!MDNS.begin(host)) { //http://esp32.local
        Log.log("Error setting up MDNS responder!");
        while (1) {
            delay(100);
        }
    }

    Log.logf("mDNS responder started, hostname: http://%s.local\n", host);
//    Log.log("http://esp32.local");

    serverSetup();
}

int Networking::connectWiFi(int wait, const char* ssid, const char* pass) {

    if(WiFi.status() == WL_CONNECTED)
        return WiFi.status();

    WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, SYSTEM_EVENT_STA_GOT_IP);

    events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
            Log.logf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
        }
        client->send("hello!",NULL,millis(),1000);
        Log.enable();
    });

    server.addHandler(&events);
    Log.setEvent(f);

    Log.logf("Connecting to WiFi network: \"%s\"\n", ssid);

    WiFi.begin(ssid, pass);
}

boolean Networking::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String processor(const String& var){
//    Log.log(var);
    char c[10];

    if(var == "settings"){

        std::stringstream ss;

        for(int i=0; i<GENERAL_SETTINGS_SIZE; i++) {
            if(settings->general[i]->isConfigurable()) {
                ss << "<p>" << settings->general[i]->getName() << "</p>";
                ss << settings->general[i]->getHTMLInput(i);
            }
        }

        return String(ss.str().c_str());

    } else if(var == "settings_js"){

        std::stringstream ss;

        for(int i=0; i<SETTINGS_SIZE; i++) {
            if(settings->general[i]->isConfigurable()) {
                switch(settings->general[i]->getType()) {
                    case Settings::CHECKBOX: {
                        ss << "data.append('";
                        ss << i;
                        ss << "', document.getElementById('";
                        ss << i;
                        ss << "').checked ? \"1\" : \"0\");\n";
                        break;
                    }
                    default: {
                        ss << "data.append('";
                        ss << i;
                        ss << "', document.getElementById('";
                        ss << i;
                        ss << "').value);\n";
                        break;
                    }
                }

            }
        }

        return String(ss.str().c_str());

    } else if(var == "dataDisplayLength") {
        return itoa(Settings::LAST, c, 10);

    } else if(var == "INPUT_0") {
        return itoa(INPUT_0, c, 10);

    } else if(var == "INPUT_SETTINGS_SIZE") {
        return itoa(INPUT_SETTINGS_SIZE, c, 10);

    } else if(var == "INPUT_SIZE") {
        return itoa(INPUT_SIZE, c, 10);

    } else if(var == "DATA_0") {
        return itoa(DATA_0, c, 10);

    } else if(var == "DATA_SETTINGS_SIZE") {
        return itoa(DATA_SETTINGS_SIZE, c, 10);

    } else if(var == "DATA_SIZE") {
        return itoa(DATA_SIZE, c, 10);

    } else if(var == "input_table") {

        std::stringstream ss;
        ss << "<table>\n<tr>";
        for(int i=0; i<INPUT_SIZE; i++) {
            if(i<4)
                ss << "<td>ADS1115_" << i << "</td>";
            else
                ss << "<td>ADC" << i << "</td>";
        }
        ss << "</tr>\n<tr>";

        for(int i=0; i<INPUT_SIZE; i++)
            ss << "<td>Preset<select id='input_" << i << "_preset' onchange='return preset(" << i <<");' type='checkbox' ><option value='-1'>Puste</option><option value='0'>Ciśń. oleju</option><option value='1'>Temp. oleju</option></section></td>";
        ss << "</tr>\n";

        for(int i=0; i<INPUT_SETTINGS_SIZE; i++) {
            ss << "<tr>";
            for(int j=0; j<INPUT_SIZE; j++) {
                int ind = INPUT_0 + INPUT_SETTINGS_SIZE * j + i;
                if(settings->general[ind]->isConfigurable())
                    ss << "<td>" << settings->general[ind]->getName() << settings->general[ind]->getHTMLInput(ind) << "</td>";
            }
            ss << "</tr>\n";
        }

        ss << "</table>";

        return String(ss.str().c_str());

    } else if(var == "data_list") {

        std::stringstream ss;

        ss << "<table>\n<tr>";
        for(int i=0; i<DATA_SIZE; i++)
            ss << "<td>" << settings->dataSourceString[i].c_str() << "</td>";
        ss << "</tr>\n<tr>";

        for(int i=0; i<DATA_SETTINGS_SIZE; i++) {
            ss << "<tr>";
            for(int j=0; j<DATA_SIZE; j++) {
                int ind = DATA_0 + DATA_SETTINGS_SIZE * j + i;
                if(settings->general[ind]->isConfigurable())
                    ss << "<td>" << settings->general[ind]->getName() << settings->general[ind]->getHTMLInput(ind) << "</td>";
            }
            ss << "</tr>\n";
        }

        ss << "</table>";

        return String(ss.str().c_str());
    }

    return String("2137");
}

void Networking::serverSetup() {
    TaskHandle_t handle;
    if(!xTaskCreatePinnedToCore(serverSetupTask,
                "setupServer",
                16*1024,
                NULL,
                1,
                &handle,
                1)) Log.log("Failed to start setupServer task");
}


/* setup function */
void Networking::serverSetupTask(void * pvParameters) {


    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
        Log.log(".");
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
                Log.logf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());

                for(int i=0; i<SETTINGS_SIZE; i++) {
                    if((String)i == p->name().c_str()) {
                        switch (settings->general[i]->getType()) {
                            case Settings::Type::STRING: {
                                settings->general[i]->set(p->value().c_str()); break;
                            }
                            default: {
                                settings->general[i]->set(strtof(p->value().c_str(), nullptr)); break;
                            }
                        }
                    }

                }

            }
        }

        settings->save();
//        screen->reset();
        screen->reset();
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

  Log.log("Server started");

  vTaskDelete(NULL);
}

