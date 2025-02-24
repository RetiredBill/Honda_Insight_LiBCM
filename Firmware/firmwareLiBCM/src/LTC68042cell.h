//Copyright 2021-2024(c) John Sullivan
//github.com/doppelhub/Honda_Insight_LiBCM

#ifndef LTC68042cell_h
    #define LTC68042cell_h

    #define LTC_STATE_FIRSTRUN 0
    #define LTC_STATE_GATHER   1
    #define LTC_STATE_PROCESS  2

    #define GATHERING_CELL_DATA 0
    #define CELL_DATA_PROCESSED 1

  #ifndef BMS_TYPE_WGCLiBCM
    #define LTC6804_MAX_CONVERSION_TIME_ms 5 //4.43 ms in '2kHz' sampling mode
  #else
    #define LTC6804_MAX_CONVERSION_TIME_ms 6 //WGCToDo: need to update this value

    // Scale factor 5v/2^^16counts = 76.3uV/bit -> 100 uV/bit
    //   => rawReadings * 5v/(2^^16counts)[V/bit] * 10000[100uV/V] = rawReadings * 0.762939
    // Note: this is just the scale factor. MAX17843 only has
    //  from 12 to 14 bits of resolution. LSBs are always 0
    #define MAX17873_CONVERSION_TO_100uV_per_bit 0.762939
  #endif

    bool LTC68042cell_nextVoltages(void);
    void LTC68042cell_acquireAllCellVoltages(void);

#endif
