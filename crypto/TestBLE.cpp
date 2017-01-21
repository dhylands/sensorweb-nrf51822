#include <stdio.h>
#include <string.h>
#include "Arduino.h"
#include "Curve25519.h"
#include "RNG.h"
#include "AES.h"

void printNumber(const char *name, const uint8_t *x, uint8_t len)
{
    static const char hexchars[] = "0123456789ABCDEF";
    Serial.print(name);
    Serial.print(" = ");
    for (uint8_t posn = 0; posn < len; ++posn) {
        Serial.print(hexchars[(x[posn] >> 4) & 0x0F]);
        Serial.print(hexchars[x[posn] & 0x0F]);
    }
    Serial.println();
}

int main(int argc, char **argv)
{
    // Start the random number generator.  We don't initialise a noise
    // source here because we don't need one for testing purposes.
    // Real applications should of course use a proper noise source.
    RNG.begin("TestBLE 1.0", 950);

    uint8_t servicePrivateKey[32];
    uint8_t servicePublicKey[32];

    Curve25519::dh1(servicePublicKey, servicePrivateKey);

    printNumber("Service Private Key", servicePrivateKey, 32);
    printNumber("Service Public  Key", servicePublicKey, 32);

    uint8_t clientPrivateKey[32];
    uint8_t clientPublicKey[32];

    Curve25519::dh1(clientPublicKey, clientPrivateKey);

    printNumber("Client  Private Key", clientPrivateKey, 32);
    printNumber("Client  Public  Key", clientPublicKey, 32);

    uint8_t serviceSharedSecret[32];
    uint8_t clientSharedSecret[32];

    memcpy(clientSharedSecret, clientPublicKey, sizeof(clientSharedSecret));
    Curve25519::dh2(clientSharedSecret, servicePrivateKey);

    memcpy(serviceSharedSecret, servicePublicKey, sizeof(serviceSharedSecret));
    Curve25519::dh2(serviceSharedSecret, clientPrivateKey);

    printNumber("Client  Shared Secret", clientSharedSecret, 32);
    printNumber("Service Shared Secret", serviceSharedSecret, 32);

    uint8_t ssid[32];
    uint8_t password[64];

    RNG.rand(ssid, sizeof(ssid));
    RNG.rand(password, sizeof(password));

    strcpy((char *)ssid, "My-SSID");
    strcpy((char *)password, "password");

    AES128 aes128;

    printf("Sizeof(aes128) = %zu\n", sizeof(aes128));
    aes128.setKey(clientSharedSecret, 16);

    uint8_t ssid_enc[16];
    uint8_t password_enc[16];
    aes128.encryptBlock(ssid_enc, ssid);
    aes128.encryptBlock(password_enc, password);

    uint8_t ssid_dec[16];
    uint8_t password_dec[16];
    aes128.decryptBlock(ssid_dec, ssid_enc);
    aes128.decryptBlock(password_dec, password_enc);

    printNumber("SSID org", ssid, 16);
    printNumber("SSID dec", ssid_dec, 16);
    printNumber("SSID enc", ssid_enc, 16);

    printNumber("password org", password, 16);
    printNumber("password dec", password_dec, 16);
    printNumber("password enc", password_enc, 16);
}
