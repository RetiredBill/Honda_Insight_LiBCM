//Copyright 2021-2024(c) John Sullivan
//github.com/doppelhub/Honda_Insight_LiBCM

//LTC6804 and MAX17xxx configuration header file

#ifndef LTC68042configure_h
    #define LTC68042configure_h

    #define TOTAL_IC_48S 4
    #define TOTAL_IC_60S 5

    //choose number of LTC ICs in isoSPI network
    // Note: in the MAX17xxx world
    //  the "isoSPI" network is replaced by a MAX17841/43 UART daisy chain
    //  TOTAL_IC is really the number of battery modules
    #ifdef RUN_BRINGUP_TESTER_MOTHERBOARD
        #define TOTAL_IC TOTAL_IC_60S
    #elif defined STACK_IS_48S
        #define TOTAL_IC TOTAL_IC_48S
        #ifdef STACK_IS_60S
            #error (pack is specified as both 48S and 60S. Select only one option in config.h)
        #endif
    #elif defined STACK_IS_60S
        #ifdef BATTERY_TYPE_5AhG3
            #error (incompatible config.h parameters selected: 60S not supported with 5AhG3 cells)
        #endif
        #define TOTAL_IC TOTAL_IC_60S
    #else
        #error (Select pack size - 48S or 60S - in config.h)
    #endif

  #ifndef BMS_TYPE_WGCLiBCM
    #define FIRST_IC_ADDR  2 //lowest address.  All additional IC addresses must be sequential
    #define CELLS_PER_IC  12 //Each LTC6804 measures QTY12 cells
  #else
    #define FIRST_IC_ADDR  0 //lowest address.  All additional IC addresses must be sequential
    #define CELLS_PER_IC  12 //Each MAX17843 measures QTY12 cells
    // map LiBCM "IC address" to MAX17843 device number and vice versa
    //   devNum = mapIc2Dev[icNum]
    //   icNum  = mapDev2Ic[devNum]
    #ifdef WGC_BB1HW
      // BB1: IC number is the same as device number
      //                       IC number: 0  1  2  3  4
      static const uint8_t mapIc2Dev[] = {0, 1, 2, 3, 4};
      //                      dev number: 0  1  2  3  4
      static const uint8_t mapDev2Ic[] = {0, 1, 2, 3, 4};
    #else
      // shortest UART daisy chain cable length in-car dictates different
      //   order of modules, so...
      //                       IC number: 0  1  2  3  4
      static const uint8_t mapIc2Dev[] = {2, 1, 3, 0, 4};
      //                      dev number: 0  1  2  3  4
      static const uint8_t mapDev2Ic[] = {3, 1, 0, 2, 4};
    #endif
  #endif

    //Acquisition time vs measurement accuracy trade-off
    // ACQ_REASONABLY_PRECISE_AND_FASTER for time sensitive operation (key-on)
    // ACQ_MOST_PRECISE_BUT_SLOWER for when time is less important, but better accuracy is helpful (key-off)
    #define ACQ_REASONABLY_PRECISE_AND_FASTER  0
    #define ACQ_MOST_PRECISE_BUT_SLOWER        1

    //============== LTC6804 specific defines
    #define SPECIFIED_MAX_WAKEUP_TIME_LTCCORE_MICROSECONDS 300 //guarantees LTC6804 is in 'standby' mode (tWake = 300 us max)
    #define SPECIFIED_MAX_WAKEUP_TIME_isoSPI_MICROSECONDS   10 //guarantees isoSPI is in 'ready' mode (tWAKE = 10 us max)

     // |CHG | Dec  |Channels to convert   |
     // |----|------|----------------------|
     // |000 | 0    | All GPIOS and 2nd Ref|
     // |001 | 1    | GPIO 1               |
     // |010 | 2    | GPIO 2               |
     // |011 | 3    | GPIO 3               |
     // |100 | 4    | GPIO 4               |
     // |101 | 5    | GPIO 5               |
     // |110 | 6    | Vref2                |
    #define AUX_CH_ALL 0
    #define AUX_CH_GPIO1 1
    #define AUX_CH_GPIO2 2
    #define AUX_CH_GPIO3 3
    #define AUX_CH_GPIO4 4
    #define AUX_CH_GPIO5 5
    #define AUX_CH_VREF2 6

    //JTS2doLater: Does reducing corner frequency to 26 Hz reduce assist/regen noise? //Add 214 ms wait before reading
    //ADC LPF Fcorner:       Total conversion time (QTY12 cells/IC)
    //ADCOPT(CFGR0[0] = 0)
    // MD = 01 27000 Hz        1.2 ms fast
    // MD = 10  7000 Hz        2.5 ms (default)
    // MD = 11    26 Hz      213.5 ms filtered
    //
    //ADCOPT(CFGR0[0] = 1)
    // MD = 01 14000 Hz        1.3 ms
    // MD = 10  3000 Hz        3.0 ms
    // MD = 11  2000 Hz        4.4 ms
    // |command    |  10   |   9   |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
    // |-----------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
    // |ADCV:      |   0   |   1   | MD[1] | MD[2] |   1   |   1   |  DCP  |   0   | CH[2] | CH[1] | CH[0] |
    // |ADAX:      |   1   |   0   | MD[1] | MD[2] |   1   |   1   |  DCP  |   0   | CHG[2]| CHG[1]| CHG[0]|
    #define MD_FAST 1
    #define MD_NORMAL 2
    #define MD_FILTERED 3

    // |CH | Dec  | Channels to convert |
    // |---|------|---------------------|
    // |000| 0    | All Cells           |
    // |001| 1    | Cell 1 and Cell 7   |
    // |010| 2    | Cell 2 and Cell 8   |
    // |011| 3    | Cell 3 and Cell 9   |
    // |100| 4    | Cell 4 and Cell 10  |
    // |101| 5    | Cell 5 and Cell 11  |
    // |110| 6    | Cell 6 and Cell 12  |
    #define CELL_CH_ALL 0
    #define CELL_CH_1and7 1
    #define CELL_CH_2and8 2
    #define CELL_CH_3and9 3
    #define CELL_CH_4and10 4
    #define CELL_CH_5and11 5
    #define CELL_CH_6and12 6

    //|DCP | Discharge Permitted During conversion                                |
    //|----|----------------------------------------------------------------------|
    //|0   | Discharge FETs temporarily turned off prior to each cell measurement |
    //|1   | Discharge FET states not altered                                     |
    #define DCP_DISABLED 0
    #define DCP_ENABLED 1

    #ifdef RUN_BRINGUP_TESTER_MOTHERBOARD
        #define IS_DISCHARGE_ALLOWED_DURING_CONVERSION DCP_ENABLED
    #else
        #define IS_DISCHARGE_ALLOWED_DURING_CONVERSION DCP_DISABLED
    #endif

    static const unsigned int crc15Table[256] = {
        0x0,    0xc599, 0xceab, 0xb32,  0xd8cf, 0x1d56, 0x1664, 0xd3fd,
        0xf407, 0x319e, 0x3aac, 0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa,
        0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1, 0xbbf3, 0x7e6a,
        0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d,
        0x5b2e, 0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3,
        0xaf29, 0x6ab0, 0x6182, 0xa41b, 0x77e6, 0xb27f, 0xb94d, 0x7cd4,
        0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd, 0x2544,
        0x2be,  0xc727, 0xcc15, 0x98c,  0xda71, 0x1fe8, 0x14da, 0xd143,
        0xf3c5, 0x365c, 0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038,
        0x7c2,  0xc25b, 0xc969, 0xcf0,  0xdf0d, 0x1a94, 0x11a6, 0xd43f,
        0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
        0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8,
        0xa8eb, 0x6d72, 0x6640, 0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16,
        0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba, 0x4a88, 0x8f11,
        0x57c,  0xc0e5, 0xcbd7, 0xe4e,  0xddb3, 0x182a, 0x1318, 0xd681,
        0xf17b, 0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286,
        0xa213, 0x678a, 0x6cb8, 0xa921, 0x7adc, 0xbf45, 0xb477, 0x71ee,
        0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070, 0x85e9,
        0xf84,  0xca1d, 0xc12f, 0x4b6,  0xd74b, 0x12d2, 0x19e0, 0xdc79,
        0xfb83, 0x3e1a, 0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e,
        0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59, 0x2ac0,
        0xd3a,  0xc8a3, 0xc391, 0x608,  0xd5f5, 0x106c, 0x1b5e, 0xdec7,
        0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757,
        0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350,
        0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b,
        0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c,
        0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc,
        0x846,  0xcddf, 0xc6ed, 0x374,  0xd089, 0x1510, 0x1e22, 0xdbbb,
        0xaf8,  0xcf61, 0xc453, 0x1ca,  0xd237, 0x17ae, 0x1c9c, 0xd905,
        0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02,
        0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492,
        0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
    };
    /*Code used to generate this crc15 table:
    void generate_crc15_table()
    {
        int remainder;
        for (int i = 0; i<256;i++)
        {
            remainder =  i<< 7;
            for (int bit = 8; bit > 0; --bit)
            {
                if ((remainder & 0x4000) > 0)//equivalent to remainder & 2^14 simply check for MSB
                {
                    remainder = ((remainder << 1)) ;
                    remainder = (remainder ^ 0x4599);
                } else {
                    remainder = ((remainder << 1));
                }
            }
            crc15Table[i] = remainder&0xFFFF;
        }
    }
    */

    //'DCTO' Discharge timeout values (inclusive) //see Table12
    #define LTC6804_DISCHARGE_TIMEOUT_02_SECONDS  0x00 //software timer disabled
    #define LTC6804_DISCHARGE_TIMEOUT_30_SECONDS  0x10 //0.5 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_01_MINUTE   0x20 //1 minute
    #define LTC6804_DISCHARGE_TIMEOUT_02_MINUTES  0x30 //2 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_03_MINUTES  0x40 //3 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_04_MINUTES  0x50 //4 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_05_MINUTES  0x60 //5 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_10_MINUTES  0x70 //10 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_15_MINUTES  0x80 //15 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_20_MINUTES  0x90 //20 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_30_MINUTES  0xA0 //30 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_40_MINUTES  0xB0 //40 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_60_MINUTES  0xC0 //60 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_75_MINUTES  0xD0 //75 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_90_MINUTES  0xE0 //90 minutes
    #define LTC6804_DISCHARGE_TIMEOUT_120_MINUTES 0xF0 //120 minutes

    #define LTC6804_CORE_ALREADY_AWAKE true
    #define LTC6804_CORE_JUST_WOKE_UP  false

    #define BROADCAST_TO_ALL_ICS 16 //valid LTC6804 addresses are 0:15

    #define LTC6804_MASK_REFON_BIT 0x02

    //============== end of LTC6804 specific defines, start of MAX17841

    // MAX17841 RX_INTERRUPT_ENABLE register bit value config defaults. "0b1" enables, "0b0" disables
    #define M871_RX_INTEN_INIT_RX_ERROR_INT_ENABLE    0b1 // Interrupt enable for RX_Error_Status
    #define M871_RX_INTEN_INIT_RX_BUSY_INT_ENABLE     0b0 // Interrupt enable for RX_Busy_Status
    #define M871_RX_INTEN_INIT_RX_IDLE_INT_ENABLE     0b0 // Interrupt enable for RX_Idle_Status
    #define M871_RX_INTEN_INIT_RX_OVERFLOW_INT_ENABLE 0b1 // Interrupt enable for RX_Overflow_Status
    #define M871_RX_INTEN_INIT_RX_FULL_INT_ENABLE     0b0 // Interrupt enable for RX_Full_Status
    #define M871_RX_INTEN_INIT_RX_STOP_INT_ENABLE     0b0 // Interrupt enable for RX_Stop_Status
    #define M871_RX_INTEN_INIT_RX_EMPTY_INT_ENABLE    0b0 // Interrupt enable for RX_Empty_Status

    #define M871_RX_INTERRUPT_ENABLE_INIT \
      (M871_RX_INTEN_INIT_RX_EMPTY_INT_ENABLE    << M871_RX_INTEN_RX_Empty_INT_Enable   ) + \
      (M871_RX_INTEN_INIT_RX_STOP_INT_ENABLE     << M871_RX_INTEN_RX_Stop_INT_Enable    ) + \
      (M871_RX_INTEN_INIT_RX_FULL_INT_ENABLE     << M871_RX_INTEN_RX_Full_INT_Enable    ) + \
      (M871_RX_INTEN_INIT_RX_OVERFLOW_INT_ENABLE << M871_RX_INTEN_RX_Overflow_INT_Enable) + \
      (M871_RX_INTEN_INIT_RX_IDLE_INT_ENABLE     << M871_RX_INTEN_RX_Idle_INT_Enable    ) + \
      (M871_RX_INTEN_INIT_RX_BUSY_INT_ENABLE     << M871_RX_INTEN_RX_Busy_INT_Enable    ) + \
      (M871_RX_INTEN_INIT_RX_ERROR_INT_ENABLE    << M871_RX_INTEN_RX_Error_INT_Enable   )

    // MAX17843 STATUS register bit value config defaults. "0b1" enables, "0b0" disables
    #define M873_STATUS_INIT_ALRTRST             0b0
    #define M873_STATUS_INIT_ALRTOV              0b0
    #define M873_STATUS_INIT_ALRTUV              0b0
    #define M873_STATUS_INIT_ALRTSHDNL           0b0
    #define M873_STATUS_INIT_ALRTSHDNLRT         0b0
    #define M873_STATUS_INIT_ALRTMSMTCH          0b0
    #define M873_STATUS_INIT_ALRTTCOLD           0b0
    #define M873_STATUS_INIT_ALRTTHOT            0b0
    #define M873_STATUS_INIT_ALRTPEC             0b0
    #define M873_STATUS_INIT_ALRTMAN             0b0
    #define M873_STATUS_INIT_ALRTPAR             0b0
    #define M873_STATUS_INIT_ALRTFMEA2           0b0
    #define M873_STATUS_INIT_ALRTFMEA1           0b0

    #define M873_STATUS_INIT \
      (M873_STATUS_INIT_ALRTFMEA1   << M873_STATUS_ALRTFMEA1  ) + \
      (M873_STATUS_INIT_ALRTFMEA2   << M873_STATUS_ALRTFMEA2  ) + \
      (M873_STATUS_INIT_ALRTPAR     << M873_STATUS_ALRTPAR    ) + \
      (M873_STATUS_INIT_ALRTMAN     << M873_STATUS_ALRTMAN    ) + \
      (M873_STATUS_INIT_ALRTPEC     << M873_STATUS_ALRTPEC    ) + \
      (M873_STATUS_INIT_ALRTTHOT    << M873_STATUS_ALRTTHOT   ) + \
      (M873_STATUS_INIT_ALRTTCOLD   << M873_STATUS_ALRTTCOLD  ) + \
      (M873_STATUS_INIT_ALRTMSMTCH  << M873_STATUS_ALRTMSMTCH ) + \
      (M873_STATUS_INIT_ALRTSHDNLRT << M873_STATUS_ALRTSHDNLRT) + \
      (M873_STATUS_INIT_ALRTSHDNL   << M873_STATUS_ALRTSHDNL  ) + \
      (M873_STATUS_INIT_ALRTUV      << M873_STATUS_ALRTUV     ) + \
      (M873_STATUS_INIT_ALRTOV      << M873_STATUS_ALRTOV     ) + \
      (M873_STATUS_INIT_ALRTRST     << M873_STATUS_ALRTRST    )

    // MAX17843 DEVCFG1 register bit value config defaults. "0b1" enables, "0b0" disables
    #define M873_DEVCFG1_INIT_POLARITY            0b0     //bit 15
    #define M873_DEVCFG1_INIT_ADCSELECT           0b0     //bit 14
    #define M873_DEVCFG1_INIT_FC                  0b010   //bit 13:11
    #define M873_DEVCFG1_INIT_EMGCYDCHG           0b0     //bit 10
    #define M873_DEVCFG1_INIT_HVCPDIS             0b0     //bit 9
    // bit 8 is reserved
    #define M873_DEVCFG1_INIT_FORCEPOR            0b0     //bit 7
    #define M873_DEVCFG1_INIT_ALIVECNTEN          0b1     //bit 6
    #define M873_DEVCFG1_INIT_ADCTSTEN            0b0     //bit 5
    #define M873_DEVCFG1_INIT_SCANTODIS           0b0     //bit 4
    #define M873_DEVCFG1_INIT_BALSWDISABLE        0b1     //bit 3
    #define M873_DEVCFG1_INIT_NOPEC               0b0     //bit 2
    #define M873_DEVCFG1_INIT_ADDRUNLOCK          0b0     //bit 1
    #define M873_DEVCFG1_INIT_SPOR                0b0     //bit 0

    #define M873_DEVCFG1_INIT \
      (M873_DEVCFG1_INIT_SPOR         << M873_DEVCFG1_SPOR        ) + \
      (M873_DEVCFG1_INIT_ADDRUNLOCK   << M873_DEVCFG1_ADDRUNLOCK  ) + \
      (M873_DEVCFG1_INIT_NOPEC        << M873_DEVCFG1_NOPEC       ) + \
      (M873_DEVCFG1_INIT_BALSWDISABLE << M873_DEVCFG1_BALSWDISABLE) + \
      (M873_DEVCFG1_INIT_SCANTODIS    << M873_DEVCFG1_SCANTODIS   ) + \
      (M873_DEVCFG1_INIT_ADCTSTEN     << M873_DEVCFG1_ADCTSTEN    ) + \
      (M873_DEVCFG1_INIT_ALIVECNTEN   << M873_DEVCFG1_ALIVECNTEN  ) + \
      (M873_DEVCFG1_INIT_FORCEPOR     << M873_DEVCFG1_FORCEPOR    ) + \
      (M873_DEVCFG1_INIT_HVCPDIS      << M873_DEVCFG1_HVCPDIS     ) + \
      (M873_DEVCFG1_INIT_EMGCYDCHG    << M873_DEVCFG1_EMGCYDCHG   ) + \
      (M873_DEVCFG1_INIT_FC           << M873_DEVCFG1_bfFC_SHIFT  ) + \
      (M873_DEVCFG1_INIT_ADCSELECT    << M873_DEVCFG1_ADCSELECT   ) + \
      (M873_DEVCFG1_INIT_POLARITY     << M873_DEVCFG1_POLARITY    )

    // MAX17843 MEASUREEN register bit value config defaults. "0b1" enables, "0b0" disables
    #define M873_MEASUREEN_INIT_BLKCONNECT          0b1
    #define M873_MEASUREEN_INIT_BLOCKEN             0b1   // Block Voltage Measurement enable
    #define M873_MEASUREEN_INIT_AIN2EN              0b1
    #define M873_MEASUREEN_INIT_AIN1EN              0b1
    #define M873_MEASUREEN_INIT_CELLEN              0b111111111111   // Cell Voltage measurement enable

    #define M873_MEASUREEN_INIT \
      (M873_MEASUREEN_INIT_CELLEN     << M873_MEASUREEN_bfCELLEN_SHIFT) + \
      (M873_MEASUREEN_INIT_AIN1EN     << M873_MEASUREEN_AIN1EN        ) + \
      (M873_MEASUREEN_INIT_AIN2EN     << M873_MEASUREEN_AIN2EN        ) + \
      (M873_MEASUREEN_INIT_BLOCKEN    << M873_MEASUREEN_BLOCKEN       ) + \
      (M873_MEASUREEN_INIT_BLKCONNECT << M873_MEASUREEN_BLKCONNECT    )

    // MAX17843 SCANCTRL register bit value config defaults. "0b1" enables, "0b0" disables
    //   M873_SCANCTRL_INIT_OVSAMPL is adjusted for Acquisition time vs measurement accuracy trade-off
    #define M873_SCANCTRL_INIT_SCANDONE            0b0
    #define M873_SCANCTRL_INIT_SCANTIMEOUT         0b0
    #define M873_SCANCTRL_INIT_DATARDY             0b0
    #define M873_SCANCTRL_INIT_DELAYSEL            0b0
    #define M873_SCANCTRL_INIT_AUTOBALSWDIS        0b0
    #define M873_SCANCTRL_INIT_BALSWDIAG           0b000
    #define M873_SCANCTRL_INIT_RDFILT              0b0
    #define M873_SCANCTRL_INIT_OVSAMPL             0b000
    #define M873_SCANCTRL_INIT_AMENDFILT           0b1
    #define M873_SCANCTRL_INIT_SCANMODE            0b0
    #define M873_SCANCTRL_INIT_FILTDONE            0b0
    #define M873_SCANCTRL_INIT_SCAN                0b0

    #define M873_SCANCTRL_INIT \
      (M873_SCANCTRL_INIT_SCAN          << M873_SCANCTRL_SCAN             ) + \
      (M873_SCANCTRL_INIT_FILTDONE      << M873_SCANCTRL_FILTDONE         ) + \
      (M873_SCANCTRL_INIT_SCANMODE      << M873_SCANCTRL_SCANMODE         ) + \
      (M873_SCANCTRL_INIT_AMENDFILT     << M873_SCANCTRL_AMENDFILT        ) + \
      (M873_SCANCTRL_INIT_OVSAMPL       << M873_SCANCTRL_bfOVSAMPL_SHIFT  ) + \
      (M873_SCANCTRL_INIT_RDFILT        << M873_SCANCTRL_RDFILT           ) + \
      (M873_SCANCTRL_INIT_BALSWDIAG     << M873_SCANCTRL_bfBALSWDIAG_SHIFT) + \
      (M873_SCANCTRL_INIT_AUTOBALSWDIS  << M873_SCANCTRL_AUTOBALSWDIS     ) + \
      (M873_SCANCTRL_INIT_DELAYSEL      << M873_SCANCTRL_DELAYSEL         ) + \
      (M873_SCANCTRL_INIT_DATARDY       << M873_SCANCTRL_DATARDY          ) + \
      (M873_SCANCTRL_INIT_SCANTIMEOUT   << M873_SCANCTRL_SCANTIMEOUT      ) + \
      (M873_SCANCTRL_INIT_SCANDONE      << M873_SCANCTRL_SCANDONE         )

   // MAX17843 ACQCFG register bit value config defaults. "0b1" enables, "0b0" disables
    #define M873_ACQCFG_INIT_THRMMODE            0b11
    #define M873_ACQCFG_INIT_AINTIME             0b111111

    #define M873_ACQCFG_INIT \
      (M873_ACQCFG_INIT_AINTIME  << M873_ACQCFG_bfAINTIME_SHIFT ) + \
      (M873_ACQCFG_INIT_THRMMODE << M873_ACQCFG_bfTHRMMODE_SHIFT)

    // MAX17843 DIAGCFG register bit value config defaults
    #define M873_DIAGCFG_INIT_bfCTSTDAC    0b0000
    #define M873_DIAGCFG_INIT_CTSTSRC         0b0
    #define M873_DIAGCFG_INIT_bfAUXINTSTEN    0b0
    #define M873_DIAGCFG_INIT_MUXDIAGBUS      0b0
    #define M873_DIAGCFG_INIT_MUXDIAGPAIR     0b0
    #define M873_DIAGCFG_INIT_MUXDIAGEN       0b0
    #define M873_DIAGCFG_INIT_ALTMUXSEL       0b0
    #define M873_DIAGCFG_INIT_bfDIAGSEL   M873_DIAGSEL_DieTemperature

    #define M873_DIAGCFG_INIT \
      (M873_DIAGCFG_INIT_bfCTSTDAC     << M873_DIAGCFG_bfCTSTDAC_SHIFT   ) + \
      (M873_DIAGCFG_INIT_CTSTSRC       << M873_DIAGCFG_CTSTSRC           ) + \
      (M873_DIAGCFG_INIT_bfAUXINTSTEN  << M873_DIAGCFG_bfAUXINTSTEN_SHIFT) + \
      (M873_DIAGCFG_INIT_MUXDIAGBUS    << M873_DIAGCFG_MUXDIAGBUS        ) + \
      (M873_DIAGCFG_INIT_MUXDIAGPAIR   << M873_DIAGCFG_MUXDIAGPAIR       ) + \
      (M873_DIAGCFG_INIT_MUXDIAGEN     << M873_DIAGCFG_MUXDIAGEN         ) + \
      (M873_DIAGCFG_INIT_ALTMUXSEL     << M873_DIAGCFG_ALTMUXSEL         ) + \
      (M873_DIAGCFG_INIT_bfDIAGSEL     << M873_DIAGCFG_bfDIAGSEL_SHIFT   )

    // MAX17843 ADR register bit value config defaults
    #define M873_ADR_INIT_bfDIAG_RECOVERY_TIME 0x00
    #define M873_ADR_INIT_bfCELL_RECOVERY_TIME 0x00

    #define M873_ADR_INIT \
      (M873_ADR_INIT_bfDIAG_RECOVERY_TIME << M873_ADR_bfDIAG_RECOVERY_TIME_SHIFT) + \
      (M873_ADR_INIT_bfCELL_RECOVERY_TIME << M873_ADR_bfCELL_RECOVERY_TIME_SHIFT)

    static const uint8_t crc8Table[256] = {
      0x00, 0x3e, 0x7c, 0x42, 0xf8, 0xc6, 0x84, 0xba,
      0x95, 0xab, 0xe9, 0xd7, 0x6d, 0x53, 0x11, 0x2f,
      0x4f, 0x71, 0x33, 0x0d, 0xb7, 0x89, 0xcb, 0xf5,
      0xda, 0xe4, 0xa6, 0x98, 0x22, 0x1c, 0x5e, 0x60,
      0x9e, 0xa0, 0xe2, 0xdc, 0x66, 0x58, 0x1a, 0x24,
      0x0b, 0x35, 0x77, 0x49, 0xf3, 0xcd, 0x8f, 0xb1,
      0xd1, 0xef, 0xad, 0x93, 0x29, 0x17, 0x55, 0x6b,
      0x44, 0x7a, 0x38, 0x06, 0xbc, 0x82, 0xc0, 0xfe,
      0x59, 0x67, 0x25, 0x1b, 0xa1, 0x9f, 0xdd, 0xe3,
      0xcc, 0xf2, 0xb0, 0x8e, 0x34, 0x0a, 0x48, 0x76,
      0x16, 0x28, 0x6a, 0x54, 0xee, 0xd0, 0x92, 0xac,
      0x83, 0xbd, 0xff, 0xc1, 0x7b, 0x45, 0x07, 0x39,
      0xc7, 0xf9, 0xbb, 0x85, 0x3f, 0x01, 0x43, 0x7d,
      0x52, 0x6c, 0x2e, 0x10, 0xaa, 0x94, 0xd6, 0xe8,
      0x88, 0xb6, 0xf4, 0xca, 0x70, 0x4e, 0x0c, 0x32,
      0x1d, 0x23, 0x61, 0x5f, 0xe5, 0xdb, 0x99, 0xa7,
      0xb2, 0x8c, 0xce, 0xf0, 0x4a, 0x74, 0x36, 0x08,
      0x27, 0x19, 0x5b, 0x65, 0xdf, 0xe1, 0xa3, 0x9d,
      0xfd, 0xc3, 0x81, 0xbf, 0x05, 0x3b, 0x79, 0x47,
      0x68, 0x56, 0x14, 0x2a, 0x90, 0xae, 0xec, 0xd2,
      0x2c, 0x12, 0x50, 0x6e, 0xd4, 0xea, 0xa8, 0x96,
      0xb9, 0x87, 0xc5, 0xfb, 0x41, 0x7f, 0x3d, 0x03,
      0x63, 0x5d, 0x1f, 0x21, 0x9b, 0xa5, 0xe7, 0xd9,
      0xf6, 0xc8, 0x8a, 0xb4, 0x0e, 0x30, 0x72, 0x4c,
      0xeb, 0xd5, 0x97, 0xa9, 0x13, 0x2d, 0x6f, 0x51,
      0x7e, 0x40, 0x02, 0x3c, 0x86, 0xb8, 0xfa, 0xc4,
      0xa4, 0x9a, 0xd8, 0xe6, 0x5c, 0x62, 0x20, 0x1e,
      0x31, 0x0f, 0x4d, 0x73, 0xc9, 0xf7, 0xb5, 0x8b,
      0x75, 0x4b, 0x09, 0x37, 0x8d, 0xb3, 0xf1, 0xcf,
      0xe0, 0xde, 0x9c, 0xa2, 0x18, 0x26, 0x64, 0x5a,
      0x3a, 0x04, 0x46, 0x78, 0xc2, 0xfc, 0xbe, 0x80,
      0xaf, 0x91, 0xd3, 0xed, 0x57, 0x69, 0x2b, 0x15
    };
      /* code used to generate crc8 table:
      void generate_crc8_table()
      {
        int poly = 0b10110010; // PEC/CRC polynomial
        for (int q = 0; q <= 255; q++)
        {
          int remainder = q;
          for (int i = 0; i < 8; i++)
          {
            int bitStatus = remainder & 1;
            if (bitStatus == 1)
            {
              remainder = remainder >> 1;
              remainder = remainder ^ poly;
            } else
            {
              remainder = remainder >> 1;
            }
          }
          crc8Table[q] = remainder;
        }
      }
      */
  #endif

    //======================= Basic cell discharge circuit test
    // This quick test first aims to detect open BMS sense wires by
    //   verifying that cell voltages measure within sane limits while
    //   the cell balance circuits are active.
    // Sane cell voltage high limit (while cell balance circuit is active):
    //   a measured cell voltage above this value is "not sane", and
    //   probably indicates an open BMS sense wire
    #define TESTBASIC_SANE_HIGH_TESTLIMIT_counts   CELL_VMAX_REGEN
    // Sane cell voltage low limit (while cell balance circuit is active):
    //   a measured cell voltage below this value is "not sane", and
    //   probably (also) indicates an open BMS sense wire
    #define TESTBASIC_SANE_LOW_TESTLIMIT_counts    CELL_VMIN_GRIDCHARGER
    // Next, the test attempts verify that the cell discharge circuits
    //   actually draw current by measuring the IR drop in the BMS sense
    //   wires and connections.
    // Cells with a voltage delta below this limit either aren't drawing
    //   current or are always drawing current. Either way there is a
    //   BMS discharge circuit failure.
    //   Another possibility is that they have anomalously low
    //   cable/wire/connection resistance (deemed unlikely).
    // cell voltage delta limit:
  #ifdef BMS_TYPE_LiBCM
    #define TESTBASIC_DELTA_TESTLIMIT_counts  30 //WGCToDoNext: this limit surely needs refinement, and likely won't work in BMS_TYPE_WGCLiBCM at all
  #else
    #define TESTBASIC_DELTA_TESTLIMIT_counts  12 //WGCToDoNext: this limit surely needs refinement
  #endif

    //======================= testDischargeFETs defines
    #define TESTDISCHASRGE_EvenCellsBitMap 0b0000010101010101
    #define TESTDISCHASRGE_OddCellsBitMap  0b0000101010101010
    #define TESTDISCHASRGE_DeltaVSep_THRESHOLD_counts   80
    #define TESTDISCHASRGE_DISCHARGE_TESTLIMIT_counts   50
    #define TESTDISCHASRGE_NONDISCHRGE_TESTLIMIT_counts 50
    #define TESTDISCHASRGE_TIMELIMIT_minutes  10
    #define TESTDISCHASRGE_TIMELIMIT_ms       (TESTDISCHASRGE_TIMELIMIT_minutes * 60 * 1000L)

    // testDischargeFETs state machine states
    #define TESTDISCHASRGESTATE_TURNON         1
    #define TESTDISCHASRGESTATE_WAITING_EVEN   2
    #define TESTDISCHASRGESTATE_TESTING_EVEN   3
    #define TESTDISCHASRGESTATE_DONE_EVEN      4
    #define TESTDISCHASRGESTATE_WAITING_ODD    5
    #define TESTDISCHASRGESTATE_TESTING_ODD    6
    #define TESTDISCHASRGESTATE_DONE           7
    #define TESTDISCHASRGESTATE_DISABLED       8

    //Exteral (public) functions (aka LTC68042 API)
    void LTC68042configure_initialize(void);
    void LTC68042configure_handleKeyStateChange(void);
    bool LTC68042configure_wakeup(void);
    uint16_t LTC68042configure_calcPEC15(uint8_t len, uint8_t const data[]);
    void LTC68042configure_spiWrite(uint8_t length, uint8_t const data[]);
    void LTC68042configure_spiWriteRead(uint8_t *TxData, uint8_t TXlen, uint8_t *rx_data, uint8_t RXlen);
    void LTC68042configure_programVolatileDefaults(void);
    void LTC68042configure_setBalanceResistors(uint8_t icAddress, uint16_t cellBitmap, uint8_t softwareTimeout);
    bool LTC68042configure_doesActualPackSizeMatchUserConfig(void);
    void LTC68042configure_pulseChipSelectLow(uint16_t lowPulsePeriod_us);
    bool LTC68042configure_acqusitionPrecision_get(void);
    void LTC68042configure_acqusitionPrecision_set(bool acqAccuracyTradeoff);
    void MAX17841configure_enableMAX17841(void);
    void MAX17841configure_disableMAX17841(void);
    void LTC68042configure_enabletestDischargeFETs(void);
    uint8_t LTC68042configure_testDischargeFETs(void);
    bool LTC68042configure_basicConfidenceTest(void);
    uint16_t MAX17841configure_calcAcquisitionTime_us(
      uint8_t NumCells, bool Ain1En, bool Ain2En, uint8_t AinTime_counts, bool VblkEn, uint8_t DiagSel, uint8_t OvrSmpls,
      bool AutoBalSwDisEn, uint8_t CellRecoveryTime_counts);
