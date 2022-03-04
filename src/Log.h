#ifndef _LOG_H_
#define _LOG_H_

#include <memory>
#include <string>
#include <stdexcept>
#include <HardwareSerial.h>
#include <AsyncEventSource.h>
#include <sstream>

//#define LOG_SETTINGS
//#define LOG_FRAMETIME
//#define LOG_DETAILED_FRAMETIME

#define MAX_MESSAGES 256


typedef std::function<void(std::string)> Send;

class LogClass {
    private:

        typedef struct Data {
            std::vector<std::string> messages;
            std::function<size_t()> countClients;
            Send _send;
            int _sent = 0;
            bool busy = false;
            unsigned long time = 0;
        } Data;

        Data _data;
        bool _enabled = false;

        void _log(std::string str);


    private:
        [[noreturn]] static void sendMessages(void *);
    public:
        void lock();
        void release();


    public:
        void setEvent(Send send);
        void setCountClients(std::function<size_t()>);
        void enable();
        void onConnect();

        template<typename ... Args>
        void logf(const char* format, Args ... args)
        {
            size_t size = snprintf( nullptr, 0, format, args ... ) + 1; // Extra space for '\0'
            if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
            std::unique_ptr<char[]> buf( new char[ size ] );
            snprintf( buf.get(), size, format, args ... );
            _log(buf.get());
        }

        void log(const char*);
        void log(int);
        void log(uint32_t);
        void log(long unsigned int);
        void log(float);
        void log(String);
        void log(StringSumHelper&);
        void log(unsigned long n, int base);
};

extern LogClass Log;

#endif