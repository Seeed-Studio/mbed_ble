/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "BLEDevice.h"

#define BLE_UUID_NUS_SERVICE            0x0001 /**< The UUID of the Nordic UART Service. */
#define BLE_UUID_NUS_TX_CHARACTERISTIC  0x0002 /**< The UUID of the TX Characteristic. */
#define BLE_UUID_NUS_RX_CHARACTERISTIC  0x0003 /**< The UUID of the RX Characteristic. */

#define NEED_CONSOLE_OUTPUT 0 /* Set this if you need debug messages on the console;
                               * it will have an impact on code-size and power consumption. */

#if NEED_CONSOLE_OUTPUT
Serial  pc(USBTX, USBRX);
#define DEBUG(...) { pc.printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */

BLEDevice  ble;
DigitalOut led1(LED1);

// The Nordic UART Service
static const uint8_t uart_base_uuid[] = {0x6e, 0x40, 0x00, 0x01, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5,0x0e, 0x24, 0xdc, 0xca, 0x9e};
static const uint8_t uart_tx_uuid[]   = {0x6e, 0x40, 0x00, 0x02, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5,0x0e, 0x24, 0xdc, 0xca, 0x9e};
static const uint8_t uart_rx_uuid[]   = {0x6e, 0x40, 0x00, 0x03, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5,0x0e, 0x24, 0xdc, 0xca, 0x9e};
static const uint8_t uart_base_uuid_rev[] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e};
uint8_t txPayload[20 + 1] = {0,};
uint8_t rxPayload[20 + 1] = {0,};
GattCharacteristic  txCharacteristic (uart_tx_uuid, txPayload, 1, 20,
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
GattCharacteristic  rxCharacteristic (uart_rx_uuid, rxPayload, 1, 20,
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&txCharacteristic, &rxCharacteristic};
GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));

void disconnectionCallback(uint16_t handle)
{
    DEBUG("Disconnected!\n\r");
    DEBUG("Restarting the advertising process\n\r");
    ble.startAdvertising();
}

void onDataWritten(uint16_t charHandle)
{
    if (charHandle == txCharacteristic.getHandle()) {
        DEBUG("onDataWritten()\n\r");
        uint16_t bytesRead;
        ble.readCharacteristicValue(txCharacteristic.getHandle(), txPayload, &bytesRead);
        txPayload[bytesRead] = '\0';
        DEBUG("ECHO: %s\n\r", (char *)txPayload);
        ble.updateCharacteristicValue(rxCharacteristic.getHandle(), txPayload, bytesRead);
    }
}

void periodicCallback(void)
{
    led1 = !led1; /* Do blinky on LED1 while we're waiting for BLE events */
}

int main(void)
{
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
                                    (const uint8_t *)uart_base_uuid_rev, sizeof(uart_base_uuid));

    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.startAdvertising();

    ble.addService(uartService);

    while (true) {
        ble.waitForEvent();
    }
}
