/******************************************************************************
* Definitions for MAX17841. (see MAX17841 datsheet)
*******************************************************************************/

#ifndef MAX17841_H
  #define MAX17841_H

 // MODEL and VERSION field value definitions
  #define M871_MODEL                       0x841 // Model MAX17841
  #define M871_VERSION                       0x4 //   version 4

  #define M871_STARTUP_TIME_ms                 2 // 2 msec max startup time

  /*
  *  MAX17841 register access commands
  */
  #define M871_READREG_RX_STATUS            0x01 // Reads RX_Status
  #define M871_READREG_TX_STATUS            0x03 // Reads TX_Status
  #define M871_WRITEREG_RX_INTERRUPT_ENABLE 0x04 // Write RX_Interrupt_Enable
  #define M871_READREG_RX_INTERRUPT_ENABLE  0x05 // Read RX_Interrupt_Enable
  #define M871_WRITEREG_TX_INTERRUPT_ENABLE 0x06 // Write TX_Interrupt_Enable
  #define M871_READREG_TX_INTERRUPT_ENABLE  0x07 // Read TX_Interrupt_Enable
  #define M871_WRITEREG_RX_INTERRUPT_FLAGS  0x08 // Write RX_Interrupt_Flags
  #define M871_READREG_RX_INTERRUPT_FLAGS   0x09 // Read RX_Interrupt_Flags
  #define M871_WRITEREG_TX_INTERRUPT_FLAGS  0x0A // Write TX_Interrupt_Flags
  #define M871_READREG_TX_INTERRUPT_FLAGS   0x0B // Read TX_Interrupt_Flags
  #define M871_WRITEREG_CONFIGURATION_1     0xOC // Write Configuration1
  #define M871_READREG_CONFIGURATION_1      0x0D // Read Configuration1
  #define M871_WRITEREG_CONFIGURATION_2     0x0E // Write Configuration2
  #define M871_READREG_CONFIGURATION_2      0x0F // Read Configuration2
  #define M871_WRITEREG_CONFIGURATION_3     0x10 // Write Configuration3
  #define M871_READREG_CONFIGURATION_3      0x11 // Read Configuration3
  #define M871_READREG_FMEA                 0x13 // Read FMEA
  #define M871_READREG_MODEL_MSB            0x15 // Read Model MSB
  #define M871_READREG_VERSION              0x17 // Read Version (and Model LSN)
  #define M871_READREG_RX_BYTE              0x19 // Read RX_byte
  #define M871_READREG_RX_SPACE             0x1B // Read RX_space
  #define M871_READREG_TX_QUEUE_SELECTS     0x95 // Read TX_queue_selects
  #define M871_READREG_RX_READ_POINTER      0x97 // Read RX_M871_RR_pointer
  #define M871_READREG_RX_WRITE_POINTER     0x99 // Read RX_M871_WRITEREG__pointer
  #define M871_READREG_RX_NEXT_MESSAGE      0x9B // Read RX_Next_message

  /*
  *  MAX17841 buffer commands (Description text taken from MAX17841 data sheet)
  */
  #define M871_CMD_CLR_RX_BUF     0xE0 // Resets the receive buffer and the receive buffer
                                       //   pointers to their default state.
  #define M871_CMD_CLR_TX_BUF     0x20 // Resets the transmit buffer to its default state and
                                       //   clears TX_Q and LD_Q
  #define M871_CMD_RX_RD_POINTER  0x91 // Reads the receive buffer starting at the address RX_RD_
                                       //   Pointer. Automatically increments the read pointer
                                       //   after the byte is read but does not increment the
                                       //   read pointer into the next message.
  #define M871_CMD_RD_NXT_MSG     0x93 // Reads the receive buffer starting at the address RX_
                                       //   NXT_MSG_Pointer (oldest unread message). Automatically
                                       //   increments the read pointer after the byte is read but
                                       //   does not increment the read pointer into the next message.
    // WR_NXT_LD_Q Commands: Increments LD_Q, then writes the transmit buffer
    // load queue. The increment occurs whether the host loads the data or not. The
    // command byte defines the first location to be written (locations 0 to 6). For
    // example, 0xB0 starts writing at location 0 and continues through location 6. Writes
    // beyond location 6 have no effect.
  #define M871_CMD_WR_NXT_LD_Q0   0xB0 // Start location LD_Q Location 0
  #define M871_CMD_WR_NXT_LD_Q1   0xB2 // Start location LD_Q Location 1
  #define M871_CMD_WR_NXT_LD_Q2   0xB4 // Start location LD_Q Location 2
  #define M871_CMD_WR_NXT_LD_Q3   0xB6 // Start location LD_Q Location 3
  #define M871_CMD_WR_NXT_LD_Q4   0xB8 // Start location LD_Q Location 4
  #define M871_CMD_WR_NXT_LD_Q5   0xBA // Start location LD_Q Location 5
  #define M871_CMD_WR_NXT_LD_Q6   0xBC // Start location LD_Q Location 6
    // WR_LD_Q Commands: Writes the transmit buffer load queue. The command byte
    // defines the first byte written (locations 0 to 6). For example, 0xC0 starts writing
    // at location 0 and continues through location 6. Writes beyond location 6 have no
    // effect.
  #define M871_CMD_WR_LD_Q0       0xC0 // Start location LD_Q Location 0
  #define M871_CMD_WR_LD_Q1       0xC2 // Start location LD_Q Location 1
  #define M871_CMD_WR_LD_Q2       0xC4 // Start location LD_Q Location 2
  #define M871_CMD_WR_LD_Q3       0xC6 // Start location LD_Q Location 3
  #define M871_CMD_WR_LD_Q4       0xC8 // Start location LD_Q Location 4
  #define M871_CMD_WR_LD_Q5       0xCA // Start location LD_Q Location 5
  #define M871_CMD_WR_LD_Q6       0xCC // Start location LD_Q Location 6
    // RD _LD_Q Command: Reads transmit buffer load queue. The command byte
    // defines the first byte read (locations 0 to 6). For example, 0xC1 starts reading
    // at location 0 and continues through location 6. Reading beyond location 6 reads
    // zeros.
  #define M871_CMD_RD_LD_Q0       0xC1 // Start location LD_Q Location 0
  #define M871_CMD_RD_LD_Q1       0xC3 // Start location LD_Q Location 1
  #define M871_CMD_RD_LD_Q2       0xC5 // Start location LD_Q Location 2
  #define M871_CMD_RD_LD_Q3       0xC7 // Start location LD_Q Location 3
  #define M871_CMD_RD_LD_Q4       0xC9 // Start location LD_Q Location 4
  #define M871_CMD_RD_LD_Q5       0xCB // Start location LD_Q Location 5
  #define M871_CMD_RD_LD_Q6       0xCD // Start location LD_Q Location 6

  /*
  *  MAX17841 register bit position definitions
  *   (amount to left shift shift a value for that bit position)
  */

  // Can be used for clearing flags...
  #define M871_CLEAR_ALL                                   0

  // RX_STATUS register bits
  #define M871_RX_STATUS_RX_Error_Status                   7
  //#define M871_RX_STATUS_Reserved                           6 // Reserved
  #define M871_RX_STATUS_RX_Busy_Status                    5
  #define M871_RX_STATUS_RX_Idle_Status                    4
  #define M871_RX_STATUS_RX_Overflow_Status                3
  #define M871_RX_STATUS_RX_Full_Status                    2
  #define M871_RX_STATUS_RX_STOP_Status                    1
  #define M871_RX_STATUS_RX_Empty_Status                   0

  // TX_STATUS register bits
  //#define M871_TX_STATUS_Reserved                          7 // Reserved
  //#define M871_TX_STATUS_Reserved                          6 // Reserved
  #define M871_TX_STATUS_TX_Busy_Status                    5
  #define M871_TX_STATUS_TX_Idle_Status                    4
  #define M871_TX_STATUS_TX_Overflow_Status                3
  #define M871_TX_STATUS_TX_Full_Status                    2
  #define M871_TX_STATUS_TX_Available_Status               1
  #define M871_TX_STATUS_TX_Empty_Status                   0

  // RX_INTERRUPT_ENABLE register bits
  #define M871_RX_INTEN_RX_Error_INT_Enable                7
  //#define M871_RX_INTEN_Reserved                           6 // Reserved
  #define M871_RX_INTEN_RX_Busy_INT_Enable                 5
  #define M871_RX_INTEN_RX_Idle_INT_Enable                 4
  #define M871_RX_INTEN_RX_Overflow_INT_Enable             3
  #define M871_RX_INTEN_RX_Full_INT_Enable                 2
  #define M871_RX_INTEN_RX_Stop_INT_Enable                 1
  #define M871_RX_INTEN_RX_Empty_INT_Enable                0

  // TX_INTERRUPT_ENABLE register bits
  //#define M871_TX_INTEN_Reserved                           7 // Reserved
  //#define M871_TX_INTEN_Reserved                           6 // Reserved
  #define M871_TX_INTEN_TX_Busy_INT_Enable                 5
  #define M871_TX_INTEN_TX_Idle_INT_Enable                 4
  #define M871_TX_INTEN_TX_Overflow_INT_Enable             3
  #define M871_TX_INTEN_TX_Full_INT_Enable                 2
  #define M871_TX_INTEN_TX_Available_INT_Enable            1
  #define M871_TX_INTEN_TX_Empty_INT_Enable                0

  // RX_INTERRUPT_FLAG register bits
  #define M871_RX_FLAG_RX_Error_INT_Flag                   7
  //#define M871_RX_FLAG_Reserved                            6 // Reserved
  #define M871_RX_FLAG_RX_Busy_INT_Flag                    5
  #define M871_RX_FLAG_RX_Idle_INT_Flag                    4
  #define M871_RX_FLAG_RX_Overflow_INT_Flag                3
  #define M871_RX_FLAG_RX_Full_INT_Flag                    2
  #define M871_RX_FLAG_RX_Stop_INT_Flag                    1
  #define M871_RX_FLAG_RX_Empty_INT_Flag                   0

  // TX_INTERRUPT_FLAG register bits
  #define M871_TX_FLAG_POR_Flag                            7 // chip Power On Reset flag
  //#define M871_TX_FLAG_Reserved                            6 // Reserved
  #define M871_TX_FLAG_TX_Busy_INT_Flag                    5
  #define M871_TX_FLAG_TX_Idle_INT_Flag                    4
  #define M871_TX_FLAG_TX_Overflow_INT_Flag                3
  #define M871_TX_FLAG_TX_Full_INT_Flag                    2
  #define M871_TX_FLAG_TX_Available_INT_Flag               1
  #define M871_TX_FLAG_TX_Empty_INT_Flag                   0

  // CONFIGURATION_1 register bits
  #define M871_CONFIG_1_Single_Ended_Mode                  7
  #define M871_CONFIG_1_bfBaud_Rate_SHIFT                  5
  #define M871_CONFIG_1_bfBaud_Rate_SIZE                     2
  #define M871_CONFIG_1_bfDevice_Count_SHIFT               0 // Not used by chip. Available for host...
  #define M871_CONFIG_1_bfDevice_Count_SIZE                  4
  // CONFIGURATION_1 field value definitions
  #define M871_CONFIG_1_BAUDRATE_500kpbs                 0 // 1 is also 500kpbs
  #define M871_CONFIG_1_BAUDRATE_1Mpbs                   2
  #define M871_CONFIG_1_BAUDRATE_2Mpbs                   3 // (POR default)

  // CONFIGURATION_2 register bits
  #define M871_CONFIG_2_RX_Raw_Data                        7
  #define M871_CONFIG_2_TX_Raw_Data                        6
  #define M871_CONFIG_2_TX_Preambles                       5 // Continuous mode
  #define M871_CONFIG_2_TX_Queue                           4
  #define M871_CONFIG_2_TX_Odd_Parity                      3
  #define M871_CONFIG_2_TX_Pause                           2
  #define M871_CONFIG_2_TX_No_Stop                         1
  #define M871_CONFIG_2_TX_No_Preamble                     0 // (in a mesage mode)
  // CONFIGURATION_2 register mode definitions
  //  Enable transmit preambles mode
  #define M871_CONF_2_TX_PREAMBLE_MODE     (BITVALUE(M871_CONFIG_2_TX_Preambles) | BITVALUE(M871_CONFIG_2_TX_Queue))
  // Enable transmit queue (messages) mode
  #define M871_CONF_2_TX_QUEUE_MODE        (BITVALUE(M871_CONFIG_2_TX_Queue))
  // Enable transmit keep-alive stop characters mode. This keeps devices in standby
  #define M871_CONF_2_TX_KEEPALIVE_MODE    0x00

  // CONFIGURATION_3 register bits
  //#define M871_CONFIG_3_Reserved                           7 // Reserved
  //#define M871_CONFIG_3_Reserved                           6 // Reserved
  #define M871_CONFIG_3_TX_Unlimited                       5
  #define M871_CONFIG_3_DOUT_Enable                        4
  #define M871_CONFIG_3_Keep_Alive                         0 // bits [3:0]
  // CONFIGURATION_3 Keep_Alive field value definitions
  #define M871_KEEP_ALIVE_0_USEC                      0x00 // Keep alive 0us (not sure what that means)
  #define M871_KEEP_ALIVE_10_USEC                     0x01 // Keep alive 10us
  #define M871_KEEP_ALIVE_20_USEC                     0x02 // Keep alive 20us
  #define M871_KEEP_ALIVE_40_USEC                     0x03 // Keep alive 40us
  #define M871_KEEP_ALIVE_80_USEC                     0x04 // Keep alive 80us
  #define M871_KEEP_ALIVE_160_USEC                    0x05 // Keep alive 160us
  #define M871_KEEP_ALIVE_320_USEC                    0x06 // Keep alive 320us
  #define M871_KEEP_ALIVE_640_USEC                    0x07 // Keep alive 640us
  #define M871_KEEP_ALIVE_1280_USEC                   0x08 // Keep alive 1280us
  #define M871_KEEP_ALIVE_2560_USEC                   0x09 // Keep alive 2560us
  #define M871_KEEP_ALIVE_5120_USEC                   0x0A // Keep alive 5120us
  #define M871_KEEP_ALIVE_10240_USEC                  0x0B // Keep alive 10240us
  #define M871_KEEP_ALIVE_DISABLED                    0x0F // Keep alive disabled

  // FMEA register bits
  //#define M871_FMEA_bfReserved_SHIFT                       3 // bits [7:3] Reserved
  //#define M871_FMEA_bfReserved_SIZE                          5
  #define M871_FMEA_AGND_Alert                             2
  #define M871_FMEA_VDDL_Alert                             1
  #define M871_FMEA_GNDL_Alert                             0

  // MODEL register bits
  //   No separate bit fields, just a single byte value

  // VERSION register bits
  #define M871_VERSION_bfModelLsb_SHIFT                  4 // Lower 4 bits of model number
  #define M871_VERSION_bfModelLsb_SIZE                     4
  #define M871_VERSION_bfVersion_SHIFT                   0 // 4 bit version number
  #define M871_VERSION_bfVersion_SIZE                      4

  // RX_BYTE register bits
  //#define M871_RX_BYTE_bfReserved_SHIFT  3   // bits [7:3] Reserved
  //#define M871_RX_BYTE_bfReserved_SIZE     5
  #define M871_RX_BYTE_First_Byte        2 // The byte at location RX_RD_Pointer is the first data byte
                                           //   in a message.
  #define M871_RX_BYTE_Byte_Error        1 // The byte at location RX_RD_Pointer may contain an error (Manchester
                                           //   and/or parity error). This bit drives the
                                           //   RX_Error interrupt.
  #define M871_RX_BYTE_Last_Byte         0 // The byte at location RX_RD_Pointer is the last byte in a message.

  // RX_SPACE register bits
  //   No separate bit fields, just a single byte value

  // TX_QUEUE_SELECTS register bits
  //#define M871_TX_QUEUE_SELECTS_Reserved             // bits [7:4] Reserved
  #define M871_TX_QUEUE_SELECTS_TX_Q                 2 // 2 bits
  #define M871_TX_QUEUE_SELECTS_LD_Q                 0 // 2 bits

  // RX_READ_POINTER register bits
  //   No separate bit fields, just a single byte value

  // RX_WRITE_POINTER register bits
  //   No separate bit fields, just a single byte value

  // RX_NEXT_MESSAGE register bits
  //   No separate bit fields, just a single byte value

  #define M871_SPECIFIED_MAX_STARTUP_TIME_ms 2 // De-assertion of SHDNn to RXP valid

#endif
