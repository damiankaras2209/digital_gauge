#include "Networking.h"

#include <utility>

NetworkingClass Networking;

AsyncWebServer server(80);
AsyncEventSource events("/events");
//AsyncWebSocket ws("/ws");

void NetworkingClass::sendEvent(const char * event, std::string str) {
    if(events.count() > 0)
        events.send(str.c_str(), event, millis());
}

int NetworkingClass::connectWiFi(const char* ssid, const char* pass) {

    if (WiFi.status() == WL_CONNECTED)
        return WiFi.status();
    events.onConnect([](AsyncEventSourceClient *client) {
//        Log.log("Client connected");
//        Log.logf("Connected clients: %d", (int)events.count());
//        Log.setCountClients([](){return events.count();});
//        Log.onConnect();
//        Log.enable();
    });

    server.addHandler(&events);
//    Log.setEvent([this](std::string str){ sendEvent( "log", std::move(str));});
    Data.setCountClients([]() { return events.count(); });
    Data.setEvent([this](const char *event, std::string str) { sendEvent(event, std::move(str)); });


    _credentials.ssid = ssid;
    _credentials.pass = pass;

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_STA);
    delay(2000);
    WiFi.begin(_credentials.ssid, _credentials.pass);

    TaskHandle_t handle;
    if (!xTaskCreate(connectionMaintainer,
                     "connectionMaintainer",
                     4 * 1024,
                     &_credentials,
                     1,
                     &handle))
        Log.log("Failed to start connectionMaintainer task");


    Log.logf("Connecting to WiFi network: \"%s\"\n", ssid);
}

_Noreturn void NetworkingClass::connectionMaintainer(void * pvParameters) {
    for(;;) {
        while(WiFi.status() != WL_CONNECTED) {
            WiFi.reconnect();
//            Log.logf("Result: %d\n", WiFi.waitForConnectResult());
            delay(5000);
        }
        delay(1000);
    }
}

String processor(const String& var){
    char c[10];

    if(var == "settings"){

        std::stringstream ss;

        for(int i=0; i<GENERAL_SETTINGS_SIZE; i++) {
            if(Settings.general[i]->isConfigurable()) {
                ss << Settings.general[i]->getHTMLInput(i) << "\n";
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

        ss << "const MAC = '" << Updater.getMac().c_str() << "';\n";
        ss << "const GENERAL_SETTINGS_SIZE = " << GENERAL_SETTINGS_SIZE << ";\n";
        ss << "const INPUT_SETTINGS_SIZE = " << INPUT_SETTINGS_SIZE << ";\n";
        ss << "const INPUT_SIZE = " << INPUT_SIZE << ";\n";
        ss << "const INPUT_BEGIN_BEGIN = " << INPUT_BEGIN_BEGIN << ";\n";
        ss << "const DATA_SETTINGS_SIZE = " << DATA_SETTINGS_SIZE << ";\n";
        ss << "const DATA_SIZE = " << DATA_SIZE << ";\n";
        ss << "const DATA_BEGIN_BEGIN = " << DATA_BEGIN_BEGIN << ";\n";
        ss << "const SETTINGS_SIZE = " << SETTINGS_SIZE << ";\n";

        return String(ss.str().c_str());

    } else if(var == "input_table") {

        std::stringstream ss;
        ss << "<table>\n";

        for(int i=0; i<INPUT_SIZE; i++) {
            if(i<4)
                ss << "<tr class='input.ADS1115_" << i << "'><td>ADS1115_" << i << "</td>";
            else
                ss << "<tr class='input.ADC" << i << "'><td>ADC" << i << "</td>";

            ss << "<td>Preset <select id='input_" << i << "_preset' onchange='return preset(" << i <<");' type='checkbox' ><option value='-1'>Puste</option><option value='0'>Ciśń. oleju</option><option value='1'>Temp. oleju</option></section></td>";

            for(int j=0; j<INPUT_SETTINGS_SIZE; j++) {
                int ind = INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + j;
                if(Settings.general[ind]->isConfigurable())
                    ss << "<td>" << Settings.general[ind]->getHTMLInput(ind) << "</td>";
            }

            ss << "<td id='voltage_" << i << "'>Voltage: </td>";
            ss << "<td id='resistance_" << i << "'>Resistance: </td>";
            ss << "<td id='value_" << i << "'>Value: </td>";

            ss << "</tr>\n";
        }
        ss << "</table>\n";

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

    server.onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS)
            request->send(200);
        else
            request->send(404);
    });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/settings.html", String(), false, processor);
        Log.log("Request handled");
    });

    server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/log.html", String(), false, nullptr);
    });

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    server.on("/settingsSet", HTTP_POST, [](AsyncWebServerRequest *request){
        int params = request->params();
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
//                Log.logf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());

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
        Screen.reloadSettings();
        request->send(HTTP_CODE_OK);
    });

    server.on("/time", HTTP_POST, [](AsyncWebServerRequest *request){
        Data.adjustTime(&Data.data);
        request->send(HTTP_CODE_OK);
    });

    server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
        Settings.loadDefault();
        Settings.save();
        Screen.reloadSettings();
        request->send(HTTP_CODE_OK);
    });

  server.begin();

  Log.log("Server started");

  vTaskDelete(NULL);
}

void NetworkingClass::sendInfo() {
    std::stringstream url;
    url << URL << "info.php";
    HTTPClient http;
    http.begin(url.str().c_str());
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    std::stringstream data;
    data << "mac=" << Updater.getMac().c_str() << "&version=" << Updater.firmware.toString().c_str();
//    Log.logf("POST data: %s\n", data.str().c_str());
    int httpResponseCode = http.POST((String)data.str().c_str());
    Log.logf("Info HTTP Response code: %d\n", httpResponseCode);
//    if (httpResponseCode>0)
//        Serial.println(http.getString());
}

