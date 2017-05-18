#define BALANCE


void setup() {
  Serial.begin(9600);
  SPI_openBus();
  Serial.println(F("Opened SPI bus."));
  displayInitSequence();
}



void displayInitSequence()
{
  int i = 1;
  while( i < 9 ) {
    MON_toggleGPIOLed1( ((i%2)==1) );
    MON_toggleGPIOLed2( ((i%2)==1) );
    SPI_writeConfigurationRegister();
    delay(200);
    i++;
  }
}
//void printVoltage(int volt_in) {
//  int voltage = (volt_in-512) * 1.5;
//  Serial.print(F("Orig val: "));
//  Serial.print(volt_in , DEC);
//  Serial.print(F(", Volt: "));
//  Serial.print(voltage , DEC);
//  Serial.println(F("mV"));
//}

void balance()
{
  //MON_printAllVoltages();
  int minimum = 4000;
  for(int i = 0; i < 12; i++) {
    minimum = min(minimum, MON_voltages[i]);
  }
  Serial.print(F("Min:"));
  printVoltage(minimum);
  Serial.print(F(" "));
  for (int i = 0; i < 12; i++) {
    Serial.print(i, DEC);
    Serial.print(":");
    printVoltage(MON_voltages[i]);
    const int STABILIZING_OFFSET = 1; //TODO: find out whether needed
    if (MON_voltages[i] > minimum + STABILIZING_OFFSET) {
      MON_dischargeCell((i > 9) ? (i - 10 + 'a') : i, true);
      Serial.print(F(":T "));
    } else {
      MON_dischargeCell((i > 9) ? (i - 10 + 'a') : i, false);
      Serial.print(F(":F "));
    }
  }
  Serial.println();
}

long int counter = 0;

void loop()
{
#ifdef BALANCE
  do_adc_conversion_discharge = true;
  counter++;
#endif
  Serial.print(counter);
  Serial.print(" ");
  updateErrMsgCtr();

  handleInput();

  SPI_sendCommandToMonitor(MON_CMD_START_DIAG_REG);
  delay(20);
  if( do_adc_conversion ) {
    SPI_sendCommandToMonitor(MON_CMD_START_ADC_CONVERSION_ALL);
  }
  if( do_adc_conversion_discharge ) {
    SPI_sendCommandToMonitor(MON_CMD_START_ADC_CONVERSION_ALL_ALLOW_DISCH);
  }


  SPI_readDiagnostics();

  SPI_readConfigurationRegister();

  SPI_readAllVoltages();

  checkVoltages();

#ifdef BALANCE
  balance();
#endif
  delay(1000);
}
