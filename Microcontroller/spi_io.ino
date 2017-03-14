#include <SPI.h>

/* Definitions about SPI bus */
/* Chip select DIO output definitions */
#define SPI_SS_MON 10 /* SPI Chip/Slave Select (SS) for lowest monitor in stack */
#define SPI_MOSI 11  /* SPI MOSI DIO output */
#define SPI_MISO 12  /* SPI MISO DIO output */
#define SPI_CLK  13  /* SPI CLOCK DIO output */



/*********************/
/* SPI API functions */
/*********************/

/* Open SPI bus for communication */
void SPI_openBus()
{
  SPI.begin();
  pinMode(SPI_SS_MON , OUTPUT);    /* Set slave select 1 to output */
  digitalWrite(SPI_SS_MON , HIGH); /* Slave select high when no traffic */
  SPI.setBitOrder(MSBFIRST);             /* LTC-6803 has Most Significant Bit first */
  SPI.setDataMode(SPI_MODE3);            /* CPHA = 1 , CPOL = 1. (LTC-6803 datasheet page 17) */
  SPI.setClockDivider(SPI_CLOCK_DIV32);  /* Bus speed */
}

/* End SPI bus communication */
void SPI_closeBus()
{
  SPI.end();
}

void SPI_setSlaveSelect(boolean level)
{
  if(level) {
    digitalWrite(SPI_SS_MON , HIGH);
  }
  else {
    digitalWrite(SPI_SS_MON , LOW);
  }
  delayMicroseconds(100);
}





/* Write configuration register to monitor */
void SPI_writeConfigurationRegister()
{
  int i = 0;
  unsigned char pec = calculatePECForByte(MON_WRITE_CONF_REG , 0 , true);

  SPI_setSlaveSelect(false);
  SPI.transfer(MON_WRITE_CONF_REG);
  SPI.transfer(pec);

  while( i < MON_SIZE_OF_CONF_REG ) {
    SPI.transfer(MON_configuration_register_local[i]);
    i++;
  }
  pec = calculatePECForByteArray(MON_configuration_register_local , 6);
  SPI.transfer(pec);

  SPI_setSlaveSelect(true);
}



/* Read configuration register from monitor */
void SPI_readConfigurationRegister()
{
  int i = 0;
  unsigned char in_pec = 0;
  unsigned char in_pec_own = 0;
  unsigned char out_pec = calculatePECForByte(MON_READ_CONF_REG , 0 , true);

  SPI_setSlaveSelect(false);
  SPI.transfer(MON_READ_CONF_REG);
  SPI.transfer(out_pec);

  i = 0;
  while( i < MON_SIZE_OF_CONF_REG )
  {
    MON_configuration_register[i] = SPI.transfer(0);
    i++;
  }
  in_pec = SPI.transfer(0);
  in_pec_own = calculatePECForByteArray(MON_configuration_register , 6);
  SPI_setSlaveSelect(true);

  if(__DEBUG__ && err_msg_print_allowed) {
    if( in_pec != in_pec_own ) {
      Serial.println(F("\nCONFIGURATION REGISTER PECs MISMATCH!"));
      Serial.print(F("Pec received: "));
      printByte(in_pec);
      Serial.print(F("Pec calculated: "));
      printByte(in_pec_own);
      Serial.println("");
    }
  }

  return;
}






void SPI_readAllVoltages()
{
  int i = 0;
  int value_ctr = 0;
  int bytes_read_not_handled = 0;
  word high_byte = 0;
  word low_byte = 0;
  unsigned char rec_byte;
  unsigned char pec_out = calculatePECForByte( MON_READ_ALL_VOLTAGES , 0 , true);
  unsigned char pec_in = 0;
  unsigned char pec_in_own = initPEC();

  SPI_setSlaveSelect(false);
  SPI.transfer(MON_READ_ALL_VOLTAGES);
  SPI.transfer(pec_out);

  i = 0;
  value_ctr = 0;
  while( i < 18 ) {
    rec_byte = SPI.transfer(0);
    pec_in_own = calculatePECForByte(rec_byte , pec_in_own , false);

    if( bytes_read_not_handled == 0 ) {
      low_byte = rec_byte;

      bytes_read_not_handled = 8;
    }

    else if( bytes_read_not_handled == 8 ) {
      high_byte = (rec_byte & 0x0F);
      MON_voltages[value_ctr] = low_byte + (high_byte << 8);

      low_byte = (rec_byte & 0xF0);
      low_byte = (low_byte >> 4);

      bytes_read_not_handled = 4;
      value_ctr++;
    }

    else {
      high_byte = rec_byte;
      MON_voltages[value_ctr] = low_byte + (high_byte << 4);

      bytes_read_not_handled = 0;
      value_ctr++;
    }

    i++;
  } /* END OF WHILE */


  pec_in = SPI.transfer(0);
  SPI_setSlaveSelect(true);

  if(__DEBUG__ && err_msg_print_allowed) {
    if( pec_in != pec_in_own ) {
      Serial.println(F("\nVOLTAGE PECs MISMATCH!"));
      Serial.print(F("Pec received: "));
      printByte(pec_in);
      Serial.print(F("Pec calculated: "));
      printByte(pec_in_own);
      Serial.println("");
    }
  }

  return;
}






void SPI_readDiagnostics()
{
  word high_byte = 0;
  unsigned char rec_byte = 0;
  unsigned char out_pec = calculatePECForByte(MON_READ_DIAG_REG , 0 , true);
  unsigned char in_pec = 0;
  unsigned char in_pec_own = initPEC();

  SPI_setSlaveSelect(false);
  SPI.transfer(MON_READ_DIAG_REG);
  SPI.transfer(out_pec);

  rec_byte = SPI.transfer(0);
  in_pec_own = calculatePECForByte(rec_byte , in_pec_own , false);
  MON_DIAG_reference_voltage = rec_byte;
  rec_byte = SPI.transfer(0);
  in_pec_own = calculatePECForByte(rec_byte , in_pec_own , false);

  MON_DIAG_muxfail = (rec_byte & 0x20);

  MON_DIAG_revision_number = (rec_byte & 0xC0);
  MON_DIAG_revision_number = (MON_DIAG_revision_number >> 6);

  high_byte = (rec_byte & 0x0F);
  MON_DIAG_reference_voltage += (high_byte << 8);

  in_pec = SPI.transfer(0);
  SPI_setSlaveSelect(true);

  if(__DEBUG__ && err_msg_print_allowed) {
    if( in_pec != in_pec_own ) {
      Serial.println(F("\nDIAGNOSTICS PECs MISMATCH!"));
      Serial.print(F("Pec received: "));
      printByte(in_pec);
      Serial.print(F("Pec calculated: "));
      printByte(in_pec_own);
      Serial.println("");
    }
  }

  return;
}






void SPI_sendCommandToMonitor(unsigned char cmd)
{
  unsigned char pec = calculatePECForByte(cmd , 0 , true);

  SPI_setSlaveSelect(false);
  SPI.transfer(cmd);
  SPI.transfer(pec);
  SPI_setSlaveSelect(true);

  return;
}
