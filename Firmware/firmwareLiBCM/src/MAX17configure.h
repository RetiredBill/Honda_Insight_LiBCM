// MAX17xxx configuration functions
// Based on LTC68042configure.h, Copyright 2021-2024(c) John Sullivan
//    github.com/doppelhub/Honda_Insight_LiBCM

// MAX17xxx configuration header file

#ifndef MAX17configure_h
    #define MAX17configure_h

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

    #define FIRST_IC_ADDR  0 //lowest address.  All additional IC addresses must be sequential
    #define CELLS_PER_IC  12 //Each MAX17843 measures QTY12 cells

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
    #define M873_DEVCFG1_INIT_BALSWDISABLE        0b0     //bit 3
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

    //WGCToDo: LTC defines just here for info. Remove at some point...
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

//    #define LTC6804_CORE_ALREADY_AWAKE true
//    #define LTC6804_CORE_JUST_WOKE_UP  false

//    #define BROADCAST_TO_ALL_ICS 16 //valid LTC6804 addresses are 0:15

//    #define LTC6804_MASK_REFON_BIT 0x02
    //WGCToDo: end of LTC defines just here for info. Remove at some point...

    #define LTC6804_CORE_ALREADY_AWAKE true
    #define LTC6804_CORE_JUST_WOKE_UP  false

    //WGCToDo: Shouldn't this be static const uint8_t? And move to MAX1784Xcomms.h?
    static const unsigned int crc8Table[256] = {
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

#endif
