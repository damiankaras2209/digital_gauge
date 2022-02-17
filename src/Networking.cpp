#include "Networking.h"

const char* host = "esp32";

NetworkingClass Networking;

AsyncWebServer server(80);

//AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void NetworkingClass::sendEvent(const char * event, std::string str) {
    events.send(str.c_str(), event, millis());
}

void NetworkingClass::f(std::string str) {
    events.send(str.c_str(), "myevent", millis());
}

[[noreturn]] void NetworkingClass::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{

    Log.logf("Connected to %s,IP: %s\n ", WiFi.SSID().c_str(),IPAddress(info.ap_staipassigned.ip.addr).toString().c_str());

    if (!MDNS.begin(host)) { //http://esp32.local
        Log.log("Error setting up MDNS responder!");
        while (1) {
            delay(100);
        }
    }

    Log.logf("mDNS responder started, hostname: http://%s.local\n", host);

    Data.adjustTime(&Data.data);
    serverSetup();
}

int NetworkingClass::connectWiFi(int wait, const char* ssid, const char* pass) {

    if(WiFi.status() == WL_CONNECTED)
        return WiFi.status();

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

String processor(const String& var){
    char c[10];

    if(var == "settings"){

        std::stringstream ss;

        for(int i=0; i<GENERAL_SETTINGS_SIZE; i++) {
            if(Settings.general[i]->isConfigurable()) {
                ss << Settings.general[i]->getHTMLInput(i);
            }
        }

        return String(ss.str().c_str());

    } else if(var == "settings_js"){

        std::stringstream ss;

        for(int i=0; i<SETTINGS_SIZE; i++) {
            if(Settings.general[i]->isConfigurable()) {
                switch(Settings.general[i]->getType()) {
                    case SettingsClass::CHECKBOX: {
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

    } else if(var == "CONSTS") {

        std::stringstream ss;

        ss << "const GENERAL_SETTINGS_SIZE = " << GENERAL_SETTINGS_SIZE << ";";
        ss << "const INPUT_SETTINGS_SIZE = " << INPUT_SETTINGS_SIZE << ";";
        ss << "const INPUT_SIZE = " << INPUT_SIZE << ";";
        ss << "const INPUT_BEGIN_BEGIN = " << INPUT_BEGIN_BEGIN << ";";
        ss << "const DATA_SETTINGS_SIZE = " << DATA_SETTINGS_SIZE << ";";
        ss << "const DATA_SIZE = " << DATA_SIZE << ";";
        ss << "const DATA_BEGIN_BEGIN = " << DATA_BEGIN_BEGIN << ";";
        ss << "const SETTINGS_SIZE = " << SETTINGS_SIZE << ";";

        return String(ss.str().c_str());

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
                int ind = INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * j + i;
                if(Settings.general[ind]->isConfigurable())
                    ss << "<td>" << Settings.general[ind]->getHTMLInput(ind) << "</td>";
            }
            ss << "</tr>\n";
        }

        ss << "</table>";

        return String(ss.str().c_str());

    } else if(var == "data_list") {

        std::stringstream ss;

        ss << "<table>\n<tr>";
        for(int i=0; i<DATA_SIZE; i++)
            ss << "<td>" << Settings.dataSourceString[i].c_str() << "</td>";
        ss << "</tr>\n<tr>";

        for(int i=0; i<DATA_SETTINGS_SIZE; i++) {
            ss << "<tr>";
            for(int j=0; j<DATA_SIZE; j++) {
                int ind = DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * j + i;
                if(Settings.general[ind]->isConfigurable())
                    ss << "<td>" << Settings.general[ind]->getHTMLInput(ind) << "</td>";
            }
            ss << "</tr>\n";
        }

        ss << "</table>";

        return String(ss.str().c_str());
    }

    return String("2137");
}

void NetworkingClass::serverSetup() {
    TaskHandle_t handle;
    if(!xTaskCreatePinnedToCore(serverSetupTask,
                "setupServer",
                16*1024,
                NULL,
                1,
                &handle,
                1)) Log.log("Failed to start setupServer task");
}

void NetworkingClass::serverSetupTask(void * pvParameters) {


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
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
                Log.logf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());

                for(int i=0; i<SETTINGS_SIZE; i++) {
                    if((String)i == p->name().c_str()) {
                        switch (Settings.general[i]->getType()) {
                            case SettingsClass::Type::STRING: {
                                Settings.general[i]->set(p->value().c_str()); break;
                            }
                            case SettingsClass::Type::COLOR: {
                                String str = p->value().substring(1);

                                char *p;
                                long color888 = std::strtol(str.c_str(), &p, 16);
                                uint16_t r = (color888 >> 8) & 0xF800;
                                uint16_t g = (color888 >> 5) & 0x07E0;
                                uint16_t b = (color888 >> 3) & 0x001F;

                                Settings.general[i]->set((r | g | b));
                                break;
                            }
                            default: {
                                Settings.general[i]->set(strtof(p->value().c_str(), nullptr)); break;
                            }
                        }
                    }

                }

            }
        }

        Settings.save();
        Screen.reset();
    });

    server.on("/time", HTTP_POST, [](AsyncWebServerRequest *request){
        Data.adjustTime(&Data.data);
    });

    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
        Settings.loadDefault();
        Settings.save();
        Screen.reset();
    });

  server.begin();

  Log.log("Server started");

  vTaskDelete(NULL);
}

