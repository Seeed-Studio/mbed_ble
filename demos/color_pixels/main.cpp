/* BLE Color Pixels
 */

#include "mbed.h"
#include "BLEDevice.h"
#include "color_pixels.h"

#define NEED_CONSOLE_OUTPUT 0 /* Set this if you need debug messages on the console;
                               * it will have an impact on code-size and power consumption. */

#if NEED_CONSOLE_OUTPUT
Serial  pc(USBTX, USBRX);
#define DEBUG(...) { pc.printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */

#define PIXELS_NUMBER   5

ColorPixels pixels(p4, PIXELS_NUMBER);

BLEDevice  ble;

// The Nordic UART Service
const uint8_t uart_base_uuid[]     = {0x6e, 0x40, 0x00, 0x01, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e};
const uint8_t uart_tx_uuid[]       = {0x6e, 0x40, 0x00, 0x02, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e};
const uint8_t uart_rx_uuid[]       = {0x6e, 0x40, 0x00, 0x03, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e};
const uint8_t uart_base_uuid_rev[] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e};

bool rxPayloadUpdated = false;
uint8_t rxPayload[20] = {0,};
uint8_t txPayload[20] = {0,};
GattCharacteristic  rxCharacteristic (uart_tx_uuid, rxPayload, 1, sizeof(rxPayload),
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
GattCharacteristic  txCharacteristic (uart_rx_uuid, txPayload, 1, sizeof(txPayload),
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&rxCharacteristic, &txCharacteristic};
GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));



void processPacket(uint8_t *packet)
{
    uint8_t red = packet[0];
    uint8_t green = packet[1];
    uint8_t blue = packet[2];

    uint8_t mode = packet[3];

    uint8_t number = packet[4] - 1;

    mode = mode & 1;

    DEBUG("r: %d, g: %d, b: %d, mode: %d\n\r", red, green, blue, mode);

    if (mode == 0) {
        pixels.set_color(number % PIXELS_NUMBER, red, green, blue);
        pixels.update();
    } else if (mode == 1) {
        for (int i = 0; i < PIXELS_NUMBER; i++) {
            pixels.set_color(i, red, green, blue);
        }
        pixels.update();
    } else if (mode == 2) {

    } else {

    }

}

void resetToRunBootloader()
{
    uint32_t *flag_address = (uint32_t *)0x20003FFC;
    *flag_address = 0xDECAF;
    NVIC_SystemReset();
}

void disconnectionCallback()
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising();
}

void onDataWritten(uint16_t charHandle)
{
    if (charHandle == rxCharacteristic.getHandle()) {
        DEBUG("onDataWritten()\n\r");
        uint16_t bytesRead;
        ble.readCharacteristicValue(rxCharacteristic.getHandle(), rxPayload, &bytesRead);
        if (memcmp(rxPayload, "decaf", 5) == 0) {
            resetToRunBootloader();
        }
        processPacket(rxPayload);
    }
}

int main(void)
{
    pixels.clear();
    pixels.set_color(0, 196, 0, 0);
    pixels.set_color(1, 196, 196, 0);
    pixels.set_color(2, 0, 196, 0);
    pixels.set_color(3, 0, 196, 196);
    pixels.set_color(4, 0, 0, 196);
    pixels.update();

    DEBUG("Initialising the nRF51822\n\r");
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);

    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                    (const uint8_t *)"Color Pixels", sizeof("Color Pixels") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                    (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid));

    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.startAdvertising();

    ble.addService(uartService);

    while (true) {
        ble.waitForEvent();
    }
}
