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

#include <stdlib.h>
#include "mbed.h"
#include "BLEDevice.h"
#include "UARTService.h"
#include "DFUService.h"
#include "nrf_delay.h"


#define TICK_PERIOD_MS  1000    // ms

#ifndef GROVE_NODE_BOARD
#define BUTTON_DOWN     1
#define BUTTON_PIN      p5
#define LED_ON          1
#define LED_OFF         0
#define LED_BLUE        p30

Serial  pc(p8, p7);
#define DEBUG(...)      { pc.printf(__VA_ARGS__); }
#else
#define BUTTON_DOWN     0
#define BUTTON_PIN      p30
#define LED_ON          0
#define LED_OFF         1
#define LED_BLUE        p18

#define DEBUG(...)      /* nothing */
#endif

#define BLE_NAME        "Grove - NODE"


BLEDevice   ble;
UARTService *uartServicePtr;
DigitalOut green(p17);
DigitalOut blue(LED_BLUE);
InterruptIn button(BUTTON_PIN);

DigitalOut actuator(p1);
AnalogIn   sensor(p3);

volatile float current_input = 0;
volatile float double_click_input = 0.6;
volatile float single_click_input = 0.4;
volatile bool  threshold_update = true;
volatile float input_threshold = 0.5;
volatile int   current_compare = 0;
volatile bool  invert_output = false;
volatile int   current_output = 0;
volatile bool  force_update = false;

volatile bool  button_event = false;

volatile uint32_t blue_led_time_to_off = 0;
volatile bool     blue_led_blink = false;

volatile uint32_t green_led_time_to_off = 0;
volatile bool     green_led_blink = false;

static const uint8_t SIZEOF_TX_RX_BUFFER = 32;
uint8_t rxPayload[SIZEOF_TX_RX_BUFFER] = {0,};
uint8_t txPayload[SIZEOF_TX_RX_BUFFER] = {0,};

void connectionCallback(Gap::Handle_t handle, const Gap::ConnectionParams_t *params)
{
    DEBUG("Connected!\n\r");
    threshold_update = true;
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
        if (bytesRead < sizeof(rxPayload)) {
            memcpy(rxPayload, params->data, bytesRead);
            rxPayload[bytesRead] = '\0';
        }

        char *num;
        long threshold = strtol((char*)&rxPayload[2], &num, 10);

        if (threshold >= 0 && threshold <= 10000) {
            if ('>' == rxPayload[0]) {
                invert_output = false;
                input_threshold = (float) threshold / 10000.0;
            } else if ('<' == rxPayload[0]) {
                invert_output = true;
                input_threshold = (float) threshold / 10000.0;
            }

            force_update = true;
        }
    }
}

void button_wakeup()
{
    button_event = true;
}

void tick(void)
{
    bool send_threshold = threshold_update;
    int last_compare = current_compare;
    current_input = sensor.read();

    if (threshold_update) {
        threshold_update = 0;

        input_threshold = (single_click_input + double_click_input) / 2.0;
        invert_output = (double_click_input < single_click_input);

        force_update = true;

        DEBUG("threshold: %f\n\r", input_threshold);
    }


    if (0 == current_compare && current_input > input_threshold) {
        float extra_input = sensor.read();
        if (extra_input > input_threshold) {
            current_compare = 1;
        }
    } else if (1 == current_compare && current_input < input_threshold) {
        float extra_input = sensor.read();
        if (extra_input < input_threshold) {
            current_compare = 0;
        }
    }

    current_output = current_compare;
    if ((last_compare != current_compare) || force_update) {
        if (invert_output) {
            current_output = 1 - current_compare;
        }

        actuator = current_output;
    }

    DEBUG("in: %f, out: %d, invert: %d\n\r", current_input, current_output, invert_output);

    int txPayloadLength;
    if (send_threshold) {
        txPayloadLength = snprintf((char*)txPayload, 20, "%c %d", invert_output ? '<' : '>', (int)(input_threshold * 10000));
    } else {
        txPayloadLength = snprintf((char*)txPayload, 20, "* %d %d", (int)(current_input * 10000), current_output);

    }

    ble.updateCharacteristicValue(uartServicePtr->getRXCharacteristicHandle(), txPayload, txPayloadLength);

    if (blue_led_time_to_off) {
        blue_led_time_to_off--;
        if (blue_led_blink) {
            blue = (blue_led_time_to_off & 1) ? LED_ON : LED_OFF;
        }

        if (0 == blue_led_time_to_off) {
            blue_led_blink = false;
            blue = LED_OFF;
        }
    }

    if (green_led_time_to_off) {
        green_led_time_to_off--;
        if (green_led_blink) {
            green = (green_led_time_to_off & 1) ? LED_ON : LED_OFF;
        }

        if (0 == green_led_time_to_off) {
            green_led_blink = false;
            green = LED_OFF;
        }
    }
}

int button_detect(void)
{
    int t = 0;

    while (1) {
        if (button.read() != BUTTON_DOWN) {
            if (t < 30) {
                return 0;     // for anti shake
            } else {
                break;
            }
        }

        if (t > 30000) {        // More than 3 seconds
            return -1;          // long click
        }

        t++;
        nrf_delay_us(100);
    }

    if (t > 4000) {             // More than 0.4 seconds
        return 1;               // single click
    }

    while (true) {
        if (button.read() == BUTTON_DOWN) {
            nrf_delay_us(1000);
            if (button.read() == BUTTON_DOWN) {
                return 2;      // double click
            }

            t += 10;
        }

        if (t > 4000) {
            return 1;          // The interval of double click should less than 0.4 seconds, so it's single click
        }

        t++;
        nrf_delay_us(100);
    }
}

extern "C" void power_down();

int main(void)
{
    blue  = LED_ON;
    green = LED_OFF;

#if BUTTON_DOWN
    button.mode(PullDown);
    button.rise(button_wakeup);
#else
    button.mode(PullUp);
    button.fall(button_wakeup);
#endif

    DEBUG("Initialising the nRF51822\n\r");
    ble.init();
    ble.onConnection(connectionCallback);
    ble.onDisconnection(disconnectionCallback);
    ble.onDataWritten(onDataWritten);

    /* setup advertising */
    ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                    (const uint8_t *)BLE_NAME, sizeof(BLE_NAME));

    ble.setAdvertisingInterval(160); /* 100ms; in multiples of 0.625ms. */
    ble.startAdvertising();

    /* Enable over-the-air firmware updates. Instantiating DFUSservice introduces a
     * control characteristic which can be used to trigger the application to
     * handover control to a resident bootloader. */
    DFUService dfu(ble);

    UARTService uartService(ble);
    uartServicePtr = &uartService;

    blue_led_time_to_off = 3000 / TICK_PERIOD_MS;

    Ticker ticker;
    ticker.attach_us(tick, TICK_PERIOD_MS * 1000);

    while (true) {
        if (button_event) {
            int click;

            blue = LED_ON;
            click = button_detect();
            blue = LED_OFF;
            DEBUG("click type: %d\n\r", click);

            button_event = false;

            if (1 == click) {
                single_click_input = current_input;
            } else if (2 == click) {
                double_click_input = current_input;
                green = LED_ON;
                green_led_time_to_off = 1000 / TICK_PERIOD_MS;
            } else if (-1 == click) {
                while (BUTTON_DOWN == button.read()) {

                }
                nrf_delay_us(3000);

                power_down();
            } else {
                continue;
            }

            DEBUG("typical input: %f, %f\n\r", single_click_input, double_click_input);

            threshold_update = 1;
        } else {
            ble.waitForEvent();
        }
    }
}
