// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at http://mozilla.org/MPL/2.0/.

#if !defined(CRYPTO_H)
#define CRYPTO_H

#include <stdint.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

void Curve25519_dh1(uint8_t k[32], uint8_t f[32]);
bool Curve25519_dh2(uint8_t k[32], uint8_t f[32]);
void aes128_decrypt(const uint8_t *key, uint8_t *output, const uint8_t *input, size_t len);

#if defined(__cplusplus)
}
#endif

#endif
