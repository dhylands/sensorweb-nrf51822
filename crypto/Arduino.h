#include <stdio.h>

class CustomSerial
{
public:
    void print(const char *str) {
        fputs(str, stdout);
    }

    void print(char ch) {
        fputc(ch, stdout);
    }

    void print(unsigned long num) {
        fprintf(stdout, "%lu", num);
    }

    void println(const char *str = "") {
        print(str);
        print("\n");
    }

    void begin(int baud) {
    }

    void flush() {
        fflush(stdout);
    }
};

extern CustomSerial Serial;

unsigned long micros();
unsigned long millis();
