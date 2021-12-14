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
    Serial.println(str.c_str());
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
    _log(str);
}

void LogClass::log(int i) {
    std::stringstream ss;
    ss << i;
    _log(ss.str().c_str());
}

void LogClass::log(uint32_t i) {
    std::stringstream ss;
    ss << i;
    _log(ss.str().c_str());
}

void LogClass::log(long unsigned int i) {
    std::stringstream ss;
    ss << i;
    _log(ss.str().c_str());
}

void LogClass::log(float f) {
    std::stringstream ss;
    ss << f;
    _log(ss.str().c_str());
}

void LogClass::log(String str) {
    _log(str.c_str());
}

void LogClass::log(StringSumHelper& stringSumHelper) {
    _log(stringSumHelper.c_str());
}