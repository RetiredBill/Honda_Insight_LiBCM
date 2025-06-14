/******************************************************************************
* Definitions for Communication functions
*******************************************************************************/

#ifndef MAX1784Xcomms_h
  #define MAX1784Xcomms_h

  #define ALIVE_COUNT_DISABLE    0
  #define ALIVE_COUNT_ENABLE     1
  #define DATA_CHECK_SEED_0      0x00   // Data check seed value
  #define DATA_CHECK_EXPECTED_POR 0x20   // Data check expected result after POR
  #define DATA_CHECK_EXPECTED_NORMAL 0x00   // Data check expected result otherwise
  #define LAST_DEVICENUM_INVALID   0xff
  #define LAST_DEVICENUM_ALL       0x1f

  #define M871_READREG_RX_STATUS_TIMEOUT_us 9600 //Empirical data shows 4.3mS for 5 devices
  #define M871_READREG_TX_STATUS_TIMEOUT_us 600  //Empirical data shows up to 200uS

  // Message handling function and check control bit positions
  #define MCONT_RX_INT_FLAGS       10
  #define MCONT_RX_LAST_BYTE        9
  #define MCONT_RX_MESSAGE          8
  #define MCONT_RX_FIRST_BYTE       7
  #define MCONT_RX_STATUS_POLL      6
  #define MCONT_TX_INT_FLAGS        5
  #define MCONT_TX_STATUS_POLL      4
  #define MCONT_PRTX_BYTE           3
  #define MCONT_PRTX_STATUS         2
  #define MCONT_PRTX_INT_FLAGS      1
  #define MCONT_M871_FMEA           0
  // Pre-defined control values
  //   Do everything
  #define MCONT_FULL_CHECKS  (   BITVALUE(MCONT_M871_FMEA)\
                               | BITVALUE(MCONT_PRTX_INT_FLAGS)\
                               | BITVALUE(MCONT_PRTX_STATUS)\
                               | BITVALUE(MCONT_PRTX_BYTE)\
                               | BITVALUE(MCONT_TX_STATUS_POLL)\
                               | BITVALUE(MCONT_TX_INT_FLAGS)\
                               | BITVALUE(MCONT_RX_STATUS_POLL)\
                               | BITVALUE(MCONT_RX_FIRST_BYTE)\
                               | BITVALUE(MCONT_RX_MESSAGE)\
                               | BITVALUE(MCONT_RX_LAST_BYTE)\
                               | BITVALUE(MCONT_RX_INT_FLAGS))
  //   Do only a few pre-TX checks
  #define MCONT_FEW_PRTX     (   BITVALUE(MCONT_M871_FMEA)\
                               | BITVALUE(MCONT_PRTX_INT_FLAGS)\
                               | BITVALUE(MCONT_RX_STATUS_POLL)\
                               | BITVALUE(MCONT_RX_MESSAGE))
  //   Only RX the return, no extra checks
  #define MCONT_RX_MINIMUM_CHECKS (   BITVALUE(MCONT_RX_STATUS_POLL)\
                                    | BITVALUE(MCONT_RX_MESSAGE))
  //   Just launch the message, no RX, no checks
  //   Note: this is more efficiently done with a simple spiWrite() call
  #define MCONT_NO_RX_NO_CHECKS  0

  // for helping to clarify the meaning of argument values
  #define ARGUMENT_LEN_1         1
  #define ARGUMENT_LEN_2         2
  #define ARGUMENT_LEN_3         3
  #define ARGUMENT_LEN_4         4
  #define ARGUMENT_LEN_5         5
  #define ARGUMENT_LEN_6         6
  #define ARGUMENT_LEN_7         7
  #define ARGUMENT_LEN_8         8
  #define ARGUMENT_LEN_9         9

  #define MESSAGE_LEN_1          1
  #define MESSAGE_LEN_2          2
  #define MESSAGE_LEN_3          3
  #define MESSAGE_LEN_4          4
  #define MESSAGE_LEN_5          5
  #define MESSAGE_LEN_6          6
  #define MESSAGE_LEN_7          7
  #define MESSAGE_LEN_8          8
  #define MESSAGE_LEN_9          9

  // Per device read and write device commands
  static const uint8_t m873readDevice[] = {
    (0 << 3) + M873_READDEVICE,
    (1 << 3) + M873_READDEVICE,
    (2 << 3) + M873_READDEVICE,
    (3 << 3) + M873_READDEVICE,
    (4 << 3) + M873_READDEVICE,
  };
  static const uint8_t m873writeDevice[] = {
    (0 << 3) + M873_WRITEDEVICE,
    (1 << 3) + M873_WRITEDEVICE,
    (2 << 3) + M873_WRITEDEVICE,
    (3 << 3) + M873_WRITEDEVICE,
    (4 << 3) + M873_WRITEDEVICE,
  };

  // Exteral (public) functions
  bool MAX1784Xcomms_justWokeUp(void);
  void MAX1784Xcomms_setJustWokeUpState(bool justWokeUp);
  bool MAX1784Xcomms_max17841_CheckForPOR(void);
  void MAX1784Xcomms_max17841_Init(void);
  void MAX1784Xcomms_setup843Registers(int Device_count);
  void MAX1784Xcomms_max17843_reset(void);
  void MAX1784Xcomms_max17841_shutdown(void);
  void MAX1784Xcomms_wakeup(void);
  int  MAX1784Xcomms_enumerateDevices(void);
  uint8_t MAX1784Xcomms_getExpectedDataCheck(void);
  void MAX1784Xcomms_setExpectedDataCheck(uint8_t expectedDC);
  void MAX1784Xcomms_enableAliveCount(void);
  void MAX1784Xcomms_disableAliveCount(void);
  bool MAX1784Xcomms_readAll843Reg(int regAddr, int Device_count, uint16_t * resultBuff, int messageControl);
  bool MAX1784Xcomms_readDev843Reg(int regAddr, int devNum, uint16_t * resultBuff, int messageControl);
  bool MAX1784Xcomms_readBlock843(int startRegAddr, int blockSize, int devNum, uint16_t * resultBuff, int messageControl);
  bool MAX1784Xcomms_writeAll843Reg(int regAddr, int Device_count, int regValue, int messageControl);
  bool MAX1784Xcomms_writeDev843Reg(int regAddr, int devNum, int regValue, int messageControl);
  bool MAX1784Xcomms_checkActualVsExpected(uint32_t actual, uint32_t expected, char * testName, char * functionName);
  bool LTC68042comms_fullErrorChecking_get(void);
  void LTC68042comms_fullErrorChecking_set(bool doFullErrorChecks);
  void MAX1784Xcomms_diagnoseErrors(char * functionName);

#endif
