#include <iomanip>
#include <utility>
#include "Log.h"

LogClass Log;

void LogClass::_log(std::string str, const bool addNewLine) {
    if(addNewLine) {
        str += "\n";
    }
    Serial.print(str.c_str());
    {
        std::lock_guard<std::mutex> lock(_data.guard);
        if(_data.messages.size() < MAX_MESSAGES)
            _data.messages.emplace_back(millis(),str);
    }
}

void LogClass::sendMessages(void *params) {
    const auto data = static_cast<Data*>(params);
    for(;;) {
        if(data->countClients() > 0) {
            {
                std::lock_guard<std::mutex> lock(data->guard);
                for(uint i=data->_sent; i<data->messages.size();i++) {
                    data->_send(data->messages.at(i).second, data->messages.at(i).first);
                    data->_sent++;
                }
            }
        }
        delay(10);
    }
}

std::vector<std::pair<ulong, std::string>>* LogClass::getMessages() {
    return &_data.messages;
}

std::mutex* LogClass::getGuard() {
    return &_data.guard;
}

void LogClass::setSent(const uint sent) {
    _data._sent = sent;
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
                                    1))
            logf("Failed to start sendMessagesTask task");
        else {
            logf("sendMessagesTask started on core 0");
        }
        _enabled = true;
    }
}

// void LogClass::onConnect() {
//     _data._sent = 0;
// }