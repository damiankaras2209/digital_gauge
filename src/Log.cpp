#include <iomanip>
#include <utility>
#include "Log.h"

LogClass Log;

void LogClass::_log(std::string str) {
    Serial.printf("%d: %s", _data.messages.size() , str.c_str());
    lock();
    if(_data.messages.size() < MAX_MESSAGES)
        _data.messages.push_back(str);
    release();
}

void LogClass::sendMessages(void *params) {
    auto data = (Data*)params;
    for(;;) {
        while(data->busy)
            delay(1);
        data->busy = true;
//        Serial.print("Connected clients: ");
//        Serial.println(data->countClients());
        if(data->countClients() > 0) {
            for(int i=data->_sent; i<data->messages.size();i++) {
//                Serial.printf("Sending: %s", data->messages.at(i).c_str());
                data->_send(data->messages.at(i));
                data->_sent++;
            }
        }
//        if(!data->messages.empty()) {
//            Serial.printf("Sending: %s\n", data->messages.front().c_str());
//            data->_send(data->messages.front());
//            data->messages.erase(data->messages.begin());
//        }
        data->busy = false;
        delay(10);
    }
}

void LogClass::lock() {
    while(_data.busy)
        delay(1);
    _data.busy = true;
}

void LogClass::release() {
    _data.busy = false;
}

void LogClass::setEvent(Send send) {
    _data._send = std::move(send);
}

void LogClass::setCountClients(std::function<size_t()> f) {
    _data.countClients = std::move(f);
}

void LogClass::enable() {
    if(!_enabled) {
        TaskHandle_t handle;
        if(!xTaskCreatePinnedToCore(sendMessages,
                                    "sendMessagesTask",
                                    4*1024,
                                    &_data,
                                    1,
                                    &handle,
                                    0))
            log("Failed to start canLoop task");
        _enabled = true;
    }
}

void LogClass::onConnect() {
    _data._sent = 0;
}

void LogClass::log(const char* str) {
    std::string s = std::string(str);
    s.append("\n");
    _log(s);
}

void LogClass::log(int i) {
    std::stringstream ss;
    ss << i << "\n";
    _log(ss.str());
}

void LogClass::log(uint32_t i) {
    std::stringstream ss;
    ss << i << "\n";
    _log(ss.str());
}

void LogClass::log(long unsigned int i) {
    std::stringstream ss;
    ss << i << "\n";
    _log(ss.str());
}

void LogClass::log(float f) {
    std::stringstream ss;
    ss << f << "\n";
    _log(ss.str());
}

void LogClass::log(String str) {
    str = str + "\n";
    _log(str.c_str());
}

void LogClass::log(StringSumHelper& stringSumHelper) {
    _log((stringSumHelper + "\n").c_str());
}

void LogClass::log(unsigned long n, int base)
{
    std::stringstream ss;
    switch(base){
        case DEC: ss << n; break;
        case HEX: ss << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << n; break;
    }
    _log(ss.str().c_str());
}
