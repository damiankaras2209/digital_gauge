#include "Networking.h"

#include <utility>

NetworkingClass Networking;


void NetworkingClass::sendEvent(const char * event, const std::string& content, const ulong id = 0) const {
    if(events != nullptr)
        if(events->count() > 0)
            events->send(content.c_str(), event, id == 0 ? millis() : id);
}

int NetworkingClass::connectWiFi(const char* ssid, const char* pass) {

    if (WiFi.status() == WL_CONNECTED)
        return WiFi.status();


    _credentials.ssid = ssid;
    _credentials.pass = pass;

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        Log.logf("Connected to %s, IP: %s\n", WiFi.SSID().c_str(),IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());

        Log.logf("Connected; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

//        if (!MDNS.begin(HOSTNAME)) { //http://esp32.local
//            Log.logf("Error setting up MDNS responder!");
//        }
//        Log.logf("mDNS responder started, hostname: http://%s.local\n", HOSTNAME);

//        Log.logf("MDNS; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

        serverSetup();
        Log.logf("Info; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

        Log.enable();

        //wait for rtc initialization
        while(Data.getTime().year() == 2000)
            delay(100);

        if(Data.getTime().year() >= 2099)
            Data.adjustTime(&Data.data);

    }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_STA);
    // delay(2000);
    WiFi.begin(_credentials.ssid, _credentials.pass);

    TaskHandle_t handle;
    if (!xTaskCreate(connectionMaintainer,
                     "connectionMaintainer",
                     4 * 1024,
                     &_credentials,
                     1,
                     &handle))
        Log.logf("Failed to start connectionMaintainer task");


    Log.logf("Connecting to WiFi network: \"%s\"\n", ssid);
    return 0;
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
//        Log.logf("Client connected");
//        Log.logf("Connected clients: %d", (int)events->count());
//        Log.setCountClients([](){return events->count();});
//        Log.onConnect();
//        Log.enable();
//    });

    server->addHandler(events);
//    Log.setEvent([this](std::string str){ sendEvent( "log", std::move(str));});
    Log.setCountClients([this]() { return events->count(); });
    Log.setEvent([this](const std::string& str, const ulong id) { sendEvent("log", str, id); });
    Data.setCountClients([this]() { return events->count(); });
    Data.setEvent([this](const char *event, const std::string& str) { sendEvent(event, str); });

    server->onNotFound([](AsyncWebServerRequest *request) {
        if (request->method() == HTTP_OPTIONS)
            request->send(HTTP_CODE_OK);
        else
            request->send(HTTP_CODE_NOT_FOUND);
    });

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/settings.html", "text/html", false, processorRoot);
    });

    server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/log.html", "text/html", false, processorLog);
    });

    server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    server->on("/settingsSet", HTTP_POST, [](AsyncWebServerRequest *request){
        int params = request->params();
        bool resetScreen = false;
        Log.logf("%d, %d\n", VISUAL_SETTINGS_START, VISUAL_SETTINGS_END);
        for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){

                int ind = (int)strtof(p->name().c_str(), nullptr);

                Log.logf("POST[%d, %s]: %s\n", ind, Settings.general[ind]->getId().c_str(), p->value().c_str());

                if(ind >= VISUAL_SETTINGS_START && ind <= VISUAL_SETTINGS_END)
                    resetScreen = true;

                switch (Settings.general[ind]->getType()) {
                    case SettingsClass::Type::STRING: {
                        Settings.general[ind]->set(p->value().c_str()); break;
                    }
                    case SettingsClass::Type::COLOR: {
                        String str = p->value().substring(1);

                        char *p;
                        long color888 = std::strtol(str.c_str(), &p, 16);
                        uint16_t r = (color888 >> 8) & 0xF800;
                        uint16_t g = (color888 >> 5) & 0x07E0;
                        uint16_t b = (color888 >> 3) & 0x001F;

                        Settings.general[ind]->set((r | g | b));
                        break;
                    }
                    default: {
                        Settings.general[ind]->set(strtof(p->value().c_str(), nullptr)); break;
                    }
                }
            }
        }

        if(resetScreen) {
            Log.logf("Resetting screen");
            Screen.reloadSettings();
        }
        S_STATUS status = Settings.save();
        request->send(HTTP_CODE_OK, "plain/text", status == S_SUCCESS ? "success" : "fail");
    });

    server->on("/time", HTTP_POST, [](AsyncWebServerRequest *request){
        Data.adjustTime(&Data.data);
        request->send(HTTP_CODE_OK);
    });

    server->on("/reset_all", HTTP_POST, [](AsyncWebServerRequest *request){
        Settings.loadDefault();
        Screen.reloadSettings();
        Settings.save();
        request->send(HTTP_CODE_OK);
    });

    server->on("/reset_visual", HTTP_POST, [](AsyncWebServerRequest *request){
        for(int i=VISUAL_SETTINGS_START; i<=VISUAL_SETTINGS_END; i++)
            Settings.general[i]->setDefault();
        Screen.reloadSettings();
        Settings.save();
        request->send(HTTP_CODE_OK);
    });

    server->on("/reset_input", HTTP_POST, [](AsyncWebServerRequest *request){
        for(int i=INPUT_BEGIN_BEGIN; i<=INPUT_END_END; i++)
            Settings.general[i]->setDefault();
        Settings.save();
        request->send(HTTP_CODE_OK);
    });

    server->on("/reset_data", HTTP_POST, [](AsyncWebServerRequest *request){
        for(int i=DATA_BEGIN_BEGIN; i<=DATA_END_END; i++)
            Settings.general[i]->setDefault();
        Settings.save();
        request->send(HTTP_CODE_OK);
    });

    server->on("/ota", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/ota.html", "text/html", false, processorOta);
    });

    server->on("/ota", HTTP_POST, [](AsyncWebServerRequest *request){
        Log.logf("OTA");
        int params = request->params();
        if (params == 1) {
            AsyncWebParameter* p = request->getParam(0);
            Updater.setOnSuccessCallback([] {
                Log.logf("Restarting in 1 seconds");
                Screen.tick();
                delay(1000);
                Screen.setBrightness(0);
                esp_restart();
            });
            Updater.updateFW(String(URL) + "files/" + p->value().c_str());
        }
        request->send(HTTP_CODE_OK);
    });

    server->begin();

    Log.logf("Server started");
}

String NetworkingClass::processorRoot(const String& var) {

    char c[10];

    String str;

    if(var == "general"){

        for(int i=0; i<VISUAL_SETTINGS_START; i++) {
            if(Settings.general[i]->isConfigurable()) {
                str += Settings.general[i]->getHTMLInput(i).c_str();
                str += "\n";
            }
        }
        return str;

    } else if(var == "visual"){

        for(int i=VISUAL_SETTINGS_START; i<GENERAL_SETTINGS_SIZE; i++) {
            if(Settings.general[i]->isConfigurable()) {
                str += Settings.general[i]->getHTMLInput(i).c_str();
                str += "\n";
            }
        }
        return str;

    } else if(var == "CONSTS") {

        str += (String)"const GENERAL_SETTINGS_SIZE = " + GENERAL_SETTINGS_SIZE + ";\n";
        str += (String)"const VISUAL_SETTINGS_START = " + VISUAL_SETTINGS_START + ";\n";
        str += (String)"const VISUAL_SETTINGS_END = " + VISUAL_SETTINGS_END + ";\n";
        str += (String)"const INPUT_SETTINGS_SIZE = " + INPUT_SETTINGS_SIZE + ";\n";
        str += (String)"const INPUT_SIZE = " + INPUT_SIZE + ";\n";
        str += (String)"const INPUT_BEGIN_BEGIN = " + INPUT_BEGIN_BEGIN + ";\n";
        str += (String)"const DATA_SETTINGS_SIZE = " + DATA_SETTINGS_SIZE + ";\n";
        str += (String)"const DATA_SIZE = " + DATA_SIZE + ";\n";
        str += (String)"const DATA_BEGIN_BEGIN = " + DATA_BEGIN_BEGIN + ";\n";
        str += (String)"const SETTINGS_SIZE = " + SETTINGS_SIZE + ";\n";

        str += "let configurable = [";

        for(int i=0; i<SETTINGS_SIZE; i++)
            str += (String) (Settings.general[i]->isConfigurable() ? "1," : "0,");

        str.remove(str.length()-1);
        str += "];";

        return str;

    } else if(var == "input_table") {

        str += "<table>\n";

        for(int i=0; i<INPUT_SIZE; i++) {
            if(i<4)
                str += (String) "<tr clastr='input.ADS1115_" + i + "'><td>ADS1115_" + i + "</td>";
            else
                str += (String) "<tr clastr='input.ADC" + i + "'><td>VOLTAGE</td>";

            str += (String) "<td>Preset <select id='input_" + i + "_preset' onchange='return preset(" + i +");' type='checkbox' ><option value='0'>Puste</option><option value='1'>Ciśń. oleju</option><option value='2'>Temp. oleju</option></section></td>";

            for(int j=0; j<INPUT_SETTINGS_SIZE; j++) {
                int ind = INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + j;
                if(Settings.general[ind]->isConfigurable())
                    str += (String) "<td>" + Settings.general[ind]->getHTMLInput(ind).c_str() + "</td>";
            }

            str +=  (String)"<td id='voltage_" + i + "'>Napięcie: </td>";
            str +=  (String) + "<td id='resistance_" + i + "'>Obliczona oporność: </td>";
            str +=  (String) + "<td id='value_" + i + "'>Obliczona wartość: </td>";

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
    return "missing preprocessor case";
}

String NetworkingClass::processorLog(const String& var) {

    char c[10];

    String str;

    if(var == "logs"){

        {
            std::lock_guard<std::mutex> lock(*Log.getGuard());
            auto messages = Log.getMessages();
            for(auto & message : *messages) {
                str += "[";
                str += message.first;
                str += "] ";;
                str += message.second.c_str();
                str += "<br>";
            }
            Log.setSent(messages->size());
        }

        return str;

    }
    return "missing preprocessor case";
}

String NetworkingClass::processorOta(const String& var) {

    char c[10];

    String str;

    if(var == "versions"){

        HTTPClient http;
        http.begin(URL);
        int httpResponseCode = http.GET();

        if (httpResponseCode>0) {
            Log.logf("HTTP Response code: %d\n", httpResponseCode);
            std::string payload = http.getString().c_str();

            std::size_t nextLine = 0;
            int i = 0;

            while(nextLine != std::string::npos) {
                nextLine = payload.find_first_of('\n');

                std::string line = payload.substr(0, nextLine);
                payload = payload.substr(nextLine+1);

                if(line.length() == 0)
                    continue;

                std::string filename = line.substr(line.find_last_of('/') + 1);

                size_t start = filename.find_last_of("_v") + 1;

                std::string name = filename.substr(0, start - 2);
                if (name != "firmware")
                    continue;

                str += "<input type='radio' name='ver' id='ota_";
                str += i;
                str += "' value='";
                str += filename.c_str();
                str += "'><label for='ota_";
                str += i;
                str += "'>";
                str += filename.c_str();
                str += "</label></br>";
                i++;
            }

        } else {
            Log.logf("Error code: %d\n", httpResponseCode);
        }

        http.end();


        return str;

    }
    return "missing preprocessor case";
}

