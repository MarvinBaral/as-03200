
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




void loop()
{
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

  delay(80);
}
