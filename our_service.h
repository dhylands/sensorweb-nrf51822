
#ifndef OUR_SERVICE_H__
#define OUR_SERVICE_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

// Generated the following UUID using https://www.uuidgenerator.net/ to use
// as our BLE Service UUID.
// 
// 9e91c020-e20b-11e6-bf01-fe55135034f3 - BLE Service UUID
// 9e910001-e20b-11e6-bf01-fe55135034f3 - Device/Server Public Key Characteristic UUID
// 9e910002-e20b-11e6-bf01-fe55135034f3 - Phone/Client  Public Key Characteristic UUID
// 9e910003-e20b-11e6-bf01-fe55135034f3 - Password Characteristic UUID
// 9e910004-e20b-11e6-bf01-fe55135034f3 - SSID Characteristic UUID
//
// Service Private Key = 68D6319DE50EE1083320C8FC1E1EB18E544C80872403E74372C0492FDDF87543
// Service Public  Key = D92D7A3FEE2F675F475FC452A681A9255C2150553702E5F127D837623D4C8135
// Client  Private Key = C0A9B304B1FB77CCA7FBD3D88D0D1AB27DFA2D53D9032978258C81772CB47E49
// Client  Public  Key = 5CFFD4B3E8391E6EC63070C1A55CB282BD6E3C7D28BFA27B095EE69EA426B26D
// Client  Shared Secret = A35EE00E403DFC815DA788A8EE4C08A999CAF1AEC2634F75584CE7D5379F9A55
// Service Shared Secret = A35EE00E403DFC815DA788A8EE4C08A999CAF1AEC2634F75584CE7D5379F9A55

// SSID org = 4D792D5353494400AAAAAAAAAAAAAAAA
// SSID dec = 4D792D5353494400AAAAAAAAAAAAAAAA
// SSID enc = C5A0E2035AF9BF4E3FD46D8058FBD046
// password org = 70617373776F726400AAAAAAAAAAAAAA
// password dec = 70617373776F726400AAAAAAAAAAAAAA
// password enc = EB396136E87F2A3EF17BD40C86E9F1F5

// SSID org = 4D792D5353494400997518B43FCEAFA9
// SSID dec = 4D792D5353494400997518B43FCEAFA9
// SSID enc = C103F399B11EE4E3E8B6B6255BF2903C
// password org = 70617373776F7264008B375AD22EB12E
// password dec = 70617373776F7264008B375AD22EB12E
// password enc = 0682B5684FE2701966AC1060DB83F194


// FROM_SERVICE_TUTORIAL: Defining 16-bit service and 128-bit base UUIDs
//#define BLE_UUID_OUR_BASE_UUID              {{0x23, 0xD1, 0x13, 0xEF, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00}} // 128-bit base UUID
#define BLE_UUID_OUR_BASE_UUID              {{0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}} // 128-bit base UUID
#define BLE_UUID_OUR_SERVICE_UUID                0xa123 // Just a random, but recognizable value

// ALREADY_DONE_FOR_YOU: Defining 16-bit characteristic UUID
#define BLE_UUID_OUR_CHARACTERISTC_UUID          0xccc1 // Just a random, but recognizable value

// Structure which contains information about a characteristic.
typedef struct {
    uint16_t                    uuid;
    bool                        write_only;
    uint8_t                     value_len;
    uint8_t                    *value;
    ble_gatts_char_handles_t    handles;
    bool                        written;
} ble_char_t;

// This structure contains various status information for our service. 
// The name is based on the naming convention used in Nordics SDKs. 
// 'ble’ indicates that it is a Bluetooth Low Energy relevant structure and 
// ‘os’ is short for Our Service). 
typedef struct
{
    uint16_t    conn_handle;    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).*/
    uint16_t    service_handle; /**< Handle of Our Service (as provided by the BLE stack). */
    ble_char_t  char_our_public_key;
    ble_char_t  char_client_public_key;
    ble_char_t  char_ssid;
    ble_char_t  char_password;
} ble_os_t;

void printhex(const char *name, const uint8_t *data, unsigned n);

/**@brief Function for handling BLE Stack events related to our service and characteristic.
 *
 * @details Handles all events from the BLE stack of interest to Our Service.
 *
 * @param[in]   p_our_service       Our Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_our_service_on_ble_evt(ble_os_t * p_our_service, ble_evt_t * p_ble_evt);

/**@brief Function for initializing our new service.
 *
 * @param[in]   p_our_service       Pointer to Our Service structure.
 */
void our_service_init(ble_os_t * p_our_service);

/**@brief Function for updating and sending new characteristic values
 *
 * @details The application calls this function whenever our timer_timeout_handler triggers
 *
 * @param[in]   p_our_service                     Our Service structure.
 * @param[in]   characteristic_value     New characteristic value.
 */
void our_termperature_characteristic_update(ble_os_t *p_our_service, int32_t *temperature_value);

#endif  /* _ OUR_SERVICE_H__ */
