#include <iomanip>
#include "Log.h"

LogClass Log;

void LogClass::_log(std::string str) {
#ifdef ENABLE_SERVER_LOG
    Serial.printf("message %d: %s\n", _data.messages.size() , str.c_str());
    lock();
    if(_data.messages.size() < MAX_MESSAGES)
        _data.messages.push_back(str);
    release();
#else
    Serial.print(str.c_str());
#endif
}

#ifdef ENABLE_SERVER_LOG

void LogClass::sendMessages(void *params) {
    auto data = (Data*)params;
    for(;;) {
        while(data->busy)
            delay(1);
        data->busy = true;
        if(!data->messages.empty()) {
            Serial.printf("Sending: %s\n", data->messages.front().c_str());
            data->_send(data->messages.front());
            data->messages.erase(data->messages.begin());
        }
        data->busy = false;
        delay(1);
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

#endif

void LogClass::setEvent(Send send) {
    _data._send = send;
}

void LogClass::enable() {
#ifdef ENABLE_SERVER_LOG
    TaskHandle_t handle;
    if(!xTaskCreatePinnedToCore(sendMessages,
                                "sendMessagesTask",
                                4*1024,
                                &_data,
                                1,
                                &handle,
                                0))
        log("Failed to start canLoop task");
#endif
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