boolean start_discharge_select = false;
boolean stop_discharge_select = false;

void printHelp()
{
  printHelp_1();
  printHelp_2();
  printHelp_3();
  printHelp_4();
}

void printHelp_1() {
  Serial.println(F("\n**************************"));
  Serial.println(F(" *   USER INPUT OPTIONS   *"));
  Serial.println(F("***************************"));
  Serial.println(F("** DIRECT COMMANDS"));
  Serial.println(F("******************"));
  Serial.println(F(" * a     - Toggle start adc conversion"));
  Serial.println(F(" * s     - Toggle start adc conversion - Allow discharge"));
  Serial.println(F(" *"));
}
void printHelp_2()
{
  Serial.println(F("*******************"));
  Serial.println(F("** REGISTER CHANGES"));
  Serial.println(F("*******************"));

  Serial.println(F(" * d     - START discharging. Select cells after entering d"));
  Serial.println(F(" * z     - STOP discharging. Select cells after entering z"));
  Serial.println(F(" * 0-b|x - ONLY DIRECTLY AFTER d or z. Cell selection as hex, x for all"));
  Serial.println(F(" * 0-7   - Set comparator duty cycle"));
  Serial.println(F(" * w     - Write config changes to monitor"));
  Serial.println(F(" *"));
}
void printHelp_3()
{
  Serial.println(F("*******************"));
  Serial.println(F("** PRINT DEBUG DATA"));
  Serial.println(F("*******************"));
  Serial.println(F(" * C     - Print configuration register"));
  Serial.println(F(" * D     - Print diagnostics register and adc conversion status "));
  Serial.println(F(" * H     - Print help message"));
  Serial.println(F(" * S     - Print Battery status"));
  Serial.println(F(" * V     - Print voltages data"));
  Serial.println(F("*******************"));
}
void printHelp_4()
{
  Serial.println(F(""));
  Serial.println(F("*******************"));
  Serial.println(F(" * USAGE"));
  Serial.println(F("*******************"));
  Serial.println(F("To wake the monitor from sleep after boot the"));
  Serial.println(F("comparator duty cycle has to be set. The LTC6803"));
  Serial.println(F("starts to function only after the comp.duty. cycle is set."));
  Serial.println(F(""));
  Serial.println(F("ALL register changes have to be separately written to monitor"));
  Serial.println(F("with the 'w' command."));
  Serial.println(F("The 'a' and 's' commands are directly sent to the monitor."));
  Serial.println(F("*******************"));
  Serial.println(F(""));
}





void handleInput()
{
  char user_input = Serial.read();
  if( user_input == -1 ) {
    return;
  }


  if( start_discharge_select ) {
    startDischarge(user_input);
  }
  else if( stop_discharge_select ) {
    stopDischarge(user_input);
  }


  else if( (user_input >= '0') && (user_input <= '7') ) {
    MON_setComparatorDutyCycle((unsigned char) (user_input & 0x0F));
    if(__DEBUG__) {
      Serial.println(F("\nChanged comparator duty cycle in local buffer."));
    }
  }

  else if(user_input == 'a') {
    if(__DEBUG__) {
      Serial.println(F("\nToggle start adc conversion command to monitor."));
    }
    do_adc_conversion = !do_adc_conversion;
  }
  else if(user_input == 'd') {
    if(__DEBUG__) {
      Serial.println(F("\nDischarge on: Which cell? (0-b or 'x' for all):"));
    }
    start_discharge_select = true;
  }
  else if(user_input == 's') {
    if(__DEBUG__) {
      Serial.println(F("\nToggle adc conversion, allow discharge command to monitor."));
    }
  do_adc_conversion_discharge = !do_adc_conversion_discharge;

  }
  else if(user_input == 'z') {
    if(__DEBUG__) {
      Serial.println(F("\nStop discharge: Which cell? (0-b or 'x' for all):"));
    }
    stop_discharge_select = true;
  }
  else if(user_input == 'w') {
    if(__DEBUG__) {
      Serial.println(F("\nWriting local configuration register buffer to monitor."));
    }
    SPI_writeConfigurationRegister();
  }


  else if(user_input == 'H') {
    printHelp();
  }
  else if(user_input == 'C') {
    if(__DEBUG__) {
      Serial.println(F("\nPrint monitor configuration register."));
    }
    MON_printConfigurationRegister();
  }
  else if(user_input == 'D') {
    if(__DEBUG__) {
      Serial.println(F("\nPrint monitor diagnostics register."));
    }
    MON_printDiagnosticsRegister();
  }
  else if(user_input == 'L') {
    if(__DEBUG__) {
      Serial.println(F("\nPrint local configuration register buffer."));
    }
    MON_printConfigurationRegisterLocal();
  }
  else if(user_input == 'S') {
    printBatteryStatus();
  }
  else if(user_input == 'V') {
    if(__DEBUG__) {
      Serial.println(F("\nPrint all voltages."));
    }
    MON_printAllVoltages();
  }
}

void startDischarge(char user_input)
{
  if( user_input >= '0' && user_input <= '9' ) {
    MON_dischargeCell(user_input - '0', true);
  }
  else if( user_input == 'a' ) {
    MON_dischargeCell(10, true);
  }
  else if( user_input == 'b' ) {
    MON_dischargeCell(11, true);
  }
  else if( user_input == 'x' ) {
    for (int i = 0; i < 12; i++) {
      MON_dischargeCell(i, true);
    }
  }
  Serial.print(F("Input received (ascii hex): "));
  Serial.println(user_input , HEX);
  start_discharge_select = false;
}


void stopDischarge(char user_input)
{
  if( user_input >= '0' && user_input <= '9' ) {
    MON_dischargeCell(user_input - '0', false);
  }
  else if( user_input == 'a' ) {
    MON_dischargeCell(10, false);
  }
  else if( user_input == 'b' ) {
    MON_dischargeCell(11, false);
  }
  else if( user_input == 'x' ) {
    for (int i = 0; i < 12; i++) {
      MON_dischargeCell(i, false);
    }
  }
  stop_discharge_select = false;
  Serial.print(F("Input received (ascii hex): "));
  Serial.println(user_input , HEX);
}
