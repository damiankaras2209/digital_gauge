#include "Networking.h"

#include <utility>

NetworkingClass Networking;


void NetworkingClass::sendEvent(const char * event, std::string str) {
    if(events != nullptr)
        if(events->count() > 0)
            events->send(str.c_str(), event, millis());
}

int NetworkingClass::connectWiFi(const char* ssid, const char* pass) {

    if (WiFi.status() == WL_CONNECTED)
        return WiFi.status();


    _credentials.ssid = ssid;
    _credentials.pass = pass;

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        Log.logf("Connected to %s, IP: %s\n", WiFi.SSID().c_str(),IPAddress(info.ap_staipassigned.ip.addr).toString().c_str());

        Log.logf("Connected; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

//        if (!MDNS.begin(HOSTNAME)) { //http://esp32.local
//            Log.log("Error setting up MDNS responder!");
//        }
//        Log.logf("mDNS responder started, hostname: http://%s.local\n", HOSTNAME);

//        Log.logf("MDNS; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

        sendInfo();
        serverSetup();
        Log.logf("Info; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());
    }, SYSTEM_EVENT_STA_GOT_IP);

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



void NetworkingClass::serverSetup() {
    _serverOn = true;
    server = new AsyncWebServer(80);
    events = new AsyncEventSource("/events");


    Log.logf("Objects created; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

//    events->onConnect([](AsyncEventSourceClient *client) {
//        Log.log("Client connected");
//        Log.logf("Connected clients: %d", (int)events->count());
//        Log.setCountClients([](){return events->count();});
//        Log.onConnect();
//        Log.enable();
//    });

    server->addHandler(events);
//    Log.setEvent([this](std::string str){ sendEvent( "log", std::move(str));});
    Data.setCountClients([this]() { return events->count(); });
    Data.setEvent([this](const char *event, std::string str) { sendEvent(event, std::move(str)); });

    server->onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS)
            request->send(HTTP_CODE_OK);
        else
            request->send(HTTP_CODE_NOT_FOUND);
    });

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/settings.html", "text/html", false, processor);
        Log.log("Request handled");
    });

    server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/log.html", "text/html", false, nullptr);
    });

    server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    server->on("/settingsSet", HTTP_POST, [](AsyncWebServerRequest *request){
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

    server->on("/time", HTTP_POST, [](AsyncWebServerRequest *request){
        Data.adjustTime(&Data.data);
        request->send(HTTP_CODE_OK);
    });

    server->on("/reset", HTTP_POST, [](AsyncWebServerRequest *request){
        Settings.loadDefault();
        Settings.save();
        Screen.reloadSettings();
        request->send(HTTP_CODE_OK);
    });

    server->begin();

    Log.log("Server started");
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
//    http.end();
//    if (httpResponseCode>0)
//        Serial.println(http.getString());
}

String NetworkingClass::processor(const String& var) {

    char c[10];

    String str;

    if(var == "settings"){

        for(int i=0; i<GENERAL_SETTINGS_SIZE; i++) {
            if(Settings.general[i]->isConfigurable()) {
                str += Settings.general[i]->getHTMLInput(i).c_str();
                str += "\n";
            }
        }
        return str;

    } else if(var == "settings_js"){

        for(int i=0; i<SETTINGS_SIZE; i++) {
            if(Settings.general[i]->isConfigurable()) {
                switch(Settings.general[i]->getType()) {
                    case SettingsClass::CHECKBOX: {
                        str += "data.append('";
                        str += i;
                        str += "', document.getElementById('";
                        str += i;
                        str += "').checked ? \"1\" : \"0\");\n";
                        break;
                    }
                    default: {
                        str += "data.append('";
                        str += i;
                        str += "', document.getElementById('";
                        str += i;
                        str += "').value);\n";
                        break;
                    }
                }

            }
        }

        return str;

    } else if(var == "CONSTS") {

        str += (String)"const MAC = '" + Updater.getMac().c_str() + "';\n";
        str += (String)"const GENERAL_SETTINGS_SIZE = " + GENERAL_SETTINGS_SIZE + ";\n";
        str += (String)"const INPUT_SETTINGS_SIZE = " + INPUT_SETTINGS_SIZE + ";\n";
        str += (String)"const INPUT_SIZE = " + INPUT_SIZE + ";\n";
        str += (String)"const INPUT_BEGIN_BEGIN = " + INPUT_BEGIN_BEGIN + ";\n";
        str += (String)"const DATA_SETTINGS_SIZE = " + DATA_SETTINGS_SIZE + ";\n";
        str += (String)"const DATA_SIZE = " + DATA_SIZE + ";\n";
        str += (String)"const DATA_BEGIN_BEGIN = " + DATA_BEGIN_BEGIN + ";\n";
        str += (String)"const SETTINGS_SIZE = " + SETTINGS_SIZE + ";\n";

        return str;

    } else if(var == "input_table") {

        str += "<table>\n";

        for(int i=0; i<INPUT_SIZE; i++) {
            if(i<4)
                str += (String) "<tr clastr='input.ADS1115_" + i + "'><td>ADS1115_" + i + "</td>";
            else
                str += (String) "<tr clastr='input.ADC" + i + "'><td>ADC" + i + "</td>";

            str += (String) "<td>Preset <select id='input_" + i + "_preset' onchange='return preset(" + i +");' type='checkbox' ><option value='-1'>Puste</option><option value='0'>Ciśń. oleju</option><option value='1'>Temp. oleju</option></section></td>";

            for(int j=0; j<INPUT_SETTINGS_SIZE; j++) {
                int ind = INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + j;
                if(Settings.general[ind]->isConfigurable())
                    str += (String) "<td>" + Settings.general[ind]->getHTMLInput(ind).c_str() + "</td>";
            }

            str +=  (String)"<td id='voltage_" + i + "'>Voltage: </td>";
            str +=  (String) + "<td id='resistance_" + i + "'>Resistance: </td>";
            str +=  (String) + "<td id='value_" + i + "'>Value: </td>";

            str += "</tr>\n";
        }
        str += "</table>\n";

        return str;

    } else if(var == "data_list") {

        str += "<table>\n<tr>";
        for(int i=0; i<DATA_SIZE; i++) {
            str += "<td>";
            str += Settings.dataSourceString[i].c_str();
            str += "</td>";
        }
        str += "</tr>\n<tr>";

        for(int i=0; i<DATA_SETTINGS_SIZE; i++) {
            str += "<tr>";
            for(int j=0; j<DATA_SIZE; j++) {
                int ind = DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * j + i;
                if(Settings.general[ind]->isConfigurable()) {
                    str += "<td>";
                    str += Settings.general[ind]->getHTMLInput(ind).c_str();
                    str += "</td>";
                }
            }
            str += "</tr>\n";
        }

        str += "</table>";

        return str;
    }

}
