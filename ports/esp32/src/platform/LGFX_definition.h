#pragma once

#include <LovyanGFX.h>

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01  _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Light_PWM     _light_instance;
    lgfx::Touch_CST816S _touch_instance;

    public:
        LGFX(void) {
            {
                auto cfg = _bus_instance.config();
                
                cfg.spi_host = SPI2_HOST;     // Select the SPI to use  ESP32-S2,C3 : SPI2_HOST or SPI3_HOST / ESP32 : VSPI_HOST or HSPI_HOST
                cfg.spi_mode = 0;             // Set the SPI communication mode (0 ~ 3)
                cfg.freq_write = 80000000;    // SPI clock for transmission (up to 80MHz, rounded to an integer value of 80MHz)
                cfg.freq_read  = 20000000;    // SPI clock for reception
                cfg.spi_3wire  = true;        // Set to true if receiving via the MOSI pin
                cfg.use_lock   = true;        // Set to true if using transaction lock
                cfg.dma_channel = SPI_DMA_CH_AUTO; // Set the DMA channel to use (0=No DMA / 1=1ch / 2=2ch / SPI_DMA_CH_AUTO=Auto setting)
                cfg.pin_sclk = 10;            // Set the SPI SCLK pin number
                cfg.pin_mosi = 11;            // Set the SPI MOSI pin number
                cfg.pin_miso = 12;            // Set the SPI MISO pin number (-1 = disable)
                cfg.pin_dc   = 8;             // Set the SPI D/C pin number (-1 = disable)
                _bus_instance.config(cfg);    // Apply the settings to the bus
                _panel_instance.setBus(&_bus_instance);      // Set the bus to the panel
            }

            {
                auto cfg = _panel_instance.config();

                cfg.pin_cs           =    9;  // Set the CS pin number
                cfg.pin_rst          =    14; // Set the RST pin number
                cfg.pin_busy         =    -1; // Set the BUSY pin number (-1 = disable)

                cfg.panel_width      =   240; // Actual displayable width
                cfg.panel_height     =   240; // Actual displayable height
                cfg.offset_x         =     0; // X-direction offset of the panel
                cfg.offset_y         =     0; // Y-direction offset of the panel
                cfg.offset_rotation  =     0; // Offset value of the rotation direction 0~7 (4~7 are upside down)
                cfg.dummy_read_pixel =     8; // Number of dummy reads before reading pixels
                cfg.dummy_read_bits  =     1; // Number of dummy reads before reading non-pixel data
                cfg.readable         =  true; // Set to true if data reading is possible
                cfg.invert           =  true; // Set to true if the panel's brightness is inverted
                cfg.rgb_order        = false; // Set to true if the panel's red and blue are swapped
                cfg.dlen_16bit       = false; // Set to true if the panel sends data in 16-bit units for 16-bit parallel or SPI
                cfg.bus_shared       =  true; // Set to true if sharing the bus with an SD card (controls the bus in drawJpgFile, etc.)
                _panel_instance.config(cfg);
            }
            
            {
                auto cfg = _light_instance.config();

                cfg.pin_bl = 2;               // Set the backlight pin number
                cfg.invert = false;           // Set to true if the backlight is inverted
                cfg.freq   = 44100;           // Set the PWM frequency
                cfg.pwm_channel = 7;          // Set the PWM channel

                _light_instance.config(cfg);
                _panel_instance.setLight(&_light_instance);
            }
                
            {
                auto cfg = _touch_instance.config();
                cfg.x_min      = 0;           // Minimum X value
                cfg.x_max      = 239;         // Maximum X value
                cfg.y_min      = 0;           // Minimum Y value
                cfg.y_max      = 319;         // Maximum Y value
                cfg.pin_int    = 5;           // Interrupt pin number
                cfg.bus_shared = true;        // Set to true if sharing the bus
                cfg.offset_rotation = 0;      // Offset value of the rotation direction
                cfg.i2c_port = 1;             // I2C port number
                cfg.i2c_addr = 0x15;          // I2C address
                cfg.pin_sda  = 6;             // I2C SDA pin number
                cfg.pin_scl  = 7;             // I2C SCL pin number
                cfg.freq = 400000;            // I2C frequency

                _touch_instance.config(cfg);
                _panel_instance.setTouch(&_touch_instance);
            }

            setPanel(&_panel_instance);
        }
};