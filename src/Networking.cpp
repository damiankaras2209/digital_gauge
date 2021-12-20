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
    if(var == "ssid"){
        return String((char *)settings->general.ssid);
    } else if(var == "pass") {
        return String((char *)settings->general.pass);
    } else if(var == "offsetX") {
        return itoa(settings->visual.offsetX, c, 10);
    } else if(var == "offsetY") {
        return itoa(settings->visual.offsetY, c, 10);
    } else if(var == "ellipseA") {
        return itoa(settings->visual.ellipseA, c, 10);
    } else if(var == "ellipseB") {
        return itoa(settings->visual.ellipseB, c, 10);
    } else if(var == "needleCenterRadius") {
        return itoa(settings->visual.needleCenterRadius, c, 10);
    } else if(var == "needleCenterOffset") {
        return itoa(settings->visual.needleCenterOffset, c, 10);
    } else if(var == "needleLength") {
        return itoa(settings->visual.needleLength, c, 10);
    } else if(var == "needleBottomWidth") {
        return itoa(settings->visual.needleBottomWidth, c, 10);
    } else if(var == "needleTopWidth") {
        return itoa(settings->visual.needleTopWidth, c, 10);

    } else if(var == "dataDisplayLength") {
        return itoa(Settings::LAST, c, 10);

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
            ss << "<td>Preset<select id='input_" << i << "_preset' onchange='return preset(" << i <<");' type='checkbox' ><option value='-1'>Puste</option><option value='0'>Ciśń. oleju</option><option value='1'>Temp. oleju</option></section></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++)
            ss << "<td>R<input value='" << settings->input[i].r << "' type='number' step='0.1' id='input_" << i << "_rballance'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<6; i++) {
            ss << "<td>Typ<select id='input_" << i << "_type'>";
            for(int j=0; j != Settings::Dummy; j++){
                Settings::InputType type = static_cast<Settings::InputType>(j);
                ss << "<option value='" << j <<"' " << (settings->input[i].type == j ? "selected" : "") << ">" << settings->inputTypeString[j].c_str() << "</option>";
            }
            ss << "</select></td>";
        }
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

    } else if(var == "data_list") {
        std::stringstream ss;
        ss << "<table>\n<tr>";
        for(int i=0; i<Settings::LAST; i++)
            ss << "<td>" << settings->dataSourceString[i].c_str() << "</td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<Settings::LAST; i++)
            ss <<"<td>Włącz<input type='checkbox' id='data_" << i << "_en' " << (settings->dataDisplay[i].enable ? "checked" : "") << "></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<Settings::LAST; i++)
            ss << "<td>Nazwa<input value='" << (char *)(settings->dataDisplay[i].name) << "' type='text' id='data_" << i << "_name'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<Settings::LAST; i++)
            ss << "<td>Jednostka<input value='" << (char *)(settings->dataDisplay[i].unit) << "' type='text' id='data_" << i << "_unit'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<Settings::LAST; i++)
            ss << "<td>Początek skali<input value='" << settings->dataDisplay[i].scaleStart << "' type='number' step='1' id='data_" << i << "_scaleStart'></td>";
        ss << "</tr>\n<tr>";
        for(int i=0; i<Settings::LAST; i++)
            ss << "<td>Koniec skali<input value='" << settings->dataDisplay[i].scaleEnd << "' type='number' step='1' id='data_" << i << "_scaleEnd'></td>";
        ss << "</tr>\n<table>";
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

                std::string str = p->name().c_str();

                if(str.find("input") != std::string::npos) {
                    int i = str.at(6) - 48;
                    std::string str2 = str.substr(8);

                    if(str2.compare("rballance") == 0)
                        settings->input[i].r = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("type") == 0)
                        settings->input[i].type = static_cast<Settings::InputType>(strtof(p->value().c_str(), NULL));
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

                if(str.find("data") != std::string::npos) {

                    int pos = str.find('_');
                    int pos2 = str.find('_', pos + 1);

//                    Log.logf("pos: %d, n: %d", pos, pos2-pos-1);
//                    Log.logf("sub: %s", str.substr(pos+1,pos2-pos-1).c_str());
//                    Log.logf("d: %d", strtol(str.substr(pos+1,pos2-pos-1).c_str(), NULL, DEC));

                    int i = strtol(str.substr(pos+1,pos2-pos-1).c_str(), NULL, DEC);
                    std::string str2 = str.substr(pos2+1);

                    if(str2.compare("en") == 0)
                        settings->dataDisplay[i].enable = atoi(p->value().c_str());
                    else if(str2.compare("name") == 0)
                        strcpy((char *)settings->dataDisplay[i].name, p->value().c_str());
                    else if(str2.compare("unit") == 0)
                        strcpy((char *)settings->dataDisplay[i].unit, p->value().c_str());
                    else if(str2.compare("scaleStart") == 0)
                        settings->dataDisplay[i].scaleStart = strtof(p->value().c_str(), NULL);
                    else if(str2.compare("scaleEnd") == 0)
                        settings->dataDisplay[i].scaleEnd = strtof(p->value().c_str(), NULL);
                }

                if(p->name()=="ssid")
                    strcpy((char *)settings->general.ssid, p->value().c_str());
                else if(p->name()=="pass")
                    strcpy((char *)settings->general.pass, p->value().c_str());
                else if(p->name()=="offsetX")
                    settings->visual.offsetX = atoi(p->value().c_str());
                else if(p->name()=="offsetY")
                    settings->visual.offsetY = atoi(p->value().c_str());
                else if(p->name()=="ellipseA")
                    settings->visual.ellipseA = atoi(p->value().c_str());
                else if(p->name()=="ellipseB")
                    settings->visual.ellipseB = atoi(p->value().c_str());
                else if(p->name()=="needleCenterRadius")
                    settings->visual.needleCenterRadius = atoi(p->value().c_str());
                else if(p->name()=="needleCenterOffset")
                    settings->visual.needleCenterOffset = atoi(p->value().c_str());
                else if(p->name()=="needleLength")
                    settings->visual.needleLength = atoi(p->value().c_str());
                else if(p->name()=="needleBottomWidth")
                    settings->visual.needleBottomWidth = atoi(p->value().c_str());
                else if(p->name()=="needleTopWidth")
                    settings->visual.needleTopWidth = atoi(p->value().c_str());

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

