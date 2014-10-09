

PROJECT = ble_color_pixels
MBED_BLE = ../..
OBJECTS += main.o $(MBED_BLE)/color_pixels/color_pixels.o
INCLUDE_PATHS += -I$(MBED_BLE)/color_pixels

include $(MBED_BLE)/Makefile

# optimization for time is required by ../../color_pixels/color_pixels.cpp
CC_FLAGS += -Ofast
