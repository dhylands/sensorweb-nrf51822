
#include <stdint.h>
#include <string.h>

#include "nrf_gpio.h"
#include "our_service.h"
#include "ble_srv_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "c_crypto.h"

#define QUEUED_WRITE_BUFFER_SIZE 128
static ble_user_mem_block_t mem_block;
static uint8_t queued_write_buffer[QUEUED_WRITE_BUFFER_SIZE];

static uint8_t test_private_key[32] = {0x68, 0xD6, 0x31, 0x9D, 0xE5, 0x0E, 0xE1, 0x08,
                                       0x33, 0x20, 0xC8, 0xFC, 0x1E, 0x1E, 0xB1, 0x8E,
                                       0x54, 0x4C, 0x80, 0x87, 0x24, 0x03, 0xE7, 0x43,
                                       0x72, 0xC0, 0x49, 0x2F, 0xDD, 0xF8, 0x75, 0x43};

static uint8_t test_public_key[32]  = {0xD9, 0x2D, 0x7A, 0x3F, 0xEE, 0x2F, 0x67, 0x5F,
                                       0x47, 0x5F, 0xC4, 0x52, 0xA6, 0x81, 0xA9, 0x25,
                                       0x5C, 0x21, 0x50, 0x55, 0x37, 0x02, 0xE5, 0xF1,
                                       0x27, 0xD8, 0x37, 0x62, 0x3D, 0x4C, 0x81, 0x35};

//uint32_t sharedSecret[32];

void printhex(const char *name, const uint8_t *data, unsigned n)
{
    static const char hexchars[] = "0123456789ABCDEF";
    printf("%s = ", name);
    for (uint8_t posn = 0; posn < n; ++posn) {
        putchar(hexchars[(data[posn] >> 4) & 0x0F]);
        putchar(hexchars[data[posn] & 0x0F]);
    }
    putchar('\r');
    putchar('\n');
    //nrf_delay_ms(100);
}

#define STX                 '\x02'
#define ETX                 '\x03'
#define PKT_TYPE_SSID       'S'
#define PKT_TYPE_PASSWORD   'P'

static void send_char(uint8_t ch) {
#if 1   // Just for debugging
    if ((ch < ' ') || (ch > '~')) {
        char buf[10];
        snprintf(buf, sizeof(buf), "<%02x>", ch);
        send_char(buf[0]);
        send_char(buf[1]);
        send_char(buf[2]);
        send_char(buf[3]);
        return;
    }
#endif
    while (app_uart_put(ch) != NRF_SUCCESS) {
        ;
    }
}

static void send_packet(uint8_t pkt_type, const uint8_t *pkt_data) {
    uint8_t checksum = 0;
    uint8_t len = strlen((const char *)pkt_data) + 2; // +1 for pkt-type, +1 for checksum

    send_char(STX);
    send_char(len);
    checksum ^= len;
    send_char(pkt_type);
    checksum ^= pkt_type;
    while (*pkt_data != '\0') {
        uint8_t ch = *pkt_data++;
        send_char(ch);
        checksum ^= ch;
    }
    send_char(checksum);
    send_char(ETX);
    putchar('\r');
    putchar('\n');
}

/**@brief Function for handling BLE GATTS EVENTS
 * 
 * This function prints out data that is received when you try to write to your characteristic or CCCD. 
 * In general it is a bad idea to to so much printf stuff and UART transfer inside the BLE handler,
 * but this is just for demonstrate purposes.
 *
 * @param[in]   p_our_service        Our Service structure.
 * @param[in]   p_ble_evt            BLE event passed from BLE stack
 *
 */
static void on_ble_write(ble_os_t * p_our_service, ble_evt_t * p_ble_evt)
{
    uint16_t handle = 0;
    ble_gatts_value_t rx_data;
    memset(&rx_data, 0, sizeof(rx_data));

    if (p_ble_evt->evt.gatts_evt.params.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) {
        // Long Write on a characteristic
        handle = *(uint16_t *)queued_write_buffer;
    } else {
        handle = p_ble_evt->evt.gatts_evt.params.write.handle;
    }

    ble_char_t *p_our_char = NULL;
    const char *label = "";

    if (handle == p_our_service->char_client_public_key.handles.value_handle) {
        label = "Client Public Key";
        p_our_char = &p_our_service->char_client_public_key;
    } else if (handle == p_our_service->char_ssid.handles.value_handle) {
        label = "SSID";
        p_our_char = &p_our_service->char_ssid;
    } else if (handle == p_our_service->char_password.handles.value_handle) {
        label = "Password";
        p_our_char = &p_our_service->char_password;

    }

    if (p_our_char != NULL) {
        p_our_char->written = true;
        rx_data.len = p_our_char->value_len;
        rx_data.p_value = p_our_char->value;
        sd_ble_gatts_value_get(p_our_service->conn_handle,
                               p_our_char->handles.value_handle,
                               &rx_data);
        printhex(label, rx_data.p_value, rx_data.len);
    }

    if (p_our_service->char_client_public_key.written
    &&  p_our_service->char_ssid.written
    &&  p_our_service->char_password.written) {
        uint8_t sharedSecret[32];
        memcpy(sharedSecret, p_our_service->char_client_public_key.value, sizeof(sharedSecret));
        if (Curve25519_dh2(sharedSecret, test_private_key)) {
            uint8_t ssid[33];
            uint8_t password[65];
            aes128_decrypt(sharedSecret, ssid, p_our_service->char_ssid.value, p_our_service->char_ssid.value_len);
            aes128_decrypt(sharedSecret, password, p_our_service->char_password.value, p_our_service->char_password.value_len);
            printf("SSID = '%s', password = '%s'\r\n", ssid, password);
            send_packet(PKT_TYPE_SSID, ssid);
            send_packet(PKT_TYPE_PASSWORD, password);
        } else {
            printf("Key was invalid\r\n");
            printhex("Shared Secret", sharedSecret, 32);
        }
        memset(sharedSecret, 0, sizeof(sharedSecret));
    }
}

// ALREADY_DONE_FOR_YOU: Declaration of a function that will take care of some housekeeping of ble connections related to our service and characteristic
void ble_our_service_on_ble_evt(ble_os_t * p_our_service, ble_evt_t * p_ble_evt)
{

    // OUR_JOB: Step 3.D Implement switch case handling BLE events related to our service. 
    switch (p_ble_evt->header.evt_id)
    {        
        case BLE_GAP_EVT_CONNECTED:
            p_our_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            memcpy(p_our_service->char_our_public_key.value, test_public_key, p_our_service->char_our_public_key.value_len);

            ble_gatts_value_t data;
            memset(&data, 0, sizeof(data));
            data.len = p_our_service->char_our_public_key.value_len;
            data.p_value = p_our_service->char_our_public_key.value;
            sd_ble_gatts_value_set(p_our_service->conn_handle,
                                   p_our_service->char_our_public_key.handles.value_handle,
                                   &data);
            p_our_service->char_our_public_key.written = false;
            p_our_service->char_client_public_key.written = false;
            p_our_service->char_ssid.written = false;
            p_our_service->char_password.written = false;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID) {
                mem_block.len = QUEUED_WRITE_BUFFER_SIZE;
                mem_block.p_mem = &queued_write_buffer[0];
                sd_ble_user_mem_reply(p_our_service->conn_handle, &mem_block);
                printf("BLE_EVT_USER_MEM_REQUEST\r\n");
            }
            break;

        case BLE_EVT_USER_MEM_RELEASE:
            printf("BLE_EVT_USER_MEM_RELEASE\r\n");
            break;

        case BLE_GATTS_EVT_WRITE:
            on_ble_write(p_our_service, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for adding our new characterstic to "Our service" that we initiated in the previous tutorial. 
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
static uint32_t our_char_add(ble_os_t * p_our_service, ble_char_t *p_our_char)
{
    uint32_t   err_code = 0; // Variable to hold return codes from library and softdevice functions
    
    // OUR_JOB: Step 2.A, Add a custom characteristic UUID
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_OUR_BASE_UUID;

    char_uuid.uuid      = p_our_char->uuid;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    printf("sd_ble_uuid_vs_add returned %lu\r\n", err_code);
    APP_ERROR_CHECK(err_code);
    
    // OUR_JOB: Step 2.F Add read/write properties to our characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    if (p_our_char->write_only) {
        char_md.char_props.write = 1;
    } else {
        char_md.char_props.read = 1;
    }
    
    // OUR_JOB: Step 3.A, Configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
   
    
    // OUR_JOB: Step 2.B, Configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md)); 
    //attr_md.vloc        = BLE_GATTS_VLOC_STACK;
    attr_md.vloc        = BLE_GATTS_VLOC_USER;
    
    
    // OUR_JOB: Step 2.G, Set read/write security levels to our characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    
    // OUR_JOB: Step 2.C, Configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));        
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    
    // OUR_JOB: Step 2.H, Set characteristic length in number of bytes
    attr_char_value.max_len     = p_our_char->value_len;
    attr_char_value.init_len    = p_our_char->value_len;
    attr_char_value.p_value     = p_our_char->value;

    // OUR_JOB: Step 2.E, Add our new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_our_service->service_handle,
                                       &char_md,
                                       &attr_char_value,
                                       &p_our_char->handles);
    //printf("sd_ble_gatts_characteristic_add returned %lu\r\n", err_code);
    APP_ERROR_CHECK(err_code);
    
    //printf("\r\nService handle: %#x\n\r", p_our_service->service_handle);
    printf("Char value handle: %#x\r\n", p_our_char->handles.value_handle);
    //printf("Char cccd handle: %#x\r\n\r\n", p_our_char->handles.cccd_handle);

    return NRF_SUCCESS;
}


/**@brief Function for initiating our new service.
 *
 * @param[in]   p_our_service        Our Service structure.
 *
 */
void our_service_init(ble_os_t * p_our_service)
{
    uint32_t   err_code; // Variable to hold return codes from library and softdevice functions

    // FROM_SERVICE_TUTORIAL: Declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_UUID_OUR_BASE_UUID;
    service_uuid.uuid = BLE_UUID_OUR_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    
    
    // OUR_JOB: Step 3.B, Set our service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    p_our_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    // FROM_SERVICE_TUTORIAL: Add our service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_our_service->service_handle);
    
    APP_ERROR_CHECK(err_code);
    
    // OUR_JOB: Call the function our_char_add() to add our new characteristic to the service. 
    our_char_add(p_our_service, &p_our_service->char_our_public_key);
    our_char_add(p_our_service, &p_our_service->char_client_public_key);
    our_char_add(p_our_service, &p_our_service->char_ssid);
    our_char_add(p_our_service, &p_our_service->char_password);
}

#if 0
// ALREADY_DONE_FOR_YOU: Function to be called when updating characteristic value
void our_termperature_characteristic_update(ble_os_t *p_our_service, int32_t *temperature_value)
{
    // OUR_JOB: Step 3.E, Update characteristic value
    if (p_our_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = 4;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_our_service->char_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)temperature_value;  

        sd_ble_gatts_hvx(p_our_service->conn_handle, &hvx_params);
    }   
}
#endif
