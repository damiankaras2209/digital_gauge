#include "Data.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

DataClass Data;


DataClass::DataClass() {

}


void DataClass::POST() {
    std::fill(status, status + D_LAST - 1, false);

    Log.logf("POST start");

    status[D_FT5436] = touch.init(&Serial);
    status[D_DS3231] = rtc.begin();
    status[D_ADS1115] = ads.begin();
    Wire.beginTransmission(0x50);
    status[D_A24C32] = (Wire.endTransmission() == 0);

    for(int i=0; i<D_LAST; i++) {
        Log.logf("%s... %s\n", deviceName[i].c_str(), status[i] ? "good" : "fail");
    }

    Log.logf("POST completed");
}

void DataClass::init() {

    data = DataStruct();
    data.adsPtr = &ads;
    data.rtcPtr = &rtc;
    data.dataInput = dataInput;


    if (status[D_DS3231]) {
        readTime(&data);
        if(rtc.lostPower()) {
            Log.logf("RTC lost power");
            rtc.adjust(DateTime(2099, 9, 99, 99, 99, 99));
        }
    }

    if(status[D_ADS1115]) {
        ads.setGain(1);
        ads.setDataRate(7);
    }

    touch.enableInterrupt(33, &(data.lock), 1, 0);

    TaskHandle_t adcHandle;
    if(!xTaskCreatePinnedToCore(adcLoop,
        "adcLoop",
        8*1024,
        &data,
        1,
        &adcHandle,
        0))
            Log.logf("Failed to start adcLoop task");

}


void DataClass::setEvent(SendEvent e) {
    _sendEvent = std::move(e);
}

void DataClass::setCountClients(DataClass::CountClients e) {
    _countClients = std::move(e);
}

double MySqr(double a_fVal) {  return a_fVal*a_fVal; }

unsigned long t18 = 0;

_Noreturn void DataClass::adcLoop(void * pvParameters) {
    Log.logf("%s started on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());

    auto *params = (DataStruct*)pvParameters;


    mu::Parser p;
    double voltage;
    double res;

    try {
        p.DefineVar("v", &voltage);
        p.DefineVar("r", &res);
    } catch (mu::Parser::exception_type &e) {
        Log.logf("Exception: %s\n", e.GetMsg().c_str());
    }

    uint32_t readings[SettingsClass::VOLTAGE + 1][SAMPLES_ADC];
    int readingsTaken[SettingsClass::VOLTAGE + 1];
    for(int i=0; i<SettingsClass::VOLTAGE + 1; i++)
        readingsTaken[i] = 0;

    for(;;) {
        for(int i=0; i <= SettingsClass::VOLTAGE && Data.status[D_ADS1115]; i++) {
            if(params->dataInput[i].visible) {

                for(int j= SAMPLES_ADC - 1; j > 0; j--)
                    readings[i][j] = readings[i][j - 1];

                if(i<4) {
                    params->lock.lock();
                    readings[i][0] = params->adsPtr->readADC(i);
                    params->lock.release();
                    delay(1);
                } else if(i==4)
                    readings[i][0] = analogReadMilliVolts(36);
                else if(i==5)
                    readings[i][0] = analogReadMilliVolts(39);
                else if(i == SettingsClass::VOLTAGE)
                    readings[i][0] = analogReadMilliVolts(34);

                readingsTaken[i] = min(++readingsTaken[i], SAMPLES_ADC);

                uint32_t sum = 0;
                for(int j=0; j < readingsTaken[i]; j++) {
                    sum += readings[i][j];
                }

                double avg = (double)sum / readingsTaken[i];


                if(i<4)
                    voltage = params->adsPtr->toVoltage((int16_t)lround(avg));
                else
                    voltage = avg/1000.0;

                params->dataInput[i].voltage = voltage;

                SettingsClass::Field** input = &(Settings.general[INPUT_BEGIN_BEGIN + i * INPUT_SETTINGS_SIZE]);
                res = input[INPUT_PULLUP_OFFSET]->get<float>() * voltage / (3.3 - voltage) + input[INPUT_SERIES_OFFSET]->get<float>();
                params->dataInput[i].resistance = res;

                auto up = input[INPUT_PULLUP_OFFSET]->get<double>();
                auto down = input[INPUT_PULLDOWN_OFFSET]->get<double>();
                auto series = input[INPUT_SERIES_OFFSET]->get<double>();

                p.DefineVar("up", &up);
                p.DefineVar("down", &down);
                p.DefineVar("series", &series);

                try {
                    p.SetExpr(Settings.general[INPUT_BEGIN_BEGIN + i * INPUT_SETTINGS_SIZE + INPUT_EXPRESSION_OFFSET]->getString());
                    params->dataInput[i].value = (float)p.Eval();
                } catch (mu::Parser::exception_type &e) {
                    Log.logf("Exception at %d: %s, \"%s\"\n", i, e.GetMsg().c_str(), e.GetExpr().c_str());
                }

            }
        }

        if(Data._countClients != nullptr && Data._sendEvent != nullptr) {
            if(millis() - t18 > 0 && Data._countClients() > 0) {
                std::stringstream ss;
                ss << "{";
                int x=0;
                for(int i=0; i<=SettingsClass::VOLTAGE + 1; i++) {
                    if(params->dataInput[i].visible) {
                        if (x > 0)
                            ss << ",";
                        ss << "\"" << i << "\":" << params->dataInput[i].toString();
                        x++;
                    }
                }
                ss << "}";
                Data._sendEvent("data", ss.str());
                t18 = millis();
            }
        }

        if(Data.status[D_DS3231] && millis()-params->lastRTC > 1000) {
            readTime(params);
        }

        delay(1);
    }
}

DataClass::DataInput *DataClass::getDataInput() {
    return dataInput;
}

DateTime DataClass::getTime() {
    return data.now;
}

void DataClass::readTime(DataClass::DataStruct *params) {
    params->lock.lock();
    auto now = params->rtcPtr->now();
    params->lock.release();
//            std::stringstream s;
//            s << std::setfill('0') << std::setw(2) << ((String)params->now.hour()).c_str() << ":" << std::setw(2) << ((String)params->now.minute()).c_str()  << ":" << std::setw(2) << ((String)params->now.second()).c_str();
//            Log.logf("RTC: %s\n", s.str().c_str());
    params->lastRTC = millis();

    int year = now.year();
    DateTime begin(year, 3, Date::findLastSunday(year, 3), 2, 0, 0);
    DateTime end(year, 10, Date::findLastSunday(year, 10), 2, 0, 0);
    if(now >= begin && now < end)
        now = now + TimeSpan(3600);

    if(now.hour() > 24 || now.minute() > 60 || now.day() > 31 || now.month() > 13 || now.year() > 2098)
        now = DateTime(2099, 9, 99, 99, 99);

    params->now = now;
}

int DataClass::adjustTime(DataStruct *params) {

    Log.logf("Getting time from server");
    tm time{};
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&time)){
        Log.logf("Failed to obtain time from server");
        return D_FAIL;
    }
    params->lock.lock();

    time.tm_mon += 1;
    time.tm_year += 1900;
    params->rtcPtr->adjust(DateTime(time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec));
     Log.logf("Got: %d.%d.%d %d:%d:%d\n",
             time.tm_mday, time.tm_mon, time.tm_year, time.tm_hour, time.tm_min, time.tm_sec);

    params->lock.release();
    return D_SUCCESS;
}
