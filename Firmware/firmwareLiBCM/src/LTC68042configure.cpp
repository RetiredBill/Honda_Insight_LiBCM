//Copyright 2021-2024(c) John Sullivan
//github.com/doppelhub/Honda_Insight_LiBCM

//LTC6804 and MAX17xxx configuration functions

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
bool configuureAcqPrecisionTradeoff = ACQ_REASONABLY_PRECISE_AND_FASTER;

/////////////////////////////////////////////////////////////////////////////////////////

void LTC68042configure_acqusitionPrecision_set(bool acqAccuracyTradeoff) { configuureAcqPrecisionTradeoff = acqAccuracyTradeoff; }
bool LTC68042configure_acqusitionPrecision_get(void)                     { return configuureAcqPrecisionTradeoff; }

/////////////////////////////////////////////////////////////////////////////////////////

void MAX17841configure_enableMAX17841(void)
{
  #ifdef BMS_TYPE_WGCLiBCM
    digitalWrite(PIN_SHDNL_MAX17841, HIGH); // Enable MAX17841
    lastMAX1784xTimestamp_millis = millis(); // will need to do t_startup delay
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

void MAX17841configure_disableMAX17841(void)
{
  #ifdef BMS_TYPE_WGCLiBCM
    digitalWrite(PIN_SHDNL_MAX17841, LOW); // disable MAX17841
    lastMAX1784xTimestamp_millis = millis(); // will need to do t_shutdown delay
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

uint16_t MAX17841configure_calcAcquisitionTime_us(
  uint8_t NumCells, bool Ain1En, bool Ain2En, uint8_t AinTime_counts, bool VblkEn, uint8_t DiagSel, uint8_t OvrSmplBf,
  bool AutoBalSwDisEn, uint8_t CellRecoveryTime_counts)
{
    //WGCToDoLater: Many arguments will likely end up never changing.
    // They could be deleted from the argument list, and replaced in the formula
    //   with the following, allowing constant definitions to replace run-time calculations
    // uint8_t NumCells:        CELLS_PER_IC
    // bool Ain1En:             M873_MEASUREEN_INIT_AIN1EN
    // bool Ain2En:             M873_MEASUREEN_INIT_AIN2EN
    // uint8_t AinTime_counts:  M873_ACQCFG_INIT_AINTIME
    // bool VblkEn:             M873_MEASUREEN_INIT_BLOCKEN
    // uint8_t DiagSel:         (BFN_GET(M873_DIAGCFG_bfDIAGSEL, M873_DIAGCFG_INIT))
    // uint8_t OvrSmplBf:        M873_SCANCTRL_INIT_OVSAMPL
    // bool AutoBalSwDisEn:     M873_SCANCTRL_INIT_AUTOBALSWDIS
    // uint8_t CellRecoveryTime_counts: (BFN_GET(M873_ADR_bfCELL_RECOVERY_TIME, M873_ADR_INIT))

    //take care of OVSAMPL bitfield mapping to oversamples
    uint8_t overSamples;
    if      (0 == OvrSmplBf) { overSamples =   1; }
    else if (7 == OvrSmplBf) { overSamples = 128; }
    else                     { overSamples = 1 << (OvrSmplBf + 1); }

    return M873_ACGTime_Initialization_us \
      + (Ain1En ? (M873_ACGTime_AUXINMeasurement_us + (M873_ACGTime_AUXINDelayPerCount_us * AinTime_counts)) : 0) \
      + (Ain2En ? (M873_ACGTime_AUXINMeasurement_us + (M873_ACGTime_AUXINDelayPerCount_us * AinTime_counts)) : 0) \
      + overSamples \
         * (   (VblkEn ? (M873_ACGTime_VBLKP_measurement_us + M873_ACGTime_CellScanSetupVb_us) : M873_ACGTime_CellScanSetupNoVb_us) \
             + (NumCells * M873_ACGTime_CellScansPerCell_us) \
             + ((DiagSel == M873_DIAGSEL_DieTemperature) ? M873_ACGTime_DieTempMeasure_us : 0) \
           ) \
      + M873_ACGTime_HVRecoveryPerOversmpl_us * (overSamples - 1) \
      + (AutoBalSwDisEn ?  (M873_ACGTime_CellRecoveryTimePerCount_us * (CellRecoveryTime_counts + 1)) : 0);
}

#if 0
// helper to validate MAX17841configure_calcAcquisitionTime_us()
void MAX17841configure_test_calcAcquisitionTime_us(void){
    Serial.println(F("\n Acquisition Times:"));
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,0,0,0,0,0,1,0,0)); Serial.println(F(" == 141?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,0,0,0,1,0,1,0,0)); Serial.println(F(" == 160.5?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,0,0,1,0,0)); Serial.println(F(" == 161?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,1,0,1,0,0)); Serial.println(F(" == 180.5?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,1,M873_DIAGSEL_DieTemperature,1,0,0)); Serial.println(F(" == 266.7?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,0,0,0,0,0,4,0,0)); Serial.println(F(" == 825.9?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,0,0,0,1,0,4,0,0)); Serial.println(F(" == 903.9?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,0,0,4,0,0)); Serial.println(F(" == 845.9?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,1,0,4,0,0)); Serial.println(F(" == 923.9?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,1,M873_DIAGSEL_DieTemperature,4,0,0)); Serial.println(F(" == 1268.7?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,1,1,M873_DIAGSEL_DieTemperature,4,0,0)); Serial.println(F(" == 1280.7?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,0,0,0,0,0,8,0,0)); Serial.println(F(" == 1739.1?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,0,0,8,0,0)); Serial.println(F(" == 1759.1?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,1,0,8,0,0)); Serial.println(F(" == 1915.1?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,0,1,M873_DIAGSEL_DieTemperature,8,0,0)); Serial.println(F(" == 2604.7?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,1,1,M873_DIAGSEL_DieTemperature,4,0,0)); Serial.println(F(" == 1280.7?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,1,1,1,1,M873_DIAGSEL_DieTemperature,16,0,0)); Serial.println(F(" == 5288.7?"));//worked
    Serial.print(MAX17841configure_calcAcquisitionTime_us(CELLS_PER_IC,0,0,0,0,0,16,0,0)); Serial.println(F(" == 3565.5?"));//worked
}
#endif

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

    LTC68042configure_writeConfigRegisters(icAddress);
  #else
    //WGCToDo: softwareTimeout is LTC6804_DISCHARGE_TIMEOUT_02_SECONDS, which is 0! (as of 2/9/25)
    // Set up watchdog timer for 2 sec
    MAX1784Xcomms_writeDev843Reg(M873_TIMERCFG, icAddress, (BFN_PREP(M873_TIMERCFG_bfCBPDIV, 1) | BFN_PREP(M873_TIMERCFG_bfCBTIMER, 2)), MCONT_FULL_CHECKS);
    //set cell switch bits
    MAX1784Xcomms_writeDev843Reg(M873_BALSWEN, icAddress, BFN_PREP(M873_BALSWEN_bfBALSWEN, cellBitmap), MCONT_FULL_CHECKS);
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

//program configuration register values onto each LTC6804 IC
//CFGR0:3 are reset when LTC watchdog timer expires (~2000 milliseconds)
//CFGR4:5 are reset when LTC watchdog timer expires, unless software timer is set (and hasn't expired)
//Initialize BMS system to "fresh start" state
void LTC68042configure_programVolatileDefaults(void)
{
  #ifndef BMS_TYPE_WGCLiBCM
                                                 // BIT7    BIT6    BIT5    BIT4    BIT3    BIT2    BIT1   BIT0
                                                 ///////////////////////////////////////////////////////////////
    configurationRegisterData[0] = 0b11111111 ;  //GPIO5   GPIO4   GPIO3   GPIO2   GPIO1   REFON   SWTRD  ADCOPT
    configurationRegisterData[1] = 0x00       ;  //VUV[7]  VUV[6]  VUV[5]  VUV[4]  VUV[3]  VUV[2]  VUV[1] VUV[0]
    configurationRegisterData[2] = 0x00       ;  //VOV[3]  VOV[2]  VOV[1]  VOV[0]  VUV[11] VUV[10] VUV[9] VUV[8]
    configurationRegisterData[3] = 0x00       ;  //VOV[11] VOV[10] VOV[9]  VOV[8]  VOV[7]  VOV[6]  VOV[5] VOV[4]
    configurationRegisterData[4] = 0x00       ;  //DCC8    DCC7    DCC6    DCC5    DCC4    DCC3    DCC2   DCC1
    configurationRegisterData[5] = 0x00       ;  //DCTO[3] DCTO[2] DCTO[1] DCTO[0] DCC12   DCC11   DCC10  DCC9
    //Above values turn off all discharge FETs, turns reference on, and configure ADC LPF to '2 kHz mode' (1.7 kHz LPF)
    //see Table36 (p51) for more info:
    //DCTO  = set discharge timer
    //DCC   = control cell discharge FET (1=on)
    //VUV   = undervoltage comparison voltage ((VUV+1) * 16 * 100uV)
    //VOV   = over voltage comparison voltage ((VUV  ) * 16 * 100uV)
    //GPIO  = read to get pinState, write 0/1 to enable/disable pull-down (disabled by default)
    //REFON = keep ADC reference powered whenever IC awake (reduces ADC delay)
    //SWTRD = (read only) is SWTEN (PIN_SOFTWARE_TIMER_ENABLE) high or low (high=SW timer allowed)
    //ADCOPT= sets adc fast/normal/slow LPF cutoff frequency values (0: 27k/7k/26 Hz)(1: 14k/3k/2k Hz)
        //Note: fast, normal, or slow is configured in ADCV command

    LTC68042configure_writeConfigRegisters(BROADCAST_TO_ALL_ICS);
    LTC68042cell_dischargeAllowedDuringConversion_set(IS_DISCHARGE_ALLOWED_DURING_CONVERSION);
  #else
    // Compared to BMS_TYPE_WGCLiBCM, LiBCM version of this is relatively fast (12 SPI bytes => 384us @ 32us/byte),
    //   whereas for BMS_TYPE_WGCLiBCM, wakeup() alone takes ~4.3ms! Total is ~6.8ms

    bool allOk = true;
    char msg[30];
    uint32_t moduleId[TOTAL_IC];
    uint16_t registerValue[TOTAL_IC];

    // Initialize all MAX17843 chips via reset.
    // If we are executing after a POR,
    //   then the reset is done (and we'll verify this),
    // else do the POR reset here
    //WGCToDo: Options to initialize 871:
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
  #endif
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
  #ifndef BMS_TYPE_WGCLiBCM
    spi_enable(SPI_CLOCK_DIV64); //JTS2doLater: increase clock speed //DIV16 & DIV32 work on bench
  #else
    MAX17841configure_enableMAX17841(); // get a head start on tstartup delay
    pinMode(PIN_SHDNL_MAX17841, OUTPUT);
    MAX1784Xcomms_setJustWokeUpState(true);
    spi_enable(SPI_CLOCK_DIV64); //JTS2doLater: increase clock speed //DIV16 & DIV32 work on bench
    //spi_enable(SPI_CLOCK_DIV16);//WGCToDo: works, but need some tweaking
    //spi_enable(SPI_CLOCK_DIV8);//WGCToDo: probably works, but need even more tweaking
  #endif
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

//wake up LTC core if watchdog timed out
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

//wake up isoSPI if timed out
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
//WGCToDoNext: CRITICAL if we slept, and sleep shut off MAX17841 to save power, and we wake up and don't do LTC68042configure_programVolatileDefaults() (like via LTC68042cell_nextVoltages()), but instead do something like LTC68042configure_setBalanceResistors(), THIS FAILS!
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

static uint8_t testDischargeState = TESTDISCHASRGESTATE_DISABLED; //state machine
static uint16_t cellVoltagesTest_counts[TOTAL_IC][CELLS_PER_IC];
static uint32_t latestStateTimestamp_ms = 0;
static int16_t  dischargingAverageDeltaV_counts = 0;
static int16_t  nonDischargingAverageDeltaV_counts = 0;
static uint16_t test1_cellStatusBitmap[TOTAL_IC] = {0};
static uint16_t test2_cellStatusBitmap[TOTAL_IC] = {0};

/////////////////////////////////////////////////////////////////////////////////////////

void LTC68042configure_enabletestDischargeFETs(void) {testDischargeState = TESTDISCHASRGESTATE_TURNON;}

///////// test helper functions
void testHelper_clearCellTestFlags(int16_t testFlagBitmap[])
{
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        testFlagBitmap[ic] = 0;
    }
}

//helper function to set up a cell discharge circuit test
void testHelper_saveCellVoltages(void)
{
    //save the prior cell voltage results away for later reference
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        for (uint8_t cellNumber = 0; cellNumber < CELLS_PER_IC; cellNumber++)
        {
            cellVoltagesTest_counts[ic][cellNumber] = LTC68042result_specificCellVoltage_get(ic, cellNumber);
        }
    }
}

//helper function to set up a cell discharge circuit test
void testHelper_setCellDischarge(uint16_t cellDischargeBitmap)
{
    //turn on all odd or even cell balance circuits only
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        LTC68042configure_setBalanceResistors(
           FIRST_IC_ADDR + ic,
           cellDischargeBitmap,
           LTC6804_DISCHARGE_TIMEOUT_02_SECONDS);
        debugUSB_setCellBalanceStatus(ic, cellDischargeBitmap, 0); //WGCToDo: change arg 3 (cellDischargeVoltageThreshold) to something useful?
    }
}

//helper function that looks for:
//  adjacent cell voltage absulut deltas greater than a minimum (indicating discharge circuit works)
//  cell voltages not insanely high or low (indicating no open sense wires)
bool testHelper_checkInterCellDeltaAndSaneCellVoltages(
   uint16_t cellFailsDeltaBitmap[], //cell bitmap for cells that are failing to discharge
   uint16_t cellFailsHighBitmap[],  //cell bitmap for cells with excessively high voltage
   uint16_t cellFailsLowBitmap[])   //cell bitmap for cells with excessively low voltage
{
    bool didTestFail = false;

    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        for (uint8_t cellNumber = 0 ; cellNumber < CELLS_PER_IC; cellNumber++)
        {
            if (TESTBASIC_SANE_HIGH_TESTLIMIT_counts < cellVoltagesTest_counts[ic][cellNumber])
            {
                //then this cell fails high
                cellFailsHighBitmap[ic] |= (1 << cellNumber);
            }
            else if (TESTBASIC_SANE_LOW_TESTLIMIT_counts > cellVoltagesTest_counts[ic][cellNumber])
            {
                //then this cell fails low
                cellFailsLowBitmap[ic] |= (1 << cellNumber);
            }
            else if((CELLS_PER_IC - 1) > cellNumber) //WGCToDo: could also account for end cells on adjacent IC's
            {
                //check voltage delta between this cell and the next higher
                int16_t cellDelta =   LTC68042result_specificCellVoltage_get(ic, cellNumber)      //voltage while discharging
                                    - LTC68042result_specificCellVoltage_get(ic, cellNumber + 1); //voltage while discharging

                //account for any initial cell imbalance
                cellDelta -=   cellVoltagesTest_counts[ic][cellNumber]                //resting voltage
                             - cellVoltagesTest_counts[ic][cellNumber + 1];           //resting voltage

                if (0 > cellDelta) { cellDelta = - cellDelta; } //absolute value of cellDelta
                if (TESTBASIC_DELTA_TESTLIMIT_counts > cellDelta)
                {
                    //cell fails, not enough IR drop delta, => current (I) too low (assuming R is not too low...)
                    cellFailsDeltaBitmap[ic] |= (1 << cellNumber);
                }
            }
        }
        if (cellFailsHighBitmap[ic] || cellFailsLowBitmap[ic]) { didTestFail = true; }
    }

    return didTestFail;
}

//helper function for waiting for odd/even delta voltage separation
void testHelper_calculateVoltageDeltas(int16_t * evenAverageDeltaV_counts, int16_t * oddAverageDeltaV_counts)
{
    //we're waiting for a detectable voltage difference to arrise between
    //discharging cells and non-discharging cells

    //calculate average delta for both even and not-odd cells
    *evenAverageDeltaV_counts = 0;
    *oddAverageDeltaV_counts  = 0;
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        for (uint8_t cellNumber = 0; cellNumber < CELLS_PER_IC; cellNumber += 2)
        {
            //even cells
            *evenAverageDeltaV_counts +=
              (   LTC68042result_specificCellVoltage_get(ic, cellNumber    )
                - cellVoltagesTest_counts[ic][cellNumber    ]
              );
            //odd cells
            *oddAverageDeltaV_counts +=
              (   LTC68042result_specificCellVoltage_get(ic, cellNumber + 1)
                - cellVoltagesTest_counts[ic][cellNumber + 1]
              );
        }
    }
    *evenAverageDeltaV_counts /= CELLS_PER_IC;
    *oddAverageDeltaV_counts  /= CELLS_PER_IC;
}

//helper function for performing odd/even test
bool testHeper_DischargeTestTesting(
    uint16_t cellDischargeBitmap,       // input bitmap of cells for ALL ICs to turn on cell discharge circuit
    int16_t passedDischargeBitmap[],    // returns bitmap of cells for each IC that pass discharge test
    int16_t passedNonDischargeBitmap[]) // returns bitmap of cells for each IC that pass non-discharge test
{
   //sufficient delta separation has occurred:
    //  note which discharging cells are actually discharging
    //  and also check if any not-discharging cells are unexpectadly discharging

    bool allICsPassed = true;

    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        // set loop start based on doing even or odd
        uint8_t cellNumber = (cellDischargeBitmap & 1) ? 0 : 1;
        for ( ; cellNumber < CELLS_PER_IC; cellNumber += 2)
        {
            uint16_t cellBit = (1 << cellNumber);
            //check if discharging cell not yet passed
            if ( ! (passedDischargeBitmap[ic] & cellBit))
            {
                //then check if discharging cell now passes
                if (  TESTDISCHASRGE_DISCHARGE_TESTLIMIT_counts <
                      (LTC68042result_specificCellVoltage_get(ic, cellNumber)
                       - cellVoltagesTest_counts[ic][cellNumber]
                       - nonDischargingAverageDeltaV_counts
                      )
                   )
                {
                    //then this discharging cell has passed
                    passedDischargeBitmap[ic] |= cellBit;
                }
            }
        }
        //turn off cell balance circuit for passed cells
        uint16_t newBitMap = (~(passedDischargeBitmap[ic])) & cellDischargeBitmap;
        LTC68042configure_setBalanceResistors(
          FIRST_IC_ADDR + ic,
          newBitMap,
          LTC6804_DISCHARGE_TIMEOUT_02_SECONDS);
        debugUSB_setCellBalanceStatus(ic, newBitMap, 1); //WGCToDo: change arg 3 (cellDischargeVoltageThreshold) to something useful?

        //check if any cells being discharged don't pass
        if (newBitMap)
        {
            //test is not done, we'll continue waiting for laggards
            allICsPassed = false;
        }
    }

    //if test is done or time has expired, check if any non-discharge cells have actually dischaged
    if (    allICsPassed
         || (TESTDISCHASRGE_TIMELIMIT_ms < (millis() - latestStateTimestamp_ms))
       )
    {
        for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
        {
            // set loop start based on doing odd or even
            uint8_t cellNumber = (cellDischargeBitmap & 1) ? 1 : 0;
            for (; cellNumber < CELLS_PER_IC; cellNumber += 2)
            {
                //non-discharging cell passed?
                int16_t nonDischargeResult_counts =
                     LTC68042result_specificCellVoltage_get(ic, cellNumber)
                   - cellVoltagesTest_counts[ic][cellNumber]
                   - nonDischargingAverageDeltaV_counts;
                if (    ( TESTDISCHASRGE_NONDISCHRGE_TESTLIMIT_counts > nonDischargeResult_counts)
                     && (-TESTDISCHASRGE_NONDISCHRGE_TESTLIMIT_counts < nonDischargeResult_counts)
                   )
                {
                    //then this non-discharging cell has passed
                    passedNonDischargeBitmap[ic] |= (1 << cellNumber);
                }
            }
        }
    }

    return allICsPassed;
}

void testHelper_printCellVoltages(const __FlashStringHelper * title)
{
    //WGCToDo: maybe add my own dedicated debug mode instead of DEBUGUSB_STREAM_DEBUG ('$DISP=DBG' -> 'DB2')
    if (debugUSB_dataTypeToStream_get() == DEBUGUSB_STREAM_DEBUG)
    {
        Serial.println(F("")); //newline
        Serial.print(title);
        for (uint8_t ic = 0; ic < TOTAL_IC; ic++) debugUSB_printOneICsCellVoltages( ic, FOUR_DECIMAL_PLACES);
        Serial.println("");
    }
}

void testHelper_printTestResults(uint16_t cellFailuresBitmap[])
{
    bool allICsPassed = true;
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++) {if (cellFailuresBitmap[ic]) allICsPassed = false; }

    if (allICsPassed)
    {
        Serial.print(F(" Passed"));
    }
    else
    {
        Serial.print(F(" FAILED!  Failed cell bitmaps: (0x) "));
        for (uint8_t ic = 0; ic < (TOTAL_IC - 1); ic++)
        {
            Serial.print(cellFailuresBitmap[ic], HEX);
            Serial.print(F(", "));
        }
        Serial.print(cellFailuresBitmap[TOTAL_IC - 1], HEX);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

//Run quick basic confidence test on BMS circuits
bool LTC68042configure_basicConfidenceTest(void)
{
    bool didTestFail = false;

    //note test start time
    latestStateTimestamp_ms = millis();

    //set up test
    testHelper_clearCellTestFlags(test1_cellStatusBitmap); // re-use some available statics
    testHelper_clearCellTestFlags(test2_cellStatusBitmap); // re-use some available statics
    uint16_t test3_EvenTestCellFailsHighBitmap[TOTAL_IC] = {0};
    uint16_t test4_EvenTestCellFailsLowBitmap[TOTAL_IC] = {0};
    uint16_t test5_OddTestCellFailsHighBitmap[TOTAL_IC] = {0};
    uint16_t test6_OddTestCellFailsLowBitmap[TOTAL_IC] = {0};
    LTC68042cell_dischargeAllowedDuringConversion_set(DCP_ENABLED);

    //verify LTC68042result_errorCount_get() doesn't increase during test
    uint8_t errorCounts = LTC68042result_errorCount_get();

    //tell the world that cells are balancing
    cellBalance_set_cellsAreBalancing(YES);

    //================== start with resting cell voltages
    LTC68042cell_acquireAllCellVoltages(); //abandon any in-process acquisition (waiting for it to complete, if needed)
    testHelper_saveCellVoltages();
    testHelper_printCellVoltages(F("Resting:")); // controlled by '$DISP=DBG'

    //================== now do even cells
    testHelper_setCellDischarge(TESTDISCHASRGE_EvenCellsBitMap);

    //measure and check while even cells are discharging
    LTC68042cell_acquireAllCellVoltages();
    testHelper_printCellVoltages(F("Even:")); // controlled by '$DISP=DBG'
    didTestFail &= testHelper_checkInterCellDeltaAndSaneCellVoltages(
      test1_cellStatusBitmap,            //cells not discharging
      test3_EvenTestCellFailsHighBitmap, //cell voltages that way high => open sense wire
      test4_EvenTestCellFailsLowBitmap); //cell voltages that way low  => open sense wire

    //================== now do odd cells
    //  (Yes, some redundncy in detecting open sense wires)
    testHelper_setCellDischarge(TESTDISCHASRGE_OddCellsBitMap);

    //measure and check while odd cells are discharging
    LTC68042cell_acquireAllCellVoltages();
    testHelper_printCellVoltages(F("Odd:")); // controlled by '$DISP=DBG'
    didTestFail &= testHelper_checkInterCellDeltaAndSaneCellVoltages(
      test2_cellStatusBitmap,            //cells not discharging
      test5_OddTestCellFailsHighBitmap,  //cell voltages that way high => open sense wire
      test6_OddTestCellFailsLowBitmap);  //cell voltages that way low  => open sense wire

    //turn off all cell discharge circuits
    disableDischargeResistors();

    //tell the world that cells are no longer balancing
    cellBalance_set_cellsAreBalancing(NO);

    //WGCToDoNext: simulated sense wire failures
    //test3_EvenTestCellFailsHighBitmap[0] = 0b111111111111111;
    //test4_EvenTestCellFailsLowBitmap[0]  = 0b111111111111111;
    //test3_EvenTestCellFailsHighBitmap[1] = 0b000000011000000;
    //test4_EvenTestCellFailsLowBitmap[1]  = 0b000000110000000;
    //test3_EvenTestCellFailsHighBitmap[2] = 0b111111111000000;
    //test4_EvenTestCellFailsLowBitmap[2]  = 0b111111110000000;
    //test3_EvenTestCellFailsHighBitmap[3] = 0b001100000000011;
    //test4_EvenTestCellFailsLowBitmap[3]  = 0b000110000000110;

    //test is done
    uint32_t now_ms = millis();
    errorCounts -= LTC68042result_errorCount_get();
    LTC68042cell_dischargeAllowedDuringConversion_set(IS_DISCHARGE_ALLOWED_DURING_CONVERSION);

    Serial.print(F("\nBasic BMS circuit test"));
    Serial.print(F("\n   Acquisition errors: "));
    if (0 == errorCounts) { Serial.print(F("None. Test should be good")); }
    else
    {
        Serial.print(F("ERRORS OCCURRED. Test results may not be accurate, but there are other issues"));
    }
    Serial.print(F("\n   Discharge Circuit EVEN cell test: "));
    testHelper_printTestResults(test1_cellStatusBitmap);
    Serial.print(F("\n   Discharge Circuit ODD  cell test: "));
    testHelper_printTestResults(test2_cellStatusBitmap);
    Serial.print(F("\n   HIGH Cells EVEN cell test: "));
    testHelper_printTestResults(test3_EvenTestCellFailsHighBitmap);
    Serial.print(F("\n   LOW  Cells EVEN cell test: "));
    testHelper_printTestResults(test4_EvenTestCellFailsLowBitmap);
    Serial.print(F("\n   HIGH Cells ODD  cell test: "));
    testHelper_printTestResults(test5_OddTestCellFailsHighBitmap);
    Serial.print(F("\n   LOW  Cells ODD  cell test: "));
    testHelper_printTestResults(test6_OddTestCellFailsLowBitmap);
    Serial.println("");

    //create common language failure report
    uint16_t openWireCellFlags = 0;
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        //openWireCellFlags = test1_cellStatusBitmap[ic] & test2_cellStatusBitmap[ic];//WGCToDoNext: this is wrong
        if (openWireCellFlags)
        {
            Serial.print(F(" IC "));
            Serial.print(ic);
            Serial.print(F(" cells "));
            for (uint8_t cellNumber = 0 ; cellNumber < CELLS_PER_IC; cellNumber++)
            {
                if (openWireCellFlags & (1 << cellNumber)) {
                     Serial.print(cellNumber);
                     Serial.print(F(", "));
                }
            }
            Serial.println(F("\n   likely have open sense cable wire connections"));
        }
    }
    Serial.print(F("\n   Elapsed test time (ms): "));
    Serial.print(now_ms - latestStateTimestamp_ms);
    Serial.println("");

    return didTestFail;
}

/////////////////////////////////////////////////////////////////////////////////////////

// Test the cell balance circuit on each cell
//   Ignition must be off, grid charger must be off (for this implimentation)
//Required conditions to allow running this test (maybe all decided in calling function...)
//  cell voltages are above a minimum and below maximum allowable done by isBalancingAllowed()
//  key-off (already handled by invocation via key_handleKeyEvent_off())
//WGCToDo: not charging (or turn off charger and postpone charging? or allow charging)
//  adequate SoC done by isBalancingAllowed()
//  acceptable temperature done by isBalancingAllowed()
//  Enough time elapsed since key-off/last regen/assist use to have
//    otherwise stable cell voltages? No, algorithm tracks drift
//WGCToDo: do we need a "wait for settled cell voltages" state? No, tracking drift, unless that doesn't work...
uint8_t LTC68042configure_testDischargeFETs(void)
{

if (testDischargeState != TESTDISCHASRGESTATE_DISABLED) { //WGCToDo: debugging only
    uint32_t now_ms = millis();
    Serial.print(F("\nCellBalBIST state: "));
    Serial.print(testDischargeState);
    Serial.print(F(", now (ms): "));
    Serial.print(now_ms);
    Serial.print(F(", elapsed time (ms): "));
    Serial.print(now_ms - latestStateTimestamp_ms);
    Serial.print(F(", limit (ms): "));
    Serial.print(TESTDISCHASRGE_TIMELIMIT_ms);
    Serial.print(F(", non-discharge delta: 0x"));
    Serial.print(nonDischargingAverageDeltaV_counts, HEX);
    Serial.print(F(", discharge delta: 0x"));
    Serial.println(dischargingAverageDeltaV_counts, HEX);
}

    //note test start time
    static uint32_t testStartTimestamp_ms = millis();

    if (testDischargeState == TESTDISCHASRGESTATE_TURNON)
    {
        //set up test
        testHelper_clearCellTestFlags(test1_cellStatusBitmap);
        testHelper_clearCellTestFlags(test2_cellStatusBitmap);
        LTC68042cell_dischargeAllowedDuringConversion_set(DCP_ENABLED);

        //tell the world that cells are balancing
        cellBalance_set_cellsAreBalancing(YES);

        //get resting cell voltages
        LTC68042cell_acquireAllCellVoltages();
        testHelper_saveCellVoltages();

        //start with even cells
        testHelper_setCellDischarge(TESTDISCHASRGE_EvenCellsBitMap);

        //note time
        latestStateTimestamp_ms = millis();

        testDischargeState = TESTDISCHASRGESTATE_WAITING_EVEN; // next state
    }

    else if (testDischargeState == TESTDISCHASRGESTATE_WAITING_EVEN)
    {

        //make another set of measurements
        LTC68042cell_acquireAllCellVoltages();

        //testHelper_calculateVoltageDeltas(even, odd)
        testHelper_calculateVoltageDeltas(&dischargingAverageDeltaV_counts, &nonDischargingAverageDeltaV_counts);

        //check that these 2 population's average deltas have sufficiently separated, otherwise wait
        if (TESTDISCHASRGE_DeltaVSep_THRESHOLD_counts < (nonDischargingAverageDeltaV_counts - dischargingAverageDeltaV_counts))
        {
            //then we can move on
            testDischargeState = TESTDISCHASRGESTATE_TESTING_EVEN; // next state
        }
        else if (TESTDISCHASRGE_TIMELIMIT_ms < (millis() - latestStateTimestamp_ms))
        {
            //then time has run out! Bad test design
            // disable all cell balance circuits
            disableDischargeResistors();
            //let the world know of my failure...
            Serial.print(F("Cell Balance Circuit test aborting: timout waiting for even cell delta separation (non-discharge delta: "));
            Serial.print(nonDischargingAverageDeltaV_counts);
            Serial.print(F(" discharge delta: "));
            Serial.println(dischargingAverageDeltaV_counts);
            testDischargeState = TESTDISCHASRGESTATE_DONE; // next state
        }
    }

    else if (testDischargeState == TESTDISCHASRGESTATE_TESTING_EVEN)
    {
        if (testHeper_DischargeTestTesting(TESTDISCHASRGE_EvenCellsBitMap, test1_cellStatusBitmap, test2_cellStatusBitmap))
        {
            //then test wait is over
            testDischargeState = TESTDISCHASRGESTATE_DONE_EVEN; // next state
        }
        else
        {
          //make another set of measurements
          LTC68042cell_acquireAllCellVoltages();
          //and continue waiting for laggards, or time expiration
        }
    }

    else if (testDischargeState == TESTDISCHASRGESTATE_DONE_EVEN)
    {
        //now do odd cells
        testHelper_setCellDischarge(TESTDISCHASRGE_OddCellsBitMap);

        //note time
        latestStateTimestamp_ms = millis();

        testDischargeState = TESTDISCHASRGESTATE_WAITING_ODD; // next state
    }

    else if (testDischargeState == TESTDISCHASRGESTATE_WAITING_ODD)
    {
        //make another set of measurements
        LTC68042cell_acquireAllCellVoltages();

        //testHelper_calculateVoltageDeltas(even, odd)
        testHelper_calculateVoltageDeltas(&nonDischargingAverageDeltaV_counts, &dischargingAverageDeltaV_counts);

        //check that these 2 population's average deltas have sufficiently separated, otherwise wait
        if (TESTDISCHASRGE_DeltaVSep_THRESHOLD_counts < (nonDischargingAverageDeltaV_counts - dischargingAverageDeltaV_counts))
        {
            //then we can move on
            testDischargeState = TESTDISCHASRGESTATE_TESTING_ODD; // next state
        }
        else if (TESTDISCHASRGE_TIMELIMIT_ms < (millis() - latestStateTimestamp_ms))
        {
            //then time has run out! Bad test design
            // disable all cell balance circuits
            disableDischargeResistors();
            //let the world know of my failure...
            Serial.print(F("Cell Balance Circuit test aborting: timout waiting for odd cell delta separation (non-discharge delta: "));
            Serial.print(nonDischargingAverageDeltaV_counts);
            Serial.print(F(" discharge delta: "));
            Serial.println(dischargingAverageDeltaV_counts);
            testDischargeState = TESTDISCHASRGESTATE_DONE; // next state
        }
    }

    else if (testDischargeState == TESTDISCHASRGESTATE_TESTING_ODD)
    {
        if (testHeper_DischargeTestTesting(TESTDISCHASRGE_OddCellsBitMap, test1_cellStatusBitmap, test2_cellStatusBitmap))
        {
            //then test wait is over
            testDischargeState = TESTDISCHASRGESTATE_DONE; // next state
        }
        else
        {
          //make another set of measurements
          LTC68042cell_acquireAllCellVoltages();
          //and continue waiting for laggards, or time expiration
        }
    }

    else if (testDischargeState == TESTDISCHASRGESTATE_DONE)
    {
        //turn off all cell discharge circuits
        disableDischargeResistors();

        //tell the world that cells are no longer balancing
        cellBalance_set_cellsAreBalancing(NO);

        LTC68042cell_dischargeAllowedDuringConversion_set(IS_DISCHARGE_ALLOWED_DURING_CONVERSION);

        uint32_t now_ms = millis();

        //report results
        Serial.print(F("Cells that passed discharge test: "));
        for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
        {
            Serial.print(String(test1_cellStatusBitmap[ic], HEX));
            Serial.print(',');
        }

        Serial.print(F("\nCells that passed non-discharge test: "));
        for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
        {
            Serial.print(String(test2_cellStatusBitmap[ic], HEX));
            Serial.print(',');
        }

        Serial.print(F("\n   Elapsed test time (ms): "));
        Serial.print(now_ms - testStartTimestamp_ms);
        Serial.println("");

        testDischargeState = TESTDISCHASRGESTATE_DISABLED; // next state
    }

    //else if (testDischargeState == TESTDISCHASRGESTATE_DISABLED) { ; } //nothing to do
    //else                                                         { ; } //nothing to do

    return testDischargeState;
}

/////////////////////////////////////////////////////////////////////////////////////////

//WGCToDo LTC68042configure_calcPEC15: This function has more affinity with MAX1784Xcomms.
// For MAX1784x, PEC is actually a CRC8, not CRC15
uint16_t LTC68042configure_calcPEC15(uint8_t len, //data array length
                                     uint8_t const data[] ) //data array to generate PEC from
{
  #ifndef BMS_TYPE_WGCLiBCM
    uint16_t remainder,addr;

    remainder = 16;//initialize the PEC
    for (uint8_t i = 0; i<len; i++) // loops for each byte in data array
    {
        addr = ( (remainder>>7)^data[i] ) & 0xff;//calculate PEC table address
        remainder = (remainder<<8) ^ crc15Table[addr];
    }

    return(remainder<<1);//The CRC15 LSB is 0, so multiply by 2
  #else
    uint16_t poly = 0b10110010; // PEC/CRC polynomial
    uint16_t remainder = 0x00;

    for (uint8_t i = 0; i < len; i++)
    {
        remainder = crc8Table[(uint8_t)remainder ^ data[i]];
    }
    return remainder;
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

//WGCToDo LTC68042configure_spiWrite: 1/30 OK, but may need wakeup attention
// This function has more affinity with MAX1784Xcomms.
// Write out bytes on SPI port while ignoring any bytes coming in
void LTC68042configure_spiWrite(
    uint8_t len,          // number of bytes to be written on the SPI port
    uint8_t const data[]) // array of bytes to be written on the SPI port
{
  #ifndef BMS_TYPE_WGCLiBCM
    LTC68042configure_wakeup();

    digitalWrite(PIN_SPI_CS,LOW);
    for (uint8_t i = 0; i < len; i++) { spi_write((char)data[i]); } //all SPI writes occur here
    digitalWrite(PIN_SPI_CS,HIGH);

    lastTimeDataSent_millis = millis();
  #else
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
  #endif
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
  #ifndef BMS_TYPE_WGCLiBCM
    LTC68042configure_wakeup();

    digitalWrite(PIN_SPI_CS,LOW);
    for (uint8_t i = 0; i < tx_len; i++) { spi_write(tx_Data[i]); }
    for (uint8_t i = 0; i < rx_len; i++) { rx_data[i] = (uint8_t)spi_read(0xFF); }
    digitalWrite(PIN_SPI_CS,HIGH);

    lastTimeDataSent_millis = millis();
  #else
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
  #endif
}

/////////////////////////////////////////////////////////////////////////////////////////

void LTC68042configure_handleKeyStateChange(void)
{
    LTC68042result_errorCount_set(0);
    LTC68042result_maxEverCellVoltage_set(0);
    LTC68042result_minEverCellVoltage_set(65535);
}

/////////////////////////////////////////////////////////////////////////////////////////
