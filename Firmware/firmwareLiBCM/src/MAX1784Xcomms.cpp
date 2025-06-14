// MAX1784x Communication functions

// MAX17843 is 12-cell BMS management IC, which uses differential UART busses for communication.
// MAX17841 is a differential UART-to-SPI bridge IC

#include "libcm.h"

// POR state for BMS system: This determines if the BMS chips need to be reset or not
bool m873JustWokeUp = true;
bool MAX1784Xcomms_justWokeUp(void)                    { return m873JustWokeUp; }
void MAX1784Xcomms_setJustWokeUpState(bool justWokeUp) { m873JustWokeUp = justWokeUp; }

bool max871FirstMessageFlag = true; // first message received state

uint8_t m873AliveCountSeed = ALIVE_COUNT_DISABLE; // (private) MAX17843 alive-count message ID tracker
void MAX1784Xcomms_enableAliveCount(void)  { m873AliveCountSeed = ALIVE_COUNT_ENABLE; }
void MAX1784Xcomms_disableAliveCount(void) { m873AliveCountSeed = ALIVE_COUNT_DISABLE; }

uint8_t m873ExpectedDataCheck = 0;// expected value of received data-check byte
uint8_t MAX1784Xcomms_getExpectedDataCheck(void)               { return m873ExpectedDataCheck; }
void    MAX1784Xcomms_setExpectedDataCheck(uint8_t expectedDC) { m873ExpectedDataCheck = expectedDC; }

bool m873doFullErrorChecks = true; // do full error checking state
bool LTC68042comms_fullErrorChecking_get(void)                   { return m873doFullErrorChecks; }
void LTC68042comms_fullErrorChecking_set(bool doFullErrorChecks) { m873doFullErrorChecks = doFullErrorChecks; }

// used for deferred error checking
uint8_t lastDataCheckByte = DATA_CHECK_EXPECTED_POR;
int     lastNumDevices = LAST_DEVICENUM_ALL;
int     lastDeviceNum = LAST_DEVICENUM_INVALID;

/////////////////////////////////////////////////////////////////////////////////////////

// Check provided (actual) versus expected value. If not equal, print error message.
//  return comparisson result 1 or 0
bool MAX1784Xcomms_checkActualVsExpected(
  uint32_t actual,
  uint32_t expected,
  char * testName, char * functionName)
{
  if (actual != expected) {
    // then it is not what we expect
    Serial.print(F("  ERROR: "));
    Serial.print(testName);
    Serial.print(F(" failure in "));
    Serial.print(functionName);
    Serial.print(F("! Expected 0x"));
    Serial.print(expected, HEX);
    Serial.print(F(", got 0x"));
    Serial.println(actual, HEX);
    return 0;
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

//Read and return MAX17841 ASCI chip Model and Version
uint16_t MAX1784Xcomms_ReadModelAndVersion(void)
{
  const uint8_t CMD_LENGTH = 1; // bytes to in command
  const uint8_t DATA_LENGTH = 2;
  uint8_t cmd[CMD_LENGTH];
  uint8_t data[DATA_LENGTH];

  // write out M871_READREG_MODEL (command), read back 2 bytes:
  //   with DATA_LENGTH = 2, take advantage of reading sequential registers
  //   and get both model and version in 1 transaction
  cmd[0] = M871_READREG_MODEL_MSB;
  LTC68042configure_spiWriteRead(cmd, CMD_LENGTH, &data[0], DATA_LENGTH);
  return ((data[0] << 8) | data[1]);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool MAX1784Xcomms_max17841_CheckForPOR(void)
{
  const uint8_t CMD_LENGTH = 1; // bytes in command
  const uint8_t DATA_LENGTH = 1;
  uint8_t cmd[CMD_LENGTH];
  uint8_t data[DATA_LENGTH];

  // check POR flag
  cmd[0] = M871_READREG_TX_INTERRUPT_FLAGS;
  LTC68042configure_spiWriteRead(cmd, CMD_LENGTH, data, DATA_LENGTH);
  if (BITVALUE(M871_TX_FLAG_POR_Flag) & data[0]) {return true; } // POR flag was set
  else                                           {return false;} //  it was not
}

/////////////////////////////////////////////////////////////////////////////////////////

//initialize the MAX17841 ASCI chip configuration
//WGCToDo: Add the other MAX17841 registers, rather than depending on power-on values?
void MAX1784Xcomms_max17841_Init(void)
{
  const uint8_t CMD_LENGTH_1 = 1; // bytes in command
  const uint8_t CMD_LENGTH_2 = 2; // bytes in command
  const uint8_t DATA_LENGTH = 2;
  uint8_t cmd[CMD_LENGTH_2];
  uint8_t data[DATA_LENGTH];

  //WGCToDo speedup: Move model/version check to end of coldboot or sometime later
  // mod/version should be 0x8417
  MAX1784Xcomms_checkActualVsExpected(
    MAX1784Xcomms_ReadModelAndVersion(),
    (uint16_t)((M871_MODEL << M871_VERSION_bfModelLsb_SIZE) | M871_VERSION),
    "Model/Version", __func__);

  // check and then clear POR flag
  cmd[0] = M871_READREG_TX_INTERRUPT_FLAGS;
  LTC68042configure_spiWriteRead(cmd, CMD_LENGTH_1, data, 1);
  if (MAX1784Xcomms_justWokeUp()) {
    MAX1784Xcomms_checkActualVsExpected(data[0], BITVALUE(M871_TX_FLAG_POR_Flag), "POR flag", __func__);
  }
  else {
    MAX1784Xcomms_checkActualVsExpected(data[0], M871_CLEAR_ALL, "POR flag", __func__);
  }
  cmd[0] = M871_WRITEREG_TX_INTERRUPT_FLAGS;
  cmd[1] = M871_CLEAR_ALL;
  LTC68042configure_spiWrite(CMD_LENGTH_2, cmd);

  // Set configuration 3 register the initial keep-alive period
  cmd[0] = M871_WRITEREG_CONFIGURATION_3;
  cmd[1] = M871_KEEP_ALIVE_10_USEC;
  LTC68042configure_spiWrite(CMD_LENGTH_2, cmd);

  //WGCToDo speedup: M871 RX_INTERRUPT_ENABLE and TX_INTERRUPT_ENABLE are sequential. These could be combined
  // Set Rx_interrupt enable register RX_Error_int_enable and RX_Overflow_INT_Enable bits
  cmd[0] = M871_WRITEREG_RX_INTERRUPT_ENABLE;
  cmd[1] = M871_RX_INTERRUPT_ENABLE_INIT;
  LTC68042configure_spiWrite(CMD_LENGTH_2, cmd);

 // Set TX_interrupt enable register TX_Overflow_INT_Enable bit
  cmd[0] = M871_WRITEREG_TX_INTERRUPT_ENABLE;
  cmd[1] = BITVALUE(M871_TX_INTEN_TX_Overflow_INT_Enable);
  LTC68042configure_spiWrite(CMD_LENGTH_2, cmd);

  // Clear Receive Buffer
  cmd[0] = M871_CMD_CLR_RX_BUF;
  LTC68042configure_spiWrite(CMD_LENGTH_1, cmd);
  // MAX17841.pdf pg 17: Need to resynchronize UART receiver logic after M871_CMD_CLR_RX_BUF with either
  //   "a preamble byte or an idle state lasting at least one UART byte period"
  //   wakeup() M871_CONF_2_TX_PREAMBLE_MODE should do it...
}

/////////////////////////////////////////////////////////////////////////////////////////

// Set the MAX17843 chips to their POR state
void MAX1784Xcomms_max17843_reset(void)
{
  // Try to force GPIO3 low to make sure SHDNL is not driven high, just in case
  MAX1784Xcomms_writeAll843Reg(M873_GPIO, TOTAL_IC, M871_CLEAR_ALL, MCONT_FULL_CHECKS);
  // This will change to no longer returning alive-count, so
  MAX1784Xcomms_disableAliveCount();  // disable alive-count checking
  MAX1784Xcomms_setExpectedDataCheck(DATA_CHECK_EXPECTED_POR);
  max871FirstMessageFlag = true;
  // Force POR on all MAX17843 chips
  //  7 DEVCFG1.FORCEPOR  set
  //  0 DEVCFG1.SPOR      set
  // Note: since this is resetting the chips, no message is getting returned...
  MAX1784Xcomms_writeAll843Reg(M873_DEVCFG1, TOTAL_IC,
    (BITVALUE(M873_DEVCFG1_FORCEPOR) | BITVALUE(M873_DEVCFG1_SPOR)), MCONT_NO_RX_NO_CHECKS);
}

/////////////////////////////////////////////////////////////////////////////////////////

// Shutdown sequence for MAX17841 ASCI chip:
//   Make sure MAX17843 chips see a reset, then shut down MAX17841
void MAX1784Xcomms_max17841_shutdown(void)
{
  MAX1784Xcomms_max17843_reset();

  // Disable MAX17841
  MAX17841configure_disableMAX17841();
  // If there is not a long enough delay, then the MAX17843 chips will report STATUS errors
  //   probably due to SHDNl just sagging, and not going low enough to provide a clean power on
  //  0-8 ms too short; get manchester/parity errors on many devices
  //  10-11 gives ALRTSHDNL and (expected) ALRTRST on many devices
  //  12 and above (tested to 20) works, getting just (expected) ALRTRST, but is a long time!
  //delay(12); //Assume this delay will be taken care of by sleep or whatever...
}

/////////////////////////////////////////////////////////////////////////////////////////

// Wake up the UART slave devices and initialize BMS ICs in daisy chain.
//   Leave the MAX17843 chips in active mode, ready for messages
void MAX1784Xcomms_wakeup(void)
{
  uint32_t timestamp_latestEvent_us;
  uint32_t latestEventDelay_us;
  uint8_t cmd[2];
  uint8_t rxBuff[2];

  // Set WRITE_CONFIGURATION_2 to Enable transmit preambles mode
  cmd[0] = M871_WRITEREG_CONFIGURATION_2;
  cmd[1] = M871_CONF_2_TX_PREAMBLE_MODE;
  LTC68042configure_spiWrite(2, cmd);
  timestamp_latestEvent_us = micros();
  latestEventDelay_us = 0;
  // MAX17843.pdf pg 97: expect need to send preambles for 2ms per device...
  //   Empirical data gives ~4.4ms for 5 devices

  // Read RX_Status register and wait for preambles to start to show up
  //   RX_Busy_Status and RX_Empty_Status bits true, rest false
  //     RX_Busy_Status 1 => ASCI chip UART is busy receiving data (preambles)
  //     RX_Empty_Status 1 => ASCI chip rx buffer has no data yet (preambles, no stop yet)
  cmd[0] = M871_READREG_RX_STATUS;
  while (M871_READREG_RX_STATUS_TIMEOUT_us > latestEventDelay_us) {
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    if ((BITVALUE(M871_RX_STATUS_RX_Busy_Status) | BITVALUE(M871_RX_STATUS_RX_Empty_Status)) == rxBuff[0]) break;
    latestEventDelay_us = micros() - timestamp_latestEvent_us;
  }
  if (M871_READREG_RX_STATUS_TIMEOUT_us <= latestEventDelay_us) {
    Serial.print(F("  ERROR: timed out waiting for 0x21 == RX_STATUS. Last status was : 0x"));// WGCToDo: Change text to not use fixed 0x value?
    Serial.println(rxBuff[0], HEX);
  }
  #ifdef WGC_DEBUG_COMMS
  else {
    Serial.print(F(" Delay for 0x21 == RX_STATUS: "));// WGCToDo: Change text to not use fixed 0x value?
    Serial.print(latestEventDelay_us);
    Serial.println(F(" uSec"));
  }
  #endif

  // Set WRITE_CONFIGURATION_2 to Disable transmit preambles mode, and
  //   enter TX queue mode, ready to send messages
  cmd[0] = M871_WRITEREG_CONFIGURATION_2;
  cmd[1] = M871_CONF_2_TX_QUEUE_MODE;
  LTC68042configure_spiWrite(2, cmd);
  timestamp_latestEvent_us = micros();
  latestEventDelay_us = 0;

  // Read RX_Status regiter and wait for RX_Status RX_Empty_Status bit == 0
  //   This indicates that all preambles have made it back down the daisy chain
  // MAX17841.pdf pg 17: Need to wait "untill all transmitted preambles have been received before
  //   clearing the buffer"
  cmd[0] = M871_READREG_RX_STATUS;
  while (M871_READREG_RX_STATUS_TIMEOUT_us > latestEventDelay_us) {
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    if (! (BITVALUE(M871_RX_STATUS_RX_Empty_Status) & rxBuff[0])) break;
    latestEventDelay_us = micros() - timestamp_latestEvent_us;
  }
  if (M871_READREG_RX_STATUS_TIMEOUT_us <= latestEventDelay_us) {
    Serial.print(F("  ERROR: timed out waiting for RX_STATUS.RX_Empty_Status cleared. Last status was : 0x"));
    Serial.println(rxBuff[0], HEX);
  }
  #ifdef WGC_DEBUG_COMMS
  else {
    Serial.print(F(" Delay for RX_STATUS.RX_Empty_Status cleared: "));
    Serial.print(latestEventDelay_us);
    Serial.println(F(" uSec"));
  }
  #endif

  // Now set long-term configuration 3 register keep-alive period
  cmd[0] = M871_WRITEREG_CONFIGURATION_3;
  cmd[1] = M871_KEEP_ALIVE_640_USEC;
  LTC68042configure_spiWrite(2, cmd);

  // Clear transmit buffer
  cmd[0] = M871_CMD_CLR_TX_BUF;
  LTC68042configure_spiWrite(1, cmd);

  // Clear receive buffer
  cmd[0] = M871_CMD_CLR_RX_BUF;
  LTC68042configure_spiWrite(1, cmd);
  // MAX17841.pdf pg 17:
  //   Need to resynchronize UART receiver logic after M871_CMD_CLR_RX_BUF with either
  //   "a preamble byte or an idle state lasting at least one UART byte period"
  // UART byte period is 12usec (@2Mbps default, which we use)
  delayMicroseconds(12); //   This delay is required to implement that idle state
}

/////////////////////////////////////////////////////////////////////////////////////////

//  Message handling for the MAX17843 chips
//    Private function
bool MAX17843_message873Workhorse(
  uint8_t tx_Data[],
  uint8_t tx_len,
  uint8_t *rx_data,
  uint8_t rx_len,
  int messageControl,
  char * functionName)
{
digitalWrite(PIN_LASIG, LOW); // #1
digitalWrite(PIN_LASIG, HIGH);
  bool allOK = 1;
  uint8_t cmd[2];
  uint8_t rxBuff[4];
  uint32_t timestamp_latestEvent_us;
  uint32_t latestEventDelay_us;

  // Transfter outgoing message to ASCI chip load queue
  LTC68042configure_spiWrite(tx_len, tx_Data);

  // ================= Do specified checks before starting a message
  if (messageControl & BITVALUE(MCONT_M871_FMEA)) {
    // Check for overall ASCI (MAX17841) health
    // Read and check FMEA bits
    cmd[0] = M871_READREG_FMEA;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    allOK &= MAX1784Xcomms_checkActualVsExpected(
      ( (   BITVALUE(M871_FMEA_AGND_Alert)
          | BITVALUE(M871_FMEA_VDDL_Alert)
          | BITVALUE(M871_FMEA_GNDL_Alert)
        ) & rxBuff[0]
      ),
      M871_CLEAR_ALL,
      "Pre message FMEA", functionName);
  }

  if (messageControl & BITVALUE(MCONT_PRTX_INT_FLAGS)) {
    // Check for RX and TX interrupt flags all clear
    // RX_INTERRUPT_FLAG in rxBuff[0]
    // TX_INTERRUPT_FLAG in rxBuff[1]
    cmd[0] = M871_READREG_RX_INTERRUPT_FLAGS;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 2);
    if (! MAX1784Xcomms_checkActualVsExpected(
         ( (   BITVALUE(M871_RX_FLAG_RX_Error_INT_Flag) // mask for all bits to check
             | BITVALUE(M871_RX_FLAG_RX_Busy_INT_Flag)
             | BITVALUE(M871_RX_FLAG_RX_Idle_INT_Flag)
             | BITVALUE(M871_RX_FLAG_RX_Overflow_INT_Flag)
             | BITVALUE(M871_RX_FLAG_RX_Full_INT_Flag)
             | BITVALUE(M871_RX_FLAG_RX_Stop_INT_Flag)
             | BITVALUE(M871_RX_FLAG_RX_Empty_INT_Flag)
           ) & rxBuff[0]),
         M871_CLEAR_ALL,                           // bits expected to be set
         "Pre message RX interrupt flags", functionName)) {
      // then clear all the RX flags
      cmd[0] = M871_WRITEREG_RX_INTERRUPT_FLAGS;
      cmd[1] = M871_CLEAR_ALL;
      LTC68042configure_spiWrite(2, cmd);
      allOK = 0;
    }
    if (! MAX1784Xcomms_checkActualVsExpected(
         ( (   BITVALUE(M871_TX_FLAG_POR_Flag)     // mask for all bits to check
             | BITVALUE(M871_TX_FLAG_TX_Overflow_INT_Flag)
             | BITVALUE(M871_TX_FLAG_TX_Full_INT_Flag)
           ) & rxBuff[1]),
         M871_CLEAR_ALL,                           // bits expected to be set
         "Pre message TX interrupt flags", functionName)) {
      // then clear all the TX flags
      cmd[0] = M871_WRITEREG_TX_INTERRUPT_FLAGS;
      cmd[1] = M871_CLEAR_ALL;
      LTC68042configure_spiWrite(2, cmd);
      allOK = 0;
    }
  }

  if (messageControl & BITVALUE(MCONT_PRTX_STATUS)) {
    //   Read RX_STATUS and TX_STATUS, and verify that we are ready to TX and RX a message:
    //     RX_STATUS.RX_Full_Status not set (RX_STATUS is in rxBuff[0])
    //     TX_STATUS.TX_Overflow_Status not set (TX_STATUS is in rxBuff[1])
    //WGCToDo: checking more status bits than just these for now...
    // Note: if a keep-alive happens to occur during the M871_READREG_RX_STATUS,
    //   the M871_RX_STATUS_RX_Busy_Status bit will likely be set. This should not
    //   occur frequently, but will occur at times, as long as the keep-alive
    //   interval is rather short compared to the TX_Q message interval.
    //   The keep-alive of necessity is 640usec. Will lots of message error
    //   checking, the TX_Q message interval IS long.
    //   The fix here is to do 2 reads in close succession. A "collision"
    //   should only leave the M871_RX_STATUS_RX_Busy_Status bit set in 1 of
    //   the 2.
    //   Another aproach would be to set GPIO3 high and not use the keep-alive
    //   during active times, but only use keep-alive in a "standby" mode.
    //   This introduces a risk that if GPIO3 is left on, it can stay on
    //   "forever", the battery will slowly self discharge to destruction...
    cmd[0] = M871_READREG_RX_STATUS;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 2);
    LTC68042configure_spiWriteRead(cmd, 1, &(rxBuff[2]), 2);
    if (BITVALUE(M871_RX_STATUS_RX_Busy_Status) & rxBuff[0]) {
      // then the first sample has M871_RX_STATUS_RX_Busy_Status set, so use
      //   the second
      rxBuff[0] = rxBuff[2];
    }
    allOK &= MAX1784Xcomms_checkActualVsExpected(
      ( (   BITVALUE(M871_RX_STATUS_RX_Error_Status) // mask for all bits to check
          | BITVALUE(M871_RX_STATUS_RX_Busy_Status)
          | BITVALUE(M871_RX_STATUS_RX_Idle_Status)
          | BITVALUE(M871_RX_STATUS_RX_Overflow_Status)
          | BITVALUE(M871_RX_STATUS_RX_Full_Status)
          | BITVALUE(M871_RX_STATUS_RX_STOP_Status)
          | BITVALUE(M871_RX_STATUS_RX_Empty_Status)
        ) & rxBuff[0]),
      (   BITVALUE(M871_RX_STATUS_RX_Idle_Status) // bits expected to be set
        | BITVALUE(M871_RX_STATUS_RX_Empty_Status)),
      "Pre message RX_STATUS", functionName);

    allOK &= MAX1784Xcomms_checkActualVsExpected(
      ( (   BITVALUE(M871_TX_STATUS_TX_Busy_Status)  // mask for all bits to check
          | BITVALUE(M871_TX_STATUS_TX_Idle_Status)
          | BITVALUE(M871_TX_STATUS_TX_Overflow_Status)
          | BITVALUE(M871_TX_STATUS_TX_Full_Status)
          | BITVALUE(M871_TX_STATUS_TX_Available_Status)
          | BITVALUE(M871_TX_STATUS_TX_Empty_Status)
        ) & rxBuff[1]),
      (   BITVALUE(M871_TX_STATUS_TX_Idle_Status)   // bits expected to be set
        | BITVALUE(M871_TX_STATUS_TX_Available_Status)
        | BITVALUE(M871_TX_STATUS_TX_Empty_Status)),
      "Pre message TX_STATUS", functionName);
  }

  if (messageControl & BITVALUE(MCONT_PRTX_BYTE)) {
    //  RX_BYTE.Byte_Error is not set
    //  RX_BYTE.Last_Byte is set (except for very first time)
    // NOTE: Last_Byte will NOT be set on the very first use after POR. Test for 0 == Device_count
    //WGCToDo: checking more status bits than just these for now...
    cmd[0] = M871_READREG_RX_BYTE;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    if (max871FirstMessageFlag) {
      // then this is the first usage, and Last_Byte should be 0
      max871FirstMessageFlag = false;
      allOK &= MAX1784Xcomms_checkActualVsExpected(
        ( (   BITVALUE(M871_RX_BYTE_First_Byte)
            | BITVALUE(M871_RX_BYTE_Byte_Error)
            | BITVALUE(M871_RX_BYTE_Last_Byte)
          ) & rxBuff[0]),
        M871_CLEAR_ALL,
        "Pre message RX_BYTE", functionName);
    }
    else {
      // Last_Byte should be 1
      allOK &= MAX1784Xcomms_checkActualVsExpected(
        ( (   BITVALUE(M871_RX_BYTE_First_Byte)
            | BITVALUE(M871_RX_BYTE_Byte_Error)
            | BITVALUE(M871_RX_BYTE_Last_Byte)
          ) & rxBuff[0]),
        BITVALUE(M871_RX_BYTE_Last_Byte),
        "Pre message RX_BYTE", functionName);
    }
  }

digitalWrite(PIN_LASIG, LOW); // #2
digitalWrite(PIN_LASIG, HIGH);
  // Launch the message
  cmd[0] = M871_CMD_WR_NXT_LD_Q0;
  LTC68042configure_spiWrite(1, cmd);
  timestamp_latestEvent_us = micros();
  latestEventDelay_us = 0;
digitalWrite(PIN_LASIG, LOW); // #3
digitalWrite(PIN_LASIG, HIGH);

  if (messageControl & BITVALUE(MCONT_TX_STATUS_POLL)) {
    // Poll TX_STATUS and wait for TX complete
    //   TX_STATUS.TX_Busy_Status is not set
    //   TX_STATUS.TX_Idle_Status is set
    cmd[0] = M871_READREG_TX_STATUS;
    while (M871_READREG_TX_STATUS_TIMEOUT_us > latestEventDelay_us) {
      LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
      if (    BITVALUE(M871_TX_STATUS_TX_Idle_Status)        // bits expected to be set
           == (  (   BITVALUE(M871_TX_STATUS_TX_Busy_Status) // mask for all bits to check
                   | BITVALUE(M871_TX_STATUS_TX_Idle_Status)
                 ) & rxBuff[0]))
        break;
      latestEventDelay_us = micros() - timestamp_latestEvent_us;
    }
    if (M871_READREG_TX_STATUS_TIMEOUT_us <= latestEventDelay_us) {
      Serial.print(F("  ERROR: timed out waiting for Post TX TX_STATUS. Last status was : 0x"));
      Serial.println(rxBuff[0], HEX);
      allOK = 0;
    }
    else {
      #ifdef WGC_DEBUG_COMMS
      Serial.print(" Delay for Post TX TX_STATUS: ");
      Serial.print(latestEventDelay_us);
      Serial.println(F(" uSec"));
      #endif
      allOK &= MAX1784Xcomms_checkActualVsExpected(
        ( (   BITVALUE(M871_TX_STATUS_TX_Busy_Status)  // mask for all bits to check
            | BITVALUE(M871_TX_STATUS_TX_Idle_Status)
            | BITVALUE(M871_TX_STATUS_TX_Overflow_Status)
          ) & rxBuff[0]),
        BITVALUE(M871_TX_STATUS_TX_Idle_Status),      // bits expected to be set
        "Post TX TX_STATUS", functionName);
    }
  }

  if (messageControl & BITVALUE(MCONT_TX_INT_FLAGS)) {
    // Check for TX success:
    //   TX_INTERRUPT_FLAG.TX_Overflow_Status not set
    //   TX_INTERRUPT_FLAG.POR_Flag not set
    cmd[0] = M871_READREG_TX_INTERRUPT_FLAGS;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    allOK &= MAX1784Xcomms_checkActualVsExpected(
      ( (   BITVALUE(M871_TX_FLAG_POR_Flag)          // mask for all bits to check
          | BITVALUE(M871_TX_FLAG_TX_Overflow_INT_Flag)
        ) & rxBuff[0]),
      M871_CLEAR_ALL,                                // bits expected to be set
      "Post TX TX interrupt flags", functionName);
  }

  if (messageControl & BITVALUE(MCONT_RX_STATUS_POLL)) {
    // Poll RX_Status register until a mesage is received
    //   RX_STATUS.RX_STOP_Status set
    cmd[0] = M871_READREG_RX_STATUS;
    while (M871_READREG_RX_STATUS_TIMEOUT_us > latestEventDelay_us) {
      LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
      if (    BITVALUE(M871_RX_STATUS_RX_STOP_Status)     // bits expected to be set
           == ( (   BITVALUE(M871_RX_STATUS_RX_STOP_Status) // mask for all bits to check
                  | BITVALUE(M871_RX_STATUS_RX_Busy_Status)
                ) & rxBuff[0]))
        break;
    }
    if (M871_READREG_RX_STATUS_TIMEOUT_us <= latestEventDelay_us) {
      Serial.print(F("  ERROR: timed out waiting for Post TX RX_STATUS. Last status was : 0x"));
      Serial.println(rxBuff[0], HEX);
      allOK = 0;
    }
    else {
      #ifdef WGC_DEBUG_COMMS
      Serial.print(" Delay for Post TX RX_STATUS: ");
      Serial.print(latestEventDelay_us);
      Serial.println(F(" uSec"));
      #endif
      // Check for errors in RX_STATUS
      allOK &= MAX1784Xcomms_checkActualVsExpected(
        ( (   BITVALUE(M871_RX_STATUS_RX_Error_Status) // mask for all bits to check
            | BITVALUE(M871_RX_STATUS_RX_Idle_Status)
            | BITVALUE(M871_RX_STATUS_RX_Overflow_Status)
            | BITVALUE(M871_RX_STATUS_RX_Full_Status)
            | BITVALUE(M871_RX_STATUS_RX_Empty_Status)
          ) & rxBuff[0]),
        BITVALUE(M871_RX_STATUS_RX_Idle_Status),       // bits expected to be set
        "Post TX RX_STATUS", functionName);
    }
  }

  uint8_t rx_length = rx_len;
  if (messageControl & BITVALUE(MCONT_RX_LAST_BYTE)) {
    // then we'll be doing all but the last byte in first read
    rx_length--;
  }

  if (messageControl & (BITVALUE(MCONT_RX_FIRST_BYTE) | BITVALUE(MCONT_RX_MESSAGE))) {
    // Start reading of RX message
    cmd[0] = M871_CMD_RD_NXT_MSG;
    LTC68042configure_spiWrite(1, cmd);

    // Read RX_BYTE register and verify First_Byte is set
    cmd[0] = M871_READREG_RX_BYTE;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    allOK &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], BITVALUE(M871_RX_BYTE_First_Byte), "First_Byte", functionName);

digitalWrite(PIN_LASIG, LOW); // #4
digitalWrite(PIN_LASIG, HIGH);
    // Read received message data
    cmd[0] = M871_CMD_RX_RD_POINTER;
    LTC68042configure_spiWriteRead(cmd, 1, rx_data, rx_length);
digitalWrite(PIN_LASIG, LOW); // #5
digitalWrite(PIN_LASIG, HIGH);
  }
  else if (messageControl & BITVALUE(MCONT_RX_MESSAGE)) {
digitalWrite(PIN_LASIG, LOW); // #4
digitalWrite(PIN_LASIG, HIGH);
    // Read received message data
    cmd[0] = M871_CMD_RD_NXT_MSG;
    LTC68042configure_spiWriteRead(cmd, 1, rx_data, rx_length);
digitalWrite(PIN_LASIG, LOW); // #5
digitalWrite(PIN_LASIG, HIGH);
  }
  else {
    //WGCToDo:  would need to clear RX buffer? Not implimented yet
    ;
  }

  if (messageControl & BITVALUE(MCONT_RX_LAST_BYTE)) {
    // Read RX_BYTE register and verify Last_Byte is NOT set
    cmd[0] = M871_READREG_RX_BYTE;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    allOK &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], M871_CLEAR_ALL, "Last_Byte not set", functionName);

    // Read last byte
    cmd[0] = M871_CMD_RX_RD_POINTER;
    LTC68042configure_spiWriteRead(cmd, 1, &(rx_data[rx_len - 1]), 1);

    // Read RX_BYTE register and verify Last_Byte is set
    cmd[0] = M871_READREG_RX_BYTE;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    allOK &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], BITVALUE(M871_RX_BYTE_Last_Byte), "Last_Byte set", functionName);
  }

  if (messageControl & BITVALUE(MCONT_RX_INT_FLAGS)) {
    // Read and check RX_interrupt flags: Expect M871_CLEAR_ALL
    cmd[0] = M871_READREG_RX_INTERRUPT_FLAGS;
    LTC68042configure_spiWriteRead(cmd, 1, rxBuff, 1);
    if (! MAX1784Xcomms_checkActualVsExpected(rxBuff[0], M871_CLEAR_ALL, "Rx flags", functionName)) {
      // then it is not what we expect
      // WGCToDo: This has not been tested
      // clear flags
      cmd[0] = M871_WRITEREG_RX_INTERRUPT_FLAGS;
      cmd[1] = M871_CLEAR_ALL;
      LTC68042configure_spiWrite(2, cmd);
      allOK = 0;
    }
  }
  return allOK;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Check provided data-check value.
//  return comparisson result 1 or 0
bool MAX1784Xcomms_checkDataCheck(
  uint8_t actualDataCheckByte,
  int numDevices,
  int devNum,
  char * functionName)
{
  //save what was last seen for deferred diagnosis
  lastDataCheckByte = actualDataCheckByte;
  lastNumDevices = numDevices;
  lastDeviceNum = devNum;

  if (actualDataCheckByte != m873ExpectedDataCheck) {
    // then it is not what we expect
    Serial.print(F("  ERROR: data-check failure in "));
    Serial.print(functionName);
    if (LAST_DEVICENUM_ALL == devNum) {
        // then data-check error is one or more devices
        Serial.print(F(" on one or more devices"));
    }
    else {
        // data-check error occurred on devNum
        Serial.print(F(" on device "));
        Serial.print(devNum);
    }
    Serial.print(F("! Expected 0x"));
    Serial.print(m873ExpectedDataCheck, HEX);
    Serial.print(F(", got 0x"));
    Serial.print(actualDataCheckByte, HEX);
    uint8_t failedBits = actualDataCheckByte ^ m873ExpectedDataCheck;
    // parse out data-check failed bits
    //WGCToDo: Note that if this is from a readAll, the data-check bits are logical or of all devices
    if (failedBits & BITVALUE(M873_DATACHECK_ALRTPEC)) {
        // also available in individual device STATUS register
        Serial.print(F(" ALRTPEC: Expected "));
        Serial.print((actualDataCheckByte & BITVALUE(M873_DATACHECK_ALRTPEC)) ? F(" 0 got 1") : F(" 1 got 0"));
    }
    if (failedBits & BITVALUE(M873_DATACHECK_ALRTFMEA)) {
        // also available (with more info) from individual device STATUS register
        Serial.print(F(" ALRTFMEA: Expected "));
        Serial.print((actualDataCheckByte & BITVALUE(M873_DATACHECK_ALRTFMEA)) ? F(" 0 got 1") : F(" 1 got 0"));
    }
    if (failedBits & BITVALUE(M873_DATACHECK_ALRTSTATUS)) {
        // need to access individual device STATUS register to find cause
        Serial.print(F(" ALRTSTATUS: Expected "));
        Serial.print((actualDataCheckByte & BITVALUE(M873_DATACHECK_ALRTSTATUS)) ? F(" 0 got 1") : F(" 1 got 0"));
        //WGCToDo: Pick up STATUS register to determine issue
    }
    if (failedBits & BITVALUE(M873_DATACHECK_ALRTOV)) {
        // also available in individual device STATUS register
        Serial.print(F(" ALRTOV: Expected "));
        Serial.print((actualDataCheckByte & BITVALUE(M873_DATACHECK_ALRTOV)) ? F(" 0 got 1") : F(" 1 got 0"));
    }
    if (failedBits & BITVALUE(M873_DATACHECK_ALRTUV)) {
        // also available in individual device STATUS register
        Serial.print(F(" ALRTUV: Expected "));
        Serial.print((actualDataCheckByte & BITVALUE(M873_DATACHECK_ALRTUV)) ? F(" 0 got 1") : F(" 1 got 0"));
    }
    Serial.println();
    return 0;
  }
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Do alive-cound byte checking and seed increment
  // alive-count is garbage until ALIVECNTEN in DEVCFG1 is set.
  // Checking is therefor gated by m873AliveCountSeed: 0 => don't check alive-count yet, else do
bool MAX1784Xcomms_checkAliveCount(int aliveCount, int aliveCountIncrement, char * functionName)
{
  bool aliveCountValid = 1;

  if (m873AliveCountSeed) {
    if (! MAX1784Xcomms_checkActualVsExpected(aliveCount, (m873AliveCountSeed + aliveCountIncrement), "alive-count", functionName)) {
      // then this is an error
      aliveCountValid = 0;
    }
    m873AliveCountSeed = 0xff & (m873AliveCountSeed + 1);
    if (250 <= m873AliveCountSeed) // need to leave room to keep it from rolling over
      MAX1784Xcomms_enableAliveCount();
  }
  return aliveCountValid;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Send the HELLOALL command up the daisy chain to assigh each device its owh
//   unique address, and (incidentally) determine the number of devices
int MAX1784Xcomms_enumerateDevices(void)
{
  bool messageValid = 1; //WGCToDo: Since I'm returning devCount, there is no place to return this...
  uint8_t cmd[5];
  uint8_t rxBuff[3];
  int devCount;

  // Max17841Ds Tbl10Txn10: Send the Helloall command to initialize the address of the BMS daisy chain
  //   WR_LD_Q0 starts loading the current Load Queue 0 at byte 0 (message length)
  //   0 for register address (dummy value?)
  //   0 for starting address
  cmd[0] = M871_CMD_WR_LD_Q0;
  cmd[1] = MESSAGE_LEN_3;
  cmd[2] = M873_HELLOALL;
  cmd[3] = 0;
  cmd[4] = 0; // aka First Address (here 0)

  //WGCToDo speedup: Reduce checking here?
  MAX17843_message873Workhorse(cmd, 5, rxBuff, 3, MCONT_FULL_CHECKS, __func__);

  // Check message content vs expected
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], M873_HELLOALL, "command return", __func__);
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[1], 0, "expected null byte", __func__);

  devCount = rxBuff[2];

  // WGCToDo: debug print?
//  Serial.print(F("Number of devices in daisy chain is : "));
//  Serial.println(devCount);
  return devCount;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Read a single register from all devices
//   register given by regAddr
//   register values left in resultBuff[]
//  Returns false if there is a message error
bool MAX1784Xcomms_readAll843Reg(int regAddr, int Device_count, uint16_t * resultBuff, int messageControl)
{
  bool messageValid = 1;
  uint8_t cmd[7];
  uint8_t rxBuff[16];
  uint8_t msgLength = 5 + (2 * Device_count);
  uint8_t rxDataCheck, rxPEC, rxAliveCount;

  cmd[0] = M871_CMD_WR_LD_Q0;
  cmd[1] = msgLength;
  cmd[2] = M873_READALL;
  cmd[3] = regAddr;
  cmd[4] = DATA_CHECK_SEED_0;
  cmd[5] = LTC68042configure_calcPEC15(3, &(cmd[2]));
  cmd[6] = m873AliveCountSeed;

  messageValid &= MAX17843_message873Workhorse(cmd, 7, rxBuff, msgLength,
    (messageControl | BITVALUE(MCONT_RX_MESSAGE)), __func__);

  // Check message content vs expected
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], M873_READALL, "command return", __func__);
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[1], regAddr, "register return", __func__);

  // Pick up message check items from buffer
  rxDataCheck  = rxBuff[msgLength - 3];
  rxPEC        = rxBuff[msgLength - 2];
  rxAliveCount = rxBuff[msgLength - 1];

  // Check the calculated and hardware returned PEC (should be valid for any received message)
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxPEC, LTC68042configure_calcPEC15(msgLength - 2, rxBuff), "PEC", __func__);
  messageValid &= MAX1784Xcomms_checkDataCheck(rxDataCheck, Device_count, LAST_DEVICENUM_ALL, __func__);
  messageValid &= MAX1784Xcomms_checkAliveCount(rxAliveCount, Device_count, __func__);

  // pick up data payload read back from transfer buffer
  // Note that bytes are in reverse device order, LSB first
  for (int DAx = 0; DAx < Device_count; DAx++) {
    int regOffset = 2 * (Device_count - DAx);  // offset of LSB for given DAx
    resultBuff[DAx] = (rxBuff[1 + regOffset] << 8) + rxBuff[regOffset];
  }

  return messageValid;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Read a single register from a single device
//   register given by regAddr
//   device to read from given by devNum
//   register values left in resultBuff[devNum]
//  Returns false if there is a message error
bool MAX1784Xcomms_readDev843Reg(int regAddr, int devNum, uint16_t * resultBuff, int messageControl)
{
  bool messageValid = 1;
  uint8_t cmd[7];
  uint8_t rxBuff[7];
  uint8_t rxDataCheck, rxPEC, rxAliveCount;

  cmd[0] = M871_CMD_WR_LD_Q0;
  cmd[1] = MESSAGE_LEN_7;
  cmd[2] = m873readDevice[devNum];
  cmd[3] = regAddr;
  cmd[4] = DATA_CHECK_SEED_0;
  cmd[5] = LTC68042configure_calcPEC15(3, &(cmd[2]));
  cmd[6] = m873AliveCountSeed;

  messageValid &= MAX17843_message873Workhorse(cmd, 7, rxBuff, 7,
    (messageControl | BITVALUE(MCONT_RX_MESSAGE)), __func__);

  // Check message content vs expected
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], m873readDevice[devNum], "command return", __func__);
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[1], regAddr, "register return", __func__);

  // Pick up message check items from buffer
  rxDataCheck  = rxBuff[4];
  rxPEC        = rxBuff[5];
  rxAliveCount = rxBuff[6];

  // Check the calculated and hardware returned PEC (should be valid for any received message)
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxPEC, LTC68042configure_calcPEC15(5, rxBuff), "PEC", __func__);
  messageValid &= MAX1784Xcomms_checkDataCheck(rxDataCheck, 1, devNum, __func__);
  messageValid &= MAX1784Xcomms_checkAliveCount(rxAliveCount, 1, __func__);

  // pick up data payload read back from transfer buffer
  *resultBuff = (rxBuff[3] << 8) + rxBuff[2];

  return messageValid;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Read a range of sequential registers from a single device
//   starting register given by startRegAddr
//   number of registers to read given by blockSize
//   device to read from given by devNum
//   register values put into resultBuff[]
//  Returns false if there is a message error
bool MAX1784Xcomms_readBlock843(int startRegAddr, int blockSize, int devNum, uint16_t * resultBuff, int messageControl)
{
  bool messageValid = 1;
  uint8_t cmd[8];
  uint8_t rxBuff[62]; //WGCToDo: 62 is the maximum without getting complicated. Might need less...
  uint8_t rxDataCheck, rxPEC, rxAliveCount;

  uint8_t readBlockCmd = (blockSize << 3) | M873_READBLOCK;
  uint8_t msgLength = 6 + (2 * blockSize);

  //WGCToDo: add a check for msgLength not greater than rxBuff size. Fatal error if so...

  cmd[0] = M871_CMD_WR_LD_Q0;
  cmd[1] = msgLength;
  cmd[2] = readBlockCmd;
  cmd[3] = (uint8_t)devNum;
  cmd[4] = startRegAddr;
  cmd[5] = DATA_CHECK_SEED_0;
  cmd[6] = LTC68042configure_calcPEC15(4, &(cmd[2]));
  cmd[7] = m873AliveCountSeed;

  messageValid &= MAX17843_message873Workhorse(cmd, 8, rxBuff, msgLength,
    (messageControl | BITVALUE(MCONT_RX_MESSAGE)), __func__);

  // Check message content vs expected
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], readBlockCmd, "command return", __func__);
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[1], devNum, "device return", __func__);
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[2], startRegAddr, "start register return", __func__);

  // Pick up message check items from buffer
  rxDataCheck  = rxBuff[msgLength - 3];
  rxPEC        = rxBuff[msgLength - 2];
  rxAliveCount = rxBuff[msgLength - 1];

  // Check the calculated and hardware returned PEC (should be valid for any received message)
  messageValid &= MAX1784Xcomms_checkActualVsExpected(rxPEC, LTC68042configure_calcPEC15(msgLength - 2, rxBuff), "PEC", __func__);
  messageValid &= MAX1784Xcomms_checkDataCheck(rxDataCheck, 1, devNum, __func__);
  messageValid &= MAX1784Xcomms_checkAliveCount(rxAliveCount, 1, __func__);

  // pick up data payload read back from transfer buffer
  for (int RAx = 0; RAx < blockSize; RAx++) {
    resultBuff[RAx] = (rxBuff[4 + (2 * RAx)] << 8) + rxBuff[3 + (2 * RAx)];
  }

  // more checks (after payload pickup - these might overwrite buffer)

  return messageValid;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Write a single resister to all devices
//   register given by regAddr
//  Returns false if there is a message error
bool MAX1784Xcomms_writeAll843Reg(int regAddr, int Device_count, int regValue, int messageControl)
{
  bool messageValid = 1;
  uint8_t cmd[8];
  uint8_t rxBuff[6];
  uint8_t rxPEC, rxAliveCount;

  cmd[0] = M871_CMD_WR_LD_Q0;
  cmd[1] = MESSAGE_LEN_6;
  cmd[2] = M873_WRITEALL;
  cmd[3] = regAddr;
  cmd[4] = lowByte(regValue);
  cmd[5] = highByte(regValue);
  cmd[6] = LTC68042configure_calcPEC15(4, &(cmd[2]));
  cmd[7] = m873AliveCountSeed;

  messageValid &= MAX17843_message873Workhorse(cmd, 8, rxBuff, 6, messageControl, __func__);

  if (messageControl & BITVALUE(MCONT_RX_MESSAGE)) {
    // Check message content vs expected
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], M873_WRITEALL, "command return", __func__);
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[1], regAddr, "register return", __func__);
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[2], lowByte(regValue), "register LSB value return", __func__);
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[3], highByte(regValue), "register MSB value return", __func__);

    // Pick up message check items from buffer
    rxPEC        = rxBuff[4];
    rxAliveCount = rxBuff[5];

    // Check the calculated and hardware returned PEC
    //WGCToDo: Recalculate PEC, or use orriginal PEC_VALUE?
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxPEC, LTC68042configure_calcPEC15(4, rxBuff), "PEC", __func__);
    messageValid &= MAX1784Xcomms_checkAliveCount(rxAliveCount, Device_count, __func__);

    // no payload data to pick up for a write
  }

  return messageValid;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Write a single resister to one devices
//   register given by regAddr
//   device to wrote to given by devNum
//  Returns false if there is a message error
bool MAX1784Xcomms_writeDev843Reg(int regAddr, int devNum, int regValue, int messageControl)
{
  bool messageValid = 1;
  uint8_t cmd[8];
  uint8_t rxBuff[6];
  uint8_t rxPEC, rxAliveCount;

  cmd[0] = M871_CMD_WR_LD_Q0;
  cmd[1] = MESSAGE_LEN_6;
  cmd[2] = m873writeDevice[devNum];
  cmd[3] = regAddr;
  cmd[4] = lowByte(regValue);
  cmd[5] = highByte(regValue);
  cmd[6] = LTC68042configure_calcPEC15(4, &(cmd[2]));
  cmd[7] = m873AliveCountSeed;

  messageValid &= MAX17843_message873Workhorse(cmd, 8, rxBuff, 6, messageControl, __func__);

  if (messageControl & BITVALUE(MCONT_RX_MESSAGE)) {
    // Check message content vs expected
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[0], m873writeDevice[devNum], "command return", __func__);
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[1], regAddr, "register return", __func__);
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[2], lowByte(regValue), "register LSB value return", __func__);
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxBuff[3], highByte(regValue), "register MSB value return", __func__);

    // Pick up message check items from buffer
    rxPEC        = rxBuff[4];
    rxAliveCount = rxBuff[5];

    // Check the calculated and hardware returned PEC
    //WGCToDo: Recalculate PEC, or use orriginal PEC_VALUE?
    messageValid &= MAX1784Xcomms_checkActualVsExpected(rxPEC, LTC68042configure_calcPEC15(4, rxBuff), "PEC", __func__);
    messageValid &= MAX1784Xcomms_checkAliveCount(rxAliveCount, 1, __func__);

    // no payload data to pick up for a write
  }

  return messageValid;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Dump out and then clear device status registers
void MAX1784Xcomms_dumpAndClearStatus(int devNum)
{
    uint16_t regValue;

    // Read STATUS, and verify all just have M873_STATUS_ALRTRST set, with data-check of DATA_CHECK_EXPECTED_POR
    Serial.print(F("  For device "));
    Serial.print(devNum);

    // status register
    Serial.print(F(", STATUS reg: "));
    MAX1784Xcomms_readDev843Reg(M873_STATUS, devNum, &regValue, MCONT_FULL_CHECKS);
    Serial.print(regValue, HEX);
    if (M873_CLEAR_ALL != regValue) {
        MAX1784Xcomms_writeDev843Reg(M873_STATUS, devNum, M873_STATUS_INIT, MCONT_FULL_CHECKS);
        MAX1784Xcomms_setExpectedDataCheck(DATA_CHECK_EXPECTED_NORMAL);
    }

    // FMEA1 register
    Serial.print(F(", FMEA1 reg: "));
    MAX1784Xcomms_readDev843Reg(M873_FMEA1, devNum, &regValue, MCONT_FULL_CHECKS);
    Serial.print(regValue, HEX);
    if (M873_CLEAR_ALL != regValue) {
        MAX1784Xcomms_writeDev843Reg(M873_FMEA1, devNum, M873_CLEAR_ALL, MCONT_FULL_CHECKS);
    }

    // FMEA2 register
    Serial.print(F(", FMEA2 reg: "));
    MAX1784Xcomms_readDev843Reg(M873_FMEA2, devNum, &regValue, MCONT_FULL_CHECKS);
    Serial.print(regValue, HEX);
    if (M873_CLEAR_ALL != regValue) {
        MAX1784Xcomms_writeDev843Reg(M873_FMEA2, devNum, M873_CLEAR_ALL, MCONT_FULL_CHECKS);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////
// Look at last error, and diagnose cause
void MAX1784Xcomms_diagnoseErrors(char * functionName)
{
    //at the moment, this only applies to data-check errors
    if (lastDataCheckByte != m873ExpectedDataCheck) {
        // OK, we can now expect data-check will be lastDataCheckByte, until STATUS is cleared
        m873ExpectedDataCheck = lastDataCheckByte;
        if (LAST_DEVICENUM_ALL == lastDeviceNum) {
            // then data-check error is one or more devices
            for (int DAx = 0; DAx < lastNumDevices; DAx++) {
                MAX1784Xcomms_dumpAndClearStatus(DAx);
            }
        }
        else {
            // data-check error occurred on lastDeviceNum
            MAX1784Xcomms_dumpAndClearStatus(lastDeviceNum);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
// MAX17843 general configuration
void MAX1784Xcomms_setup843Registers(int Device_count)
{
  // Set STATUS to 0X00 to clear flags
  MAX1784Xcomms_writeAll843Reg(M873_STATUS, Device_count, M873_STATUS_INIT, MCONT_FULL_CHECKS);

  // since STATUS has now been cleared, from this point on data-check should be DATA_CHECK_EXPECTED_NORMAL
  MAX1784Xcomms_setExpectedDataCheck(DATA_CHECK_EXPECTED_NORMAL);

  // Set DEVCFG1
  MAX1784Xcomms_writeAll843Reg(M873_DEVCFG1, Device_count, M873_DEVCFG1_INIT, MCONT_FULL_CHECKS);

  // since DEVCFG1 has now been set, from this point on we can do alive count checking
  MAX1784Xcomms_enableAliveCount();

  //WGCToDo speedup: These flags should already be M873_CLEAR_ALL after POR
  // Set FMEA1 to 0X00 to clear flags
  MAX1784Xcomms_writeAll843Reg(M873_FMEA1, Device_count, M873_CLEAR_ALL, MCONT_FULL_CHECKS);

  // Set FMEA2 TO 0X00 to clear flags
  MAX1784Xcomms_writeAll843Reg(M873_FMEA2, Device_count, M873_CLEAR_ALL, MCONT_FULL_CHECKS);
  //WGCToDo speedup: end of setup to skip

  // Set measurement enables (MEASUREEN) and acquisition parameters
  MAX1784Xcomms_writeAll843Reg(M873_MEASUREEN, Device_count, M873_MEASUREEN_INIT, MCONT_FULL_CHECKS);

  // set ACQCFG
  MAX1784Xcomms_writeAll843Reg(M873_ACQCFG, Device_count, M873_ACQCFG_INIT, MCONT_FULL_CHECKS);

  // set DIAGCFG
  MAX1784Xcomms_writeAll843Reg(M873_DIAGCFG, Device_count, M873_DIAGSEL_DieTemperature, MCONT_FULL_CHECKS);

  // set ADR
  MAX1784Xcomms_writeAll843Reg(M873_ADR, Device_count, M873_ADR_INIT, MCONT_FULL_CHECKS);

  // and wake-up is complete
  MAX1784Xcomms_setJustWokeUpState(false);
}


/////////////////////////////////////////////////////////////////////////////////////////
