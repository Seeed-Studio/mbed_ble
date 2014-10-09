/* BLE Color Pixels
 */

#include "mbed.h"
#include "BLEDevice.h"
#include "color_pixels.h"
#include "UARTService.h"

#define NEED_CONSOLE_OUTPUT 1 /* Set this if you need debug messages on the console;
                               * it will have an impact on code-size and power consumption. */

#if NEED_CONSOLE_OUTPUT
#define DEBUG(...) { printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */

#define PIXELS_NUMBER   5

ColorPixels pixels(p4, PIXELS_NUMBER);

BLEDevice  ble;
DigitalOut led1(LED1);

UARTService *uartServicePtr;

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

void disconnectionCallback(Gap::Handle_t handle, Gap::DisconnectionReason_t reason)
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising();
}

void onDataWritten(const GattCharacteristicWriteCBParams *params)
{
    if ((uartServicePtr != NULL) && (params->charHandle == uartServicePtr->getTXCharacteristicHandle())) {
        uint16_t bytesRead = params->len;
        DEBUG("received %u bytes\n\r", bytesRead);
        processPacket((uint8_t *) params->data);
    }
}

void periodicCallback(void)
{
    led1 = !led1;
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

    led1 = 1;
    Ticker ticker;
    ticker.attach(periodicCallback, 1);

    DEBUG("Initialising the nRF51822\n\r");
    ble.init();
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);

    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                     (const uint8_t *)"BLE UART", sizeof("BLE UART") - 1);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_128BIT_SERVICE_IDS,
                                     (const uint8_t *)UARTServiceUUID_reversed, sizeof(UARTServiceUUID_reversed));

    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.startAdvertising();

    UARTService uartService(ble);
    uartServicePtr = &uartService;

    while (true) {
        ble.waitForEvent();
    }
}
