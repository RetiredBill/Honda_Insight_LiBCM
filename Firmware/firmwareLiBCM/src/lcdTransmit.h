//Copyright 2021-2024(c) John Sullivan
//github.com/doppelhub/Honda_Insight_LiBCM

#ifndef lcdTransmit_h
    #define lcdTransmit_h

    #define SCREEN_DIDNT_UPDATE false
    #define SCREEN_UPDATED      true

    //define screen elements
    //up to one screen element is updated each loop, using round robbin state machine
    enum enum_LCDVALUE {
         LCDVALUE_NO_UPDATE = 0,
         LCDVALUE_CALC_CYCLEFRAME,
//         LCDVALUE_SECONDS,
//         LCDVALUE_VPACK_ACTUAL,
//         LCDVALUE_VPACK_SPOOFED,
         LCDVALUE_LTC6804_ERRORS,
         LCDVALUE_CELL_HI,
         LCDVALUE_CELL_LO,
         LCDVALUE_CELL_DELTA,
         LCDVALUE_POWER,
         LCDVALUE_CELL_MAXEVER,
         LCDVALUE_CELL_MINEVER,
         LCDVALUE_SoC,
         LCDVALUE_CURRENT,
         LCDVALUE_TEMP_BATTERY,
         LCDVALUE_FAN_STATUS,
         LCDVALUE_GRID_STATUS,
         LCDVALUE_HEATER_STATUS,
         LCDVALUE_BALANCE_STATUS,
         LCDVALUE_FLASH_BACKLIGHT,
//         LCDVALUE_WATT_HOURS,
         LCDVALUE_dummy,
         LCDVALUE_MAX_VALUE = LCDVALUE_dummy - 1 //must equal the highest defined number (previous line)
    };
//    #define LCDVALUE_MAX_VALUE       20 //must equal the highest defined number (previous line)

    #define LCD_UPDATE_ATTEMPTS_PER_LOOP 5
    #define LCD_VALUE_MINIMUM_DISPLAY_TIME_LOOPS 20

    //the following static text never changes, and is only sent once each time the display turns on
    enum enum_LCDSTATIC {
        LCDSTATIC_SET_DEFAULTS = LCDVALUE_dummy, //must be LCDVALUE_MAX_VALUE+1
        LCDSTATIC_SECONDS,
        LCDSTATIC_VPACK_ACTUAL,
        LCDSTATIC_VPACK_SPOOFED,
        LCDSTATIC_CHAR_FLAGS,
        LCDSTATIC_CELL_HI,
        LCDSTATIC_CELL_LO,
        LCDSTATIC_CELL_DELTA,
        LCDSTATIC_POWER,
        LCDSTATIC_CELL_MAXEVER,
        LCDSTATIC_CELL_MINEVER,
        LCDSTATIC_SoC,
        LCDSTATIC_CURRENT,
        LCDSTATIC_TEMP_BATTERY,
        LCDSTATIC_WATT_HOURS,
        LCDSTATIC_dummy,
        LCDSTATIC_MAX_VALUE = LCDSTATIC_dummy - 1 //must equal the highest static number (previous line)
    };

    #define BACKLIGHT_FLASHING_PERIOD_ms 250

    #define CYCLEFRAME_A_PERIOD_ms  4000
    #define CYCLEFRAME_B_PERIOD_ms  4000

    #define CYCLEFRAME_INIT         0
    #define CYCLEFRAME_A            1
    #define CYCLEFRAME_B            2
    #define CYCLEFRAME_MAX_VALUE    2 //must equal the highest defined number (above)

    void lcdTransmit_begin(void);
    void lcdTransmit_end(void);

    void lcdTransmit_displayOn(void);
    void lcdTransmit_displayOff(void);

    void lcdTransmit_printNextElement(void); //primary interface //each call updates one screen element

    void lcdTransmit_splashscreenKeyOff(void);

    void lcdTransmit_testText(void);

    #define LCD_WARN_KEYON_GRID 1
    #define LCD_WARN_FW_EXPIRED 2
    #define LCD_WARN_COVER_GONE 3
    #define LCD_WARN_CELL_COUNT 4
    #define LCD_WARN_BASIC_TEST 5
    void lcdTransmit_Warning(uint8_t warningToDisplay);

#endif
