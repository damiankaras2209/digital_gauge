#ifndef _LOG_H_
#define _LOG_H_

#include <memory>
#include <string>
#include <HardwareSerial.h>
#include <AsyncEventSource.h>
#include <mutex>

#define MAX_MESSAGES 256

typedef std::function<void(std::string, ulong id)> Send;

class LogClass {
    private:
        typedef struct Data {
            std::vector<std::pair<ulong, std::string>> messages;
            std::function<size_t()> countClients;
            Send _send;
            uint _sent = 0;
            std::mutex guard;
            unsigned long time = 0;
        } Data;

        Data _data;
        bool _enabled = false;

        void _log(std::string str, bool addNewLine = false);
        [[noreturn]] static void sendMessages(void *);


    public:
        std::vector<std::pair<ulong, std::string>>* getMessages();
        std::mutex* getGuard();
        void setSent(uint sent);
        void setEvent(Send send);
        void setCountClients(std::function<size_t()> f);
        void enable();
        // void onConnect();

        template<typename ... Args>
        void logf(const char* format, Args ... args) {
            if(sizeof...(args) > 0 ) {
                const size_t size = snprintf( nullptr, 0, format, args ... ) + 1; // Extra space for '\0'
                if( size <= 0 ) {
                    logf("Error during formatting. '%s'", format);
                    return;
                }
                const std::unique_ptr<char[]> buf( new char[ size ] );
                snprintf( buf.get(), size, format, args ... );
                _log(buf.get(), false);
            } else {
                _log(format, true);
            }
        }

        //Debug log, printed only to Serial
        template<typename ... Args>
        void logf_d(const char* format, Args ... args) {
            const size_t size = snprintf( nullptr, 0, format, args ... ) + 1; // Extra space for '\0'
            if( size <= 0 ) {
                logf_d("Error during formatting. '%s'", format);
                return;
            }
            const std::unique_ptr<char[]> buf( new char[ size ] );
            snprintf( buf.get(), size, format, args ... );
            Serial.print(buf.get());
        }
};

extern LogClass Log;

#endif