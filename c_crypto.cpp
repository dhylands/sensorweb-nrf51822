// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#include "c_crypto.h"

#include "Arduino.h"
#include "Curve25519.h"
#include "AES.h"

void Curve25519_dh1(uint8_t k[32], uint8_t f[32]) {
    Curve25519::dh1(k, f);
}

bool Curve25519_dh2(uint8_t k[32], uint8_t f[32]) {
    return Curve25519::dh2(k, f);
}

void aes128_decrypt(const uint8_t *key, uint8_t *output, const uint8_t *input, size_t len) {
    AES128 aes128;

    aes128.setKey(key, 16);
    while (len >= 16) {
        aes128.decryptBlock(output, input);
        output += 16;
        input += 16;
        len -= 16;
    }
    if (len > 0) {
        uint8_t dec[16], enc[16];
        memset(enc, 0, sizeof(enc));
        memcpy(enc, input, len);

        aes128.decryptBlock(dec, enc);

        memcpy(output, dec, len);
        memset(dec, 0, len);
    }
}

// Adding the following prevents libstdc++'s __verbose_terminate_handler
// from being pulled in, which causes 60K of flash to be consumed. The
// termination handler is only related to exceptions which we don't use.

namespace __gnu_cxx {
    void __verbose_terminate_handler() {
    }
}
extern "C" void __cxa_pure_virtual(void) {
}
