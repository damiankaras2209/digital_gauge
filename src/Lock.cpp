#include <esp32-hal.h>
#include "Lock.h"

void Lock::lock() {
    while(isBusy)
        delay(1);
    isBusy = true;
}

void Lock::release() {
    isBusy = false;
}
