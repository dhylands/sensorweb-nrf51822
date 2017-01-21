#include "Arduino.h"

#include <sys/time.h>

CustomSerial Serial;

unsigned long micros() {
    struct timeval tv;
    static time_t first_time = 0;

    gettimeofday(&tv, NULL);

    if (first_time == 0) {
        first_time = tv.tv_sec;
    }

    return (tv.tv_sec - first_time) * 1000000 + tv.tv_usec;
}

unsigned long millis() {
    return micros() / 1000;
}
