//Copyright 2021-2024(c) John Sullivan
//github.com/doppelhub/Honda_Insight_LiBCM

#ifndef temperature_h
    #define temperature_h

    //==================================================================
    // Thermistor Configurations
    #if defined(BATTERY_TYPE_5AhG3)
      // (5AhG3 battery modules with LiBCM)
      // 4 OEM thermistors
      //   GRN: IMA Intake air
      //   BLU: Charger
      //   YEL: IMA Exhaust air
      //   WHT: IMA Ambient
      // 3 battery thermistors: IMA battery bays 1 through 3
      #define THERM_CONFIG_5AhG3
    #elif defined(BATTERY_TYPE_47AhFoMoCo) && (! defined(BMS_TYPE_WGCLiBCM))
      // (Modified FoMoCo/Samsung SDI LX68 battery modules with LiBCM)
      // 4 OEM thermistors
      //   GRN: Top rear battery module
      //   BLU: Charger
      //   YEL: Top middle battery module
      //   WHT: IMA Intake air
      // 3 battery thermistors:
      //   "BAY1": Middle tray rail, driver side
      //   "BAY2": Bottom tray, middle
      //   "BAY3": Middle tray rail, passenger side
      #define THERM_CONFIG_LiBCM_FoMoCo
    #elif defined(BATTERY_TYPE_47AhFoMoCo) && defined(BMS_TYPE_WGCLiBCM)
      // (Stock FoMoCo/Samsung SDI LX68 battery modules with distributed MAX17xxx BMS)
      // 4 OEM thermistors
      //   GRN: IMA Intake air
      //   BLU: Charger
      //   YEL: IMA Exhaust air
      //   WHT: IMA Ambient
      // internal Samsung SDI LX68 battery module thermistors
      //   2 per module, 4 or 5 modules => 8 or 10 thermistors
      #define THERM_CONFIG_WGCLiBCM
    #else
      // not a valid configuration
      #error (Not a valid configuration of BMS type and BATTERY type)
    #endif

    int8_t temperature_battery_getLatest(void);
    int8_t temperature_intake_getLatest(void);
    int8_t temperature_gridCharger_getLatest(void);
  #if defined(THERM_CONFIG_5AhG3) || defined(THERM_CONFIG_WGCLiBCM)
    int8_t temperature_exhaust_getLatest(void);
    int8_t temperature_ambient_getLatest(void); //IMA bay temperature
  #endif
  #if defined(THERM_CONFIG_WGCLiBCM)
    #define TEMPERATURE_MAX_CELL_BALANCE_DIE_TEMP_counts 31234 // 85 degC
    uint16_t temperature_ModuleDie_getLatest_counts(uint8_t icAddress);
    void     temperature_ModuleDie_setLatest_counts(uint8_t icAddress, uint16_t temp_counts);
    uint16_t temperature_ModuleTherm_getLatest_counts(uint8_t icAddress, uint8_t thermistor);
    void     temperature_ModuleTherm_setLatest_counts(uint8_t icAddress, uint8_t thermistor, uint16_t temp_counts);
    void     temperature_ModuleTherm_setSampleTime_ms(uint32_t sampleTime_ms);
  #endif

    int8_t temperature_measureOneSensor_degC(uint8_t thermistorPin);

    void temperature_measureAndPrintAll(void);
    void temperature_printAll_latest(void);

    int8_t temperature_coolBatteryAbove_C(void);
    int8_t temperature_heatBatteryBelow_C(void);

    void temperature_handler(void);

    #define TEMPERATURE_SENSOR_FAULT_HI         99
    #define TEMPERATURE_SENSOR_FAULT_LO        -99
    #define TEMPERATURE_PACK_IN_THERMAL_RUNAWAY 70

    #define TEMPSENSORSTATE_OFF      1
    #define TEMPSENSORSTATE_TURNON   2
    #define TEMPSENSORSTATE_POWERUP  4
    #define TEMPSENSORSTATE_MEASURE  8
    #define TEMPSENSORSTATE_STAYON  16
    #define TEMPSENSORSTATE_TURNOFF 32

    #define ROOM_TEMP_DEGC     23
    #define TEMP_FREEZING_DEGC  0

  #if   defined(THERM_CONFIG_5AhG3)
    #define NUM_BATTERY_TEMP_SENSORS 3
  #elif defined(THERM_CONFIG_LiBCM_FoMoCo) || (defined(THERM_CONFIG_WGCLiBCM) && defined(WGC_BB3HW))
    // what were 2 OEM temp sensors (PIN_TEMP_GRN, PIN_TEMP_YEL) are now on battery modules
    #define NUM_BATTERY_TEMP_SENSORS 5
  #elif defined(THERM_CONFIG_WGCLiBCM)
    #define NUM_BATTERY_TEMP_SENSORS (2 * TOTAL_IC)
  #endif

    #define TEMP_POWERUP_DELAY_ms 100

    #define TEMP_UPDATE_PERIOD_KEYON_ms        (1 *  1000) //  1k per second
    #ifdef WGC_DEBUG_SPEEDUP_MODE_CHANGES
      // transition far more quickly for test purposes
      #define TEMP_UPDATE_PERIOD_KEYOFF_ms       (20000) // 20 sec instead of 60
    #else
      #define TEMP_UPDATE_PERIOD_KEYOFF_ms       (1 * 60000) // 60k per minute
    #endif
    #define TEMP_UPDATE_PERIOD_GRIDCHARGING_ms (2 *  1000) //  1k per second

#endif
