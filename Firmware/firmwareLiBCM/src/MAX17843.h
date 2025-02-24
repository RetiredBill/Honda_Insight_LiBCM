/******************************************************************************
* Definitions for MAX17843. (see MAX17843 datsheet)
*******************************************************************************/

#ifndef MAX17843_H
  #define MAX17843_H

  #define M873_MODEL_VERSION           0x8431 // Model is MAX17843, version 1

  /*
  *  MAX17843 commands
  */
  #define M873_HELLOALL                  0x57
  #define M873_WRITEDEVICE               0x04
  #define M873_WRITEALL                  0x02
  #define M873_READDEVICE                0x05
  #define M873_READALL                   0x03
  #define M873_READBLOCK                 0x06

  //========================================================================
  //========================================================================
  //  MAX17843 registers
  //========================================================================
  //========================================================================
  // Status Registers

  #define M873_VERSION                   0x00 //=========== register address

  #define M873_STATUS                    0x02 //=========== register address
  // STATUS register bit positions
  #define M873_STATUS_ALRTRST              15
  #define M873_STATUS_ALRTOV               14
  #define M873_STATUS_ALRTUV               13
  #define M873_STATUS_ALRTSHDNL            12
  #define M873_STATUS_ALRTSHDNLRT          11
  #define M873_STATUS_ALRTMSMTCH           10
  #define M873_STATUS_ALRTTCOLD             9
  #define M873_STATUS_ALRTTHOT              8
  #define M873_STATUS_ALRTPEC               7
  //#define M873_STATUS_Reserved              6
  //#define M873_STATUS_Reserved              5
  #define M873_STATUS_ALRTMAN               4
  //#define M873_STATUS_Reserved              3
  #define M873_STATUS_ALRTPAR               2
  #define M873_STATUS_ALRTFMEA2             1
  #define M873_STATUS_ALRTFMEA1             0

  #define M873_FMEA1                     0x03 //=========== register address
  // FMEA1 register bit positions
  #define M873_FMEA1_ALRTOSC1              15
  #define M873_FMEA1_ALRTOSC2              14
  //#define M873_FMEA1_Reserved              13
  #define M873_FMEA1_ALRTCOMMSEU1          12
  #define M873_FMEA1_ALRTCOMMSEL1          11
  #define M873_FMEA1_ALRTCOMMSEU2          10
  #define M873_FMEA1_ALRTCOMMSEL2           9
  #define M873_FMEA1_ALRTVDDL3              8
  #define M873_FMEA1_ALRTVDDL2              7
  #define M873_FMEA1_ALRTGNDL2              6
  #define M873_FMEA1_ALRTBALSW              5
  #define M873_FMEA1_ALRTTEMP               4
  #define M873_FMEA1_ALRTHVUV               3
  #define M873_FMEA1_ALRTGNDL3              2
  #define M873_FMEA1_ALRTVDDL1              1
  #define M873_FMEA1_ALRTGNDL1              0

  #define M873_ALRTCELL                  0x04 //=========== register address

  #define M873_ALRTOVCELL                0x05 //=========== register address

  #define M873_ALRTUVCELL                0x07 //=========== register address

  #define M873_ALRTBALSW                 0x08 //=========== register address

  #define M873_MINMAXCELL                0x0A //=========== register address

  #define M873_FMEA2                     0x0B //=========== register address
  // FMEA2 register bit positions
  //#define M873_FMEA2_bfReserved_SHIFT       3
  //#define M873_FMEA2_bfReserved_SIZE          13
  #define M873_FMEA2_ALRTHVHDRM             2
  //#define M873_FMEA2_Reserved               1
  #define M873_FMEA2_ALRTHVOV               0

  #define M873_ID1                       0x0D //=========== register address
  // ID1 register bit positions
  //   No separate bit fields, just a single word value

  #define M873_ID2                       0x0E //=========== register address
  // ID2 register bit positions
  #define M873_ID2_bfROMCRC_SHIFT           8
  #define M873_ID2_bfROMCRC_SIZE              8
  #define M873_ID2_bfDEVIDMsb_SHIFT         0
  #define M873_ID2_bfDEVIDMsb_SIZE            8

  //========================================================================
  // General configuration registers

  #define M873_DEVADDRESS                0x01 //=========== register address

  #define M873_DEVCFG1                   0x10 //=========== register address
  // DEVCFG1 register bit positions
  #define M873_DEVCFG1_POLARITY            15
  #define M873_DEVCFG1_ADCSELECT           14
  #define M873_DEVCFG1_bfFC_SHIFT          11
  #define M873_DEVCFG1_bfFC_SIZE              3
  #define M873_DEVCFG1_EMGCYDCHG           10
  #define M873_DEVCFG1_HVCPDIS              9
  //#define M873_DEVCFG1_Reserved             8
  #define M873_DEVCFG1_FORCEPOR             7
  #define M873_DEVCFG1_ALIVECNTEN           6
  #define M873_DEVCFG1_ADCTSTEN             5
  #define M873_DEVCFG1_SCANTODIS            4
  #define M873_DEVCFG1_BALSWDISABLE         3
  #define M873_DEVCFG1_NOPEC                2
  #define M873_DEVCFG1_ADDRUNLOCK           1
  #define M873_DEVCFG1_SPOR                 0

  #define M873_DEVCFG2                   0x1B //=========== register address
  #define M873_GPIO                      0x11 //=========== register address
  #define M873_TOPCELL                   0x1E //=========== register address

  //========================================================================
  // SCAN control registers

  #define M873_MEASUREEN                 0x12 //=========== register address
  // MEASUREEN register bit positions
  #define M873_MEASUREEN_BLKCONNECT        15
  #define M873_MEASUREEN_BLOCKEN           14
  #define M873_MEASUREEN_AIN2EN            13
  #define M873_MEASUREEN_AIN1EN            12
  #define M873_MEASUREEN_bfCELLEN_SHIFT     0
  #define M873_MEASUREEN_bfCELLEN_SIZE        12

  #define M873_SCANCTRL                  0x13 //=========== register address
  // SCANCTRL register bit positions
  #define M873_SCANCTRL_SCANDONE           15
  #define M873_SCANCTRL_SCANTIMEOUT        14
  #define M873_SCANCTRL_DATARDY            13
  #define M873_SCANCTRL_DELAYSEL           12
  #define M873_SCANCTRL_AUTOBALSWDIS       11
  #define M873_SCANCTRL_bfBALSWDIAG_SHIFT   8
  #define M873_SCANCTRL_bfBALSWDIAG_SIZE      3
  #define M873_SCANCTRL_RDFILT              7
  #define M873_SCANCTRL_bfOVSAMPL_SHIFT     4
  #define M873_SCANCTRL_bfOVSAMPL_SIZE        3
  #define M873_SCANCTRL_AMENDFILT           3
  #define M873_SCANCTRL_SCANMODE            2
  #define M873_SCANCTRL_FILTDONE            1
  #define M873_SCANCTRL_SCAN                0

  #define M873_ACQCFG                    0x19 //=========== register address
  // ACQCFG register bit positions
  //#define M873_ACQCFG_bfReserved_SHIFT     10
  //#define M873_ACQCFG_bfReserved_SIZE         6
  #define M873_ACQCFG_bfTHRMMODE_SHIFT      8
  #define M873_ACQCFG_bfTHRMMODE_SIZE         2
  //#define M873_ACQCFG_bfReserved_SHIFT      6
  //#define M873_ACQCFG_bfReserved_SIZE         2
  #define M873_ACQCFG_bfAINTIME_SHIFT       0
  #define M873_ACQCFG_bfAINTIME_SIZE          6

  //========================================================================
  // Alert configuration registers

  #define M873_ALRTOVEN                  0x14 //=========== register address
  #define M873_ALRTUVEN                  0x15 //=========== register address

  //========================================================================
  // Cell balancing configuration registers

  #define M873_TIMERCFG                  0x18 //=========== register address
  // aka WATCHDOG Register
  // TIMERCFG (aka WATCHDOG) register bit positions
  //#define M873_TIMERCFG_Reserved            15
  #define M873_TIMERCFG_bfCBPDIV_SHIFT      12
  #define M873_TIMERCFG_bfCBPDIV_SIZE          3
  #define M873_TIMERCFG_bfCBTIMER_SHIFT      8
  #define M873_TIMERCFG_bfCBTIMER_SIZE         4
  //#define M873_TIMERCFG_bfReserved_SHIFT     0
  //#define M873_TIMERCFG_bfReserved_SIZE        8

  #define M873_ADR                       0x0C //=========== register address
  // aka AUTOBALSWDIS Delay Register
  // ADR (aka AUTOBALSWDIS Delay Register) register bit positions
  #define M873_ADR_bfDIAG_RECOVERY_TIME_SHIFT     8
  #define M873_ADR_bfDIAG_RECOVERY_TIME_SIZE        8
  #define M873_ADR_bfCELL_RECOVERY_TIME_SHIFT     0
  #define M873_ADR_bfCELL_RECOVERY_TIME_SIZE        8

  #define M873_BALSWEN                   0x1A //=========== register address
  // BALSWEN register bit positions
  //#define M873_BALSWEN_bfReserved_SHIFT     12
  //#define M873_BALSWEN_bfReserved_SIZE         4
  #define M873_BALSWEN_bfBALSWEN_SHIFT       0
  #define M873_BALSWEN_bfBALSWEN_SIZE         12

  #define M873_BALDIAGCFG                0x1C //=========== register address
  #define M873_BALSWDCHG                 0x1D //=========== register address

  //========================================================================
  // Measurement results registers

  #define M873_CELL1                     0x20 //=========== register address
  #define M873_CELL2                     0x21 //=========== register address
  #define M873_CELL3                     0x22 //=========== register address
  #define M873_CELL4                     0x23 //=========== register address
  #define M873_CELL5                     0x24 //=========== register address
  #define M873_CELL6                     0x25 //=========== register address
  #define M873_CELL7                     0x26 //=========== register address
  #define M873_CELL8                     0x27 //=========== register address
  #define M873_CELL9                     0x28 //=========== register address
  #define M873_CELL10                    0x29 //=========== register address
  #define M873_CELL11                    0x2A //=========== register address
  #define M873_CELL12                    0x2B //=========== register address
  #define M873_BLOCK                     0x2C //=========== register address
  #define M873_AUXIN1                    0x2D //=========== register address
  #define M873_AUXIN2                    0x2E //=========== register address
  #define M873_TOTAL                     0x2F //=========== register address

  //========================================================================
  // Threshold registers

  #define M873_OVTHCLR                   0x40 //=========== register address
  #define M873_OVTHSET                   0x42 //=========== register address
  #define M873_UVTHCLR                   0x44 //=========== register address
  #define M873_UVTHSET                   0x46 //=========== register address
  #define M873_MSMTCH                    0x48 //=========== register address
  #define M873_AINOT                     0x49 //=========== register address
  #define M873_AINUT                     0x4A //=========== register address
  #define M873_BALSHRTTHR                0x4B //=========== register address
  #define M873_BALLOWTHR                 0x4C //=========== register address
  #define M873_BALHIGHTHR                0x4D //=========== register address

  //========================================================================
  // Diagnostic registers

  #define M873_DIAG                      0x50 //=========== register address

  #define M873_DIAGCFG                   0x51 //=========== register address
  // DIAGCFG register bit positions
  #define M873_DIAGCFG_bfCTSTDAC_SHIFT     12
  #define M873_DIAGCFG_bfCTSTDAC_SIZE        12
  #define M873_DIAGCFG_CTSTSRC             11
  //#define M873_DIAGCFG_Reserved            10
  #define M873_DIAGCFG_bfAUXINTSTEN_SHIFT   8
  #define M873_DIAGCFG_bfAUXINTSTEN_SIZE      2
  #define M873_DIAGCFG_MUXDIAGBUS           7
  #define M873_DIAGCFG_MUXDIAGPAIR          6
  //#define M873_DIAGCFG_Reserved             5
  #define M873_DIAGCFG_MUXDIAGEN            4
  #define M873_DIAGCFG_ALTMUXSEL            3
  #define M873_DIAGCFG_bfDIAGSEL_SHIFT      0
  #define M873_DIAGCFG_bfDIAGSEL_SIZE         3
  // DIAGCFG DIAGSEL value definitionss
  #define M873_DIAGSEL_NoMeasurement0       0
  #define M873_DIAGSEL_VAltRef              1
  #define M873_DIAGSEL_VAA                  2
  #define M873_DIAGSEL_LSAMP_Offset         3
  #define M873_DIAGSEL_ZeroScalADCOutput    4
  #define M873_DIAGSEL_FullScalADCOutput    5
  #define M873_DIAGSEL_DieTemperature       6
  #define M873_DIAGSEL_NoMeasurement7       7

  #define M873_CTSTCFG                   0x52 //=========== register address
  #define M873_ADCTEST1A                 0x57 //=========== register address
  #define M873_ADCTEST1B                 0x58 //=========== register address
  #define M873_ADCTEST2A                 0x59 //=========== register address
  #define M873_ADCTEST2B                 0x5A //=========== register address

  // Can be used for clearing flags...
  #define M873_CLEAR_ALL                    0

#endif
