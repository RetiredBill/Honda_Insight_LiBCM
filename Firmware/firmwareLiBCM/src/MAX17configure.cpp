// MAX17xxx configuration functions
// Based on LTC68042configure.cpp, Copyright 2021-2024(c) John Sullivan
//    github.com/doppelhub/Honda_Insight_LiBCM

// MAX17843 is 12-cell BMS management IC, which uses differential UART busses for communication.
// MAX17841 is a differential UART-to-SPI bridge IC

// Note: "LTC68042" kept in public function names to conform to existing API

//JTS2doLater: Add 2nd reference measurement (to verify 1st reference is working properly)

#include "libcm.h"

#ifndef BMS_TYPE_WGCLiBCM
  uint32_t lastTimeDataSent_millis = 0; //LTC idle timer resets each time data is transferred

  //Stores configuration register data to write to IC
  uint8_t configurationRegisterData[6]; //[CFGR0, CFGR1, CFGR2, CFGR3, CFGR4, CFGR5]
#else
  uint32_t lastMAX1784xTimestamp_millis = 0; // for optimizing delays
#endif
/////////////////////////////////////////////////////////////////////////////////////////

void MAX17841configure_enableMAX17841(void) {
  digitalWrite(PIN_SHDNL_MAX17841, HIGH); // Enable MAX17841
  lastMAX1784xTimestamp_millis = millis(); // will need to do t_startup delay
}

/////////////////////////////////////////////////////////////////////////////////////////

void MAX17841configure_disableMAX17841(void) {
  digitalWrite(PIN_SHDNL_MAX17841, LOW); // disable MAX17841
  lastMAX1784xTimestamp_millis = millis(); // will need to do t_shutdown delay
}

/////////////////////////////////////////////////////////////////////////////////////////

//For BMS_TYPE_WGCLiBCM, LTC68042configure_writeConfigRegisters is #ifndef'd out
//WGCToDo: could resurect this if a need for shadow registers arrises...
//Write LTC6804 configuration registers
//if (icAddress == BROADCAST_TO_ALL_ICS), this function broadcasts the same data to all LTC6804 ICs
//
// | config[0] | config[1] | config[2] | config[3] | config[4] | config[5] |
// |-----------|-----------|-----------|-----------|-----------|-----------|
// | IC CFGR0  | IC CFGR1  | IC CFGR2  | IC CFGR3  | IC CFGR4  | IC CFGR5  |

void LTC68042configure_writeConfigRegisters(uint8_t icAddress)
{
  #ifndef BMS_TYPE_WGCLiBCM
    const uint8_t BYTES_IN_REG = 6;
    const uint8_t CMD_LENGTH = 2+2+6+2; //("Write Configuration Registers" command) + (PEC) + ("configuration register" data) + (PEC)
    uint8_t cmd[CMD_LENGTH];

    //Load cmd array with WRCFG command and PEC
    if (icAddress == BROADCAST_TO_ALL_ICS) { cmd[0] = 0x00; } //0b00000xxx indicates this is a broadcast command
    else                                   { cmd[0] = 0x80 + (icAddress << 3); } //see datasheet Tables 33 & 34

    cmd[1] = 0x01; //send "write configuration registers" command ('WRCFG')

    uint16_t temp_pec = LTC68042configure_calcPEC15(2, cmd); //calculate PEC

    cmd[2] = (uint8_t)(temp_pec >> 8); //upper PEC byte
    cmd[3] = (uint8_t)(temp_pec); //lower PEC byte

    uint8_t cmd_index = 4; //stored byte index in the cmd array

    //add the "configuration register" bytes (CFGR0:5) to the cmd array
    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) { cmd[cmd_index++] = configurationRegisterData[current_byte]; }

    //Calculate the PEC for the LTC6804 configuration register bytes
    temp_pec = LTC68042configure_calcPEC15(BYTES_IN_REG, &configurationRegisterData[0]);// calculate the PEC
    cmd[cmd_index++] = (uint8_t)(temp_pec >> 8); //upper PEC byte
    cmd[cmd_index++] = (uint8_t)temp_pec; //lower PEC byte

    LTC68042configure_spiWrite(CMD_LENGTH, cmd);
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

//configure discharge resistor states on a single LTC6804 IC (CFGR4:5)
void LTC68042configure_setBalanceResistors(uint8_t icAddress, uint16_t cellBitmap, uint8_t softwareTimeout)
{
    //Each bit in cellBitmap corresponds to a specific cell's DCCn discharge bit
    //Example: cellBitmap = 0b0000 1000 0000 0011 enables discharge on cells 12, 2, and 1 //LSB is cell01
    //Example: cellBitmap = 0b0000 1111 1111 1111 enables discharge on all cells
  #ifndef BMS_TYPE_WGCLiBCM
    //See Table36
    configurationRegisterData[4] = (uint8_t)(cellBitmap); //LSByte
    configurationRegisterData[5] = ( ((uint8_t)(cellBitmap >> 8)) | softwareTimeout ); //MSByte's lower nibble

    MAX17843configure_writeConfigRegisters(icAddress);
  #else
    //WGCToDo: softwareTimeout is LTC6804_DISCHARGE_TIMEOUT_02_SECONDS, which is 0! (as of 2/9/25)
    // Set up watchdog timer for 2 sec
    MAX1784Xcomms_writeDev843Reg(M873_TIMERCFG, icAddress, (BFN_GET(M873_TIMERCFG_bfCBPDIV, 1) | BFN_GET(M873_TIMERCFG_bfCBTIMER, 2)), MCONT_FULL_CHECKS);
    //set cell switch bits
    MAX1784Xcomms_writeDev843Reg(M873_BALSWEN, icAddress, BFN_GET(M873_BALSWEN_bfBALSWEN, cellBitmap), MCONT_FULL_CHECKS);
    //WGCToDo: might be time for a shadow register...
    if (cellBitmap) {
      // then enable cell balance (clear DEVCFG1.BALSWDISABLE)
      MAX1784Xcomms_writeDev843Reg(M873_DEVCFG1, icAddress, (~BITVALUE(M873_DEVCFG1_BALSWDISABLE)) & M873_DEVCFG1_INIT, MCONT_FULL_CHECKS);
    }
    else {
      //disable cell balance (set DEVCFG1.BALSWDISABLE)
      MAX1784Xcomms_writeDev843Reg(M873_DEVCFG1, icAddress, BITVALUE(M873_DEVCFG1_BALSWDISABLE) | M873_DEVCFG1_INIT, MCONT_FULL_CHECKS);
    }
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// Compared to BMS_TYPE_WGCLiBCM, LiBCM version of this is relatively fast (12 SPI bytes => 384us @ 32us/byte),
//   whereas for BMS_TYPE_WGCLiBCM, wakeup() alone takes ~4.3ms! Total is ~6.8ms
//Initialize BMS system to "fresh start" state
void LTC68042configure_programVolatileDefaults(void)
{
  bool allOk = true;
  char msg[30];
  uint32_t moduleId[TOTAL_IC];
  uint16_t registerValue[TOTAL_IC];

  // Initialize all MAX17843 chips via reset.
  // If we are executing after a POR,
  //   then the reset is done (and we'll verify this),
  // else do the POR reset here
  //WGCToDoNow: Options to initialize 871:
  //  Pulse SHDNL (inducing POR), then write 4 selected registers (2 SPI bytes each), and 1 command (1 SPI byte)
  //    1uSec tau on SHDNl line, so low time should be ... This actually requires significant delay, so:
  //  Just write a 7 byte block (2 SPI bytes), and 1 command (1 SPI byte)
  //  => quicker to write 7 1 byte registers
  //WGCToDo: Use LTC68042configure_wakeup() instead?
  if ( (! MAX1784Xcomms_justWokeUp()) ||
       (! MAX1784Xcomms_max17841_CheckForPOR()) ) { //WGCToDo: may not also need this condition
    // then a full POR needs to be forced
    MAX1784Xcomms_max17843_reset();
    //NB: want to get the MAX1784Xcomms_max17841_Init() done promptly, to keep the keep-alive going
  }
  // else the expectation is that MAX17841 and 843 chips are waking from off state
  //WGCToDo: this and delay() could be optimized; isn't always needed
  MAX17841configure_enableMAX17841();
  //WGCToDo: convert blocking delays to timestamps/ready checks
  delay(M871_STARTUP_TIME_ms); //WGCToDo: some delay is needed here, maybe not a full 2ms
  MAX1784Xcomms_max17841_Init();

  //WGCToDo: convert blocking delays to timestamps/ready checks
  while(millis() - lastMAX1784xTimestamp_millis < M871_STARTUP_TIME_ms) { ; }
  MAX1784Xcomms_wakeup();                // Wake up instructions for start up
  // For BMS_TYPE_WGCLiBCM, "Hello all" command is performed in (modified)
  //   LTC68042configure_doesActualPackSizeMatchUserConfig(), since "Hello all" is required
  //   to initialze MAX17843 devices, and it inherently checks the number of
  //   battery modules found.
  allOk &= LTC68042configure_doesActualPackSizeMatchUserConfig();

  // Note: since ALRTRST bit in STATUS register has not been cleared yet, read back Data check bytes will not be 0
  MAX1784Xcomms_setExpectedDataCheck(DATA_CHECK_EXPECTED_POR);

  // Note: since ALIVECNTEN bit in DEVCFG1 register has not benn set yet, Alive counter byte is not useful yet.
  MAX1784Xcomms_disableAliveCount();  // disable alive-count checking for now

  //WGCToDo speedup: Move checks to end of coldboot or sometime later
  // These all should return a data-check of DATA_CHECK_EXPECTED_POR
  // Get LSB of ID from ID1
  MAX1784Xcomms_readAll843Reg(M873_ID1, TOTAL_IC, registerValue, MCONT_FULL_CHECKS);
  for (int DAx = 0; DAx < TOTAL_IC; DAx++) {
    moduleId[DAx] = registerValue[DAx];
  }
  // Get MSB of ID and ROM CRC from ID2
  MAX1784Xcomms_readAll843Reg(M873_ID2, TOTAL_IC, registerValue, MCONT_FULL_CHECKS);
  Serial.println();
  for (int DAx = 0; DAx < TOTAL_IC; DAx++) {
    moduleId[DAx] += ((uint32_t)BFN_GET(M873_ID2_bfDEVIDMsb, registerValue[DAx]) << 16);
    Serial.print(F("Device "));
    Serial.print(DAx);
    Serial.print(F(" has ID: 0x"));
    Serial.print(moduleId[DAx], HEX);
    Serial.print(F(" and ROM CRC: 0x"));
    Serial.println(BFN_GET(M873_ID2_bfROMCRC, registerValue[DAx]), HEX);
  }

  strcpy(msg, "device ");
  msg[7] = '0';
  strcpy(&(msg[8]), " Model/version");
  MAX1784Xcomms_readAll843Reg(M873_VERSION, TOTAL_IC, registerValue, MCONT_FULL_CHECKS);
  for (int DAx = 0; DAx < TOTAL_IC; DAx++) {
    msg[7] = (char)DAx + '0';
    allOk &= MAX1784Xcomms_checkActualVsExpected(registerValue[DAx], M873_MODEL_VERSION, msg, __func__);
  }

  // Read STATUS, and verify all just have M873_STATUS_ALRTRST set, with data-check of DATA_CHECK_EXPECTED_POR
  strcpy(&(msg[8]), " STATUS");
  MAX1784Xcomms_readAll843Reg(M873_STATUS, TOTAL_IC, registerValue, MCONT_FULL_CHECKS);
  for (int DAx = 0; DAx < TOTAL_IC; DAx++) {
    msg[7] = (char)DAx + '0';
    allOk &= MAX1784Xcomms_checkActualVsExpected(registerValue[DAx], BITVALUE(M873_STATUS_ALRTRST), msg, __func__);
  }

  // Verify that FMEA1, FMEA2 values are all M873_CLEAR_ALL (should be after POR. Maybe only a warning?)
  strcpy(&(msg[8]), " FMEA1");
  MAX1784Xcomms_readAll843Reg(M873_FMEA1, TOTAL_IC, registerValue, MCONT_FULL_CHECKS);
  for (int DAx = 0; DAx < TOTAL_IC; DAx++) {
    msg[7] = (char)DAx + '0';
    allOk &= MAX1784Xcomms_checkActualVsExpected(registerValue[DAx], M873_CLEAR_ALL, msg, __func__);
  }
  strcpy(&(msg[8]), " FMEA2");
  MAX1784Xcomms_readAll843Reg(M873_FMEA2, TOTAL_IC, registerValue, MCONT_FULL_CHECKS);
  for (int DAx = 0; DAx < TOTAL_IC; DAx++) {
    msg[7] = (char)DAx + '0';
    allOk &= MAX1784Xcomms_checkActualVsExpected(registerValue[DAx], M873_CLEAR_ALL, msg, __func__);
  }
  //WGCToDo speedup: end of checks to defer

  // configure all MAX17842 registers
  MAX1784Xcomms_setup843Registers(TOTAL_IC);
}

/////////////////////////////////////////////////////////////////////////////////////////

// For MAX17843, must do a helloall() to start, which returns the
//   number of devices found. So, this function WILL be called when LiBCM first boots.
//WGCToDo: "we don't have time to run this test if the key is on when LiBCM first boots" may be
//   an issue for doing the helloall(): it may take too long at first boot.
bool LTC68042configure_doesActualPackSizeMatchUserConfig(void)
{
    bool helper_doesActualPackSizeMatchUserConfig = true;

    #if   defined RUN_BRINGUP_TESTER_MOTHERBOARD //don't verify cell count
    #elif defined RUN_BRINGUP_TESTER_GRIDCHARGER //don't verify cell count
    #else
      #ifndef BMS_TYPE_WGCLiBCM
        if (gpio_keyStateNow() == GPIO_KEY_OFF) //we don't have time to run this test if the key is on when LiBCM first boots
        {
            LTC6804_adax(); //send any broadcast command
            delay(6); //wait for all LTC6804 ICs to process this command

            uint8_t errorCount_LTC6804_underTest[TOTAL_IC_60S] = {0}; //allocate for 60S even when user selects 48S

            //read data back from either QTY4 ICs (if user selects PACK_IS_48S in config.h), or QTY5 ICs (if user selects PACK_IS_60S in config.h)
            //we don't care about the actual returned data; only that the PEC error count doesn't increment
            for (uint8_t dut = 0; dut < TOTAL_IC; dut++)
            {
                errorCount_LTC6804_underTest[dut] = LTC6804_rdaux(1,1,FIRST_IC_ADDR + dut); //read register 'A' on specified LTC6804
                if (errorCount_LTC6804_underTest[dut] != 0) { helper_doesActualPackSizeMatchUserConfig = false; }
            }

            //For 48S, verify cells 49:60 aren't present
            if (TOTAL_IC == TOTAL_IC_48S)
            {
                errorCount_LTC6804_underTest[4] = LTC6804_rdaux(1,1,FIRST_IC_ADDR + TOTAL_IC_48S); //attempt to read from 49:60

                if (errorCount_LTC6804_underTest[4] == 0) { helper_doesActualPackSizeMatchUserConfig = false; } //49:60 present
            }
      #else // (and beware of the different code block level!)
        int Device_count = 0;

        // Hello all command intializes the device address of the ICs in daisy chain.
        // The number of devices that responded is returned
        Device_count = MAX1784Xcomms_enumerateDevices();
        if (TOTAL_IC != Device_count) { helper_doesActualPackSizeMatchUserConfig = false; }
      #endif
            if (helper_doesActualPackSizeMatchUserConfig == false)
            {
                //fatal error
                //alert user and then turn off

                Serial.print(F("\nError: measured cell count disagrees with user specified cell count in config.h."
                               "\nLiBCM is disabled due to cell voltage monitoring IC issue. Debug:"));
      //WGCToDo: add print of Device_count on failure
      #ifndef BMS_TYPE_WGCLiBCM
                //cells 1:48 are the same for both 48S & 60S
                for (uint8_t dut = 0; dut < TOTAL_IC; dut++)
                {
                    Serial.print(F("\nIC"));
                    Serial.print(dut);
                    if (errorCount_LTC6804_underTest[dut] == 0) { Serial.print(F(": pass")); }
                    else                                        { Serial.print(F(": FAIL")); }
                }

                //For 48S, verify cells 49:60 aren't present
                Serial.print("\nIC4: ");
                if (TOTAL_IC == TOTAL_IC_48S)
                {
                    if (errorCount_LTC6804_underTest[4] == 0) { Serial.print(F("FAIL")); } //IC4 powered by cells 49:60
                    else                                      { Serial.print(F("pass")); }
                }
      #endif
                lcdTransmit_begin();
                delay(50); //delay doesn't matter because this is a fatal error
                lcdTransmit_displayOn();
                delay(50); //delay doesn't matter because this is a fatal error
                lcdTransmit_Warning(LCD_WARN_CELL_COUNT);

                gpio_turnBuzzer_on_highFreq(); //call GPIO directly

                wdt_disable(); //turn off watchdog to prevent reset
                wdt_enable(WDTO_8S);

                delay(7000); //give the user enough time to read error message

                gpio_turnLiBCM_off(); //game over... thanks for playing
            }
      #ifndef BMS_TYPE_WGCLiBCM
        }
      #endif
    #endif

    return helper_doesActualPackSizeMatchUserConfig;
}

/////////////////////////////////////////////////////////////////////////////////////////

void LTC68042configure_initialize(void)
{
    MAX17841configure_enableMAX17841(); // get a head start on tstartup delay
    pinMode(PIN_SHDNL_MAX17841, OUTPUT);
    MAX1784Xcomms_setJustWokeUpState(true);
    spi_enable(SPI_CLOCK_DIV64); //JTS2doLater: increase clock speed //DIV16 & DIV32 work on bench
}

/////////////////////////////////////////////////////////////////////////////////////////

// For BMS_TYPE_WGCLiBCM, LTC68042configure_pulseChipSelectLow is #ifndef'd out, never called
void LTC68042configure_pulseChipSelectLow(uint16_t lowPulsePeriod_us)
{
  #ifndef BMS_TYPE_WGCLiBCM
    digitalWrite(PIN_SPI_CS,LOW); //low edge wakes up LTC
    delayMicroseconds(lowPulsePeriod_us); //wait specified time for LTC to wake
    digitalWrite(PIN_SPI_CS,HIGH);
    lastTimeDataSent_millis = millis();
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// For BMS_TYPE_WGCLiBCM, LTC68042configure_wakeupCore is #ifndef'd out, never called.
//   MAX1784x chips are automatically kept awake by MAX17841 keep-alive function.
bool LTC68042configure_wakeupCore(void)
{
  #ifndef BMS_TYPE_WGCLiBCM
    const uint16_t T_SLEEP_WATCHDOG_MILLIS = 1800; //'tsleep' = 1800 (min) to 2200 (max) ms

    bool wasCoreAlreadyAwake = LTC6804_CORE_ALREADY_AWAKE;

    if ((uint32_t)(millis() - lastTimeDataSent_millis) > T_SLEEP_WATCHDOG_MILLIS)
    {
        //LTC6804 core (probably) asleep
        LTC68042configure_pulseChipSelectLow(SPECIFIED_MAX_WAKEUP_TIME_LTCCORE_MICROSECONDS);
        wasCoreAlreadyAwake = LTC6804_CORE_JUST_WOKE_UP;
    }

    return wasCoreAlreadyAwake;
  #else
    return LTC6804_CORE_ALREADY_AWAKE; // just to be safe...
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// For BMS_TYPE_WGCLiBCM, LTC68042configure_wakeupIsoSPI is #ifndef'd out, never called.
//   MAX1784x chips are automatically kept awake by MAX17841 keep-alive function.
void LTC68042configure_wakeupIsoSPI(void)
{
  #ifndef BMS_TYPE_WGCLiBCM
    const uint8_t T_IDLE_isoSPI_MILLIS = 4; //'tIDLE' = 4.3 (min) to 6.7 (max) ms

    if ((uint32_t)(millis() - lastTimeDataSent_millis) > T_IDLE_isoSPI_MILLIS)
    {
        //LTC6804 isoSPI might be asleep (tIDLE elapsed)
         LTC68042configure_pulseChipSelectLow(SPECIFIED_MAX_WAKEUP_TIME_isoSPI_MICROSECONDS);
    }
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

// For BMS_TYPE_WGCLiBCM, MAX1784x chips are automatically kept awake by MAX17841 keep-alive function,
//   and this function now just returns the JUST_WOKE_UP vs ALREADY_AWAKE state;
//   it doesn't wake anything up...
bool LTC68042configure_wakeup(void)
{
  #ifndef BMS_TYPE_WGCLiBCM
    bool wasCoreAlreadyAwake = LTC68042configure_wakeupCore();

    if (wasCoreAlreadyAwake == LTC6804_CORE_ALREADY_AWAKE) { LTC68042configure_wakeupIsoSPI(); }

    return wasCoreAlreadyAwake;
  #else
    if (MAX1784Xcomms_justWokeUp()) { return LTC6804_CORE_JUST_WOKE_UP;  }
    else                            { return LTC6804_CORE_ALREADY_AWAKE; }
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

//WGCToDo LTC68042configure_calcPEC15: This function has more affinity with MAX1784Xcomms.
// For MAX1784x, PEC is actually a CRC8, not CRC15
uint16_t LTC68042configure_calcPEC15(uint8_t len, //data array length
                                     uint8_t const data[] ) //data array to generate PEC from
{
  uint16_t poly = 0b10110010; // PEC/CRC polynomial
  uint16_t remainder = 0x00;

  for (uint8_t i = 0; i < len; i++)
  {
    remainder = crc8Table[(uint8_t)remainder ^ data[i]];
  }
  return remainder;
}

/////////////////////////////////////////////////////////////////////////////////////////

//WGCToDo LTC68042configure_spiWrite: 1/30 OK, but may need wakeup attention
// This function has more affinity with MAX1784Xcomms.
// Write out bytes on SPI port while ignoring any bytes coming in
void LTC68042configure_spiWrite(
  uint8_t len,          // number of bytes to be written on the SPI port
  uint8_t const data[]) // array of bytes to be written on the SPI port
{
  //WGCToDo: not implimented yet: LTC68042configure_wakeup();

  digitalWrite(PIN_SPI_CS, LOW);  // assert chip select
  for (uint8_t i = 0; i < len; i++) {
    SPDR = (char)data[i];                  //start the SPI transfer
    /*
     * The following NOP introduces a small delay that can prevent the wait
     * loop form iterating when running at the maximum speed. This gives
     * about 10% more speed, even if it seems counter-intuitive. At lower
     * speeds it is unnoticed.
     */
    asm volatile("nop");
    while (!(SPSR & _BV(SPIF)));  //wait for transfer to complete
  }
  digitalWrite(PIN_SPI_CS, HIGH); // de-assert chip select

  //lastTimeDataSent_millis = millis();

}

/////////////////////////////////////////////////////////////////////////////////////////

//WGCToDo LTC68042configure_spiWriteRead: 1/30 OK, but may need wakeup attention
// This function has more affinity with MAX1784Xcomms.
// Write out bytes on SPI port while ignoring any bytes coming in, then
//   read in bytes
void LTC68042configure_spiWriteRead(
  uint8_t tx_Data[], // array of data to be written on SPI port
  uint8_t tx_len,    // number of bytes to be written on the SPI port
  uint8_t *rx_data,  // Input: array that will store the data read by the SPI port
  uint8_t rx_len )   // number of bytes to be read from the SPI port
{
  //WGCToDo: not implimented yet: LTC68042configure_wakeup();

  digitalWrite(PIN_SPI_CS, LOW); // assert chip select
  for (uint8_t i = 0; i < tx_len; i++) {
    SPDR = (char)tx_Data[i];      // start the SPI transfer
    asm volatile("nop");
    while (!(SPSR & _BV(SPIF)));  // wait for transfer to complete
  }
  for (uint8_t i = 0; i < rx_len; i++) {
    SPDR = 0;                     // send out null byte to start SPI transfer
    asm volatile("nop");
    while (!(SPSR & _BV(SPIF)));  // wait for transfer to complete
    rx_data[i] = (uint8_t)SPDR;   // return read data
  }
  digitalWrite(PIN_SPI_CS, HIGH); // de-assert chip select

  //lastTimeDataSent_millis = millis();

}

/////////////////////////////////////////////////////////////////////////////////////////

void LTC68042configure_handleKeyStateChange(void)
{
    LTC68042result_errorCount_set(0);
    LTC68042result_maxEverCellVoltage_set(0);
    LTC68042result_minEverCellVoltage_set(65535);
}

/////////////////////////////////////////////////////////////////////////////////////////
