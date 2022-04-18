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
    });

    server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/log.html", "text/html", false, nullptr);
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
            Log.log("Resetting screen");
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
    });;

    server->on("/screenshot", HTTP_POST, [](AsyncWebServerRequest *request){
        Screen.enableTouch(false);
        Screen.pause(true, true);
        AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {

            if (!index)
                Log.log("Begin response");

            int width = Settings.general[WIDTH]->get<int>();
            int height = Settings.general[HEIGHT]->get<int>();
            int lineSize = 3 * width;

            int line = (int) floor(index / 3) / width;
            int newLine = (int) floor(index / 3) % width;

            if (line == height) {
                Screen.pause(false, false);
                Screen.enableTouch(true);
                return 0;
            }


//            Log.logf("index: %d, maxLen: %d, line: %d, lineProgress: %d\n", index, maxLen, line, newLine);

            int writtenNow = 0;
            int linesWritten = 0;

            if (newLine > 0) {
                int length = min(width - newLine, (int) floor(maxLen / 3));
                Screen.tft->readRectRGB(newLine, line, length, 1, buffer);
//                Log.logf("writing %d pixels from %d to line end\n", length, newLine);
                writtenNow += 3 * length;
            }

            line = (int) floor((index + writtenNow) / 3) / width;
            newLine = (int) floor((index + writtenNow) / 3) % width;

            if (newLine > 0)
                return writtenNow;

            while (maxLen - writtenNow > lineSize && line < height) {
//                Log.logf("writing line: %d\n", line);
                Screen.tft->readRectRGB(0, line, width, 1, buffer + writtenNow);
                line++;
                linesWritten++;
                writtenNow += lineSize;
            }

            if (line < height - 1) {
                int spaceLeft = (int) floor((maxLen - writtenNow) / 3);
//                Log.logf("writing remaining space: %d\n", spaceLeft);
                Screen.tft->readRectRGB(0, line, spaceLeft, 1, buffer + writtenNow);
                writtenNow += 3 * spaceLeft;
            }

//            Log.logf("\n");

            if (writtenNow == 0) {
                Screen.pause(false, false);
                Screen.enableTouch(true);
            }

            return writtenNow;
        });
        response->setContentType("application/octet-stream");
        request->send(response);
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

        str += (String)"const MAC = '" + Updater.getMac().c_str() + "';\n";
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
                str += (String) "<tr clastr='input.ADC" + i + "'><td>ADC" + i + "</td>";

            str += (String) "<td>Preset <select id='input_" + i + "_preset' onchange='return preset(" + i +");' type='checkbox' ><option value='0'>Puste</option><option value='1'>Ciśń. oleju</option><option value='2'>Temp. oleju</option></section></td>";

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
