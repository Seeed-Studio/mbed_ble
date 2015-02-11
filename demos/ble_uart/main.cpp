/* BLE Color Pixels
 */

#include "mbed.h"
#include "BLEDevice.h"
#include "Buffer.h"

#define DEBUG(args...)       //uart.printf(args)

Buffer <char> uartRxBuffer(0x100);

Serial uart(p8, p7);
BLEDevice  ble;

uint8_t bleIsConnected = 0;
uint8_t bleTxFlag = 0;

// The Nordic UART Service
const uint8_t uart_base_uuid[]     = {0x6e, 0x40, 0x00, 0x01, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e};
const uint8_t uart_tx_uuid[]       = {0x6e, 0x40, 0x00, 0x02, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e};
const uint8_t uart_rx_uuid[]       = {0x6e, 0x40, 0x00, 0x03, 0xb5, 0xa3, 0xf3, 0x93, 0xe0, 0xa9, 0xe5, 0x0e, 0x24, 0xdc, 0xca, 0x9e};
const uint8_t uart_base_uuid_rev[] = {0x9e, 0xca, 0xdc, 0x24, 0x0e, 0xe5, 0xa9, 0xe0, 0x93, 0xf3, 0xa3, 0xb5, 0x01, 0x00, 0x40, 0x6e};

bool rxPayloadUpdated = false;
uint8_t rxPayload[20 + 1] = {0,};
uint8_t txPayload[20 + 1] = {0,};
GattCharacteristic  rxCharacteristic (uart_tx_uuid, rxPayload, 1, sizeof(rxPayload),
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE);
GattCharacteristic  txCharacteristic (uart_rx_uuid, txPayload, 1, sizeof(txPayload),
                                      GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic *uartChars[] = {&rxCharacteristic, &txCharacteristic};
GattService         uartService(uart_base_uuid, uartChars, sizeof(uartChars) / sizeof(GattCharacteristic *));

void uart2ble(void)
{
    uint16_t bytesToWrite = 0;
    for (int i = 0; i < 20; i++) {
        if (uartRxBuffer.available()) {
            txPayload[bytesToWrite] = uartRxBuffer;
            bytesToWrite++;
        }
    }

    if (bytesToWrite != 0) {
        bleTxFlag = 1;

        ble.updateCharacteristicValue(txCharacteristic.getHandle(), txPayload, bytesToWrite);
    } else {
        bleTxFlag = 0;
    }
}

void resetToRunBootloader()
{
    uint32_t *flag_address = (uint32_t *)0x20003FFC;
    *flag_address = 0xDECAF;
    NVIC_SystemReset();
}

void connectionCallback()
{
    DEBUG("Connected\r\n");
    bleIsConnected = 1;
    bleTxFlag = 0;
}

void disconnectionCallback()
{
    DEBUG("Disconnected!\r\n");
    DEBUG("Restarting the advertising process\r\n");

    bleTxFlag = 0;
    bleIsConnected = 0;

    ble.startAdvertising();
}

void onDataWritten(uint16_t charHandle)
{
    if (charHandle == rxCharacteristic.getHandle()) {
        DEBUG("onDataWritten()\r\n");
        uint16_t bytesRead;
        ble.readCharacteristicValue(rxCharacteristic.getHandle(), rxPayload, &bytesRead);

        for (int i = 0; i < bytesRead; i++) {
            uart.putc(rxPayload[i]);
        }

        // memcpy(txPayload, rxPayload, bytesRead);
        // ble.updateCharacteristicValue(txCharacteristic.getHandle(), txPayload, bytesRead);
    }
}

void onDataSent()
{
    DEBUG("onDataSent\r\n");

    uart2ble();
}

int main(void)
{
    uart.baud(115200);

    DEBUG("BLE UART\r\n");

    ble.init();
    ble.onConnection(connectionCallback);
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);
    ble.onDataSent(onDataSent);

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
        uint32_t timeout = 1000;
        while (timeout) {
            timeout--;
            if (uart.readable()) {
                uartRxBuffer.put((char)uart.getc());
                timeout = 1000;
            }
        }
        
        if (bleIsConnected && bleTxFlag == 0 && uartRxBuffer.available()) {
            uart2ble();
            bleTxFlag = 1;
        }
    }
}
