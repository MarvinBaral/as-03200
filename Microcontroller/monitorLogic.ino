#ifdef __DEMO__
#define MON_CONF_OVERVOLTAGE    4000    /* DEMO mV */
#define MON_CONF_UNDERVOLTAGE   3650    /* DEMO mV */
#endif
#ifndef __DEMO__
#define MON_CONF_OVERVOLTAGE    4200    /* mV */
#define MON_CONF_UNDERVOLTAGE   3700    /* mV */
#endif


#define MON_CONF_START_BALANCING_THRESH    30  /* mV */
#define MON_CONF_STOP_BALANCING_THRESH     60  /* mV */


char voltIndicator[ 12 ];




/****************************/
/* MODIFY MONITOR BEHAVIOUR */
/****************************/

/* Toggle GPIO Led 1 in monitor configuration register */
void MON_toggleGPIOLed1(boolean toggle)
{
  if(toggle) {
    MON_configuration_register_local[0] = (MON_configuration_register_local[0] & 0xBF);
  }
  else {
    MON_configuration_register_local[0] = (MON_configuration_register_local[0] | 0x40);
  }
}

/* Toggle GPIO Led 2 in monitor configuration register */
void MON_toggleGPIOLed2(boolean toggle)
{
  if(toggle) {
    MON_configuration_register_local[0] = (MON_configuration_register_local[0] & 0xDF);
  }
  else {
    MON_configuration_register_local[0] = (MON_configuration_register_local[0] | 0x20);
  }
}

void MON_dischargeCell(byte cell, boolean toggle)
{
  byte discharge;
  byte stop_discharge;
  byte local_register = 1;
  if (cell > 7) {
    local_register = 2;
  }
  switch (cell) {
    case 0:
      discharge = 0x01;
      stop_discharge = 0xFE;
      break;
    case 1:
      discharge = 0x02;
      stop_discharge = 0xFC;
      break;
    case 2:
      discharge = 0x04;
      stop_discharge = 0xFB;
      break;
    case 3:
      discharge = 0x08;
      stop_discharge = 0xF7;
      break;
    case 4:
      discharge = 0x10;
      stop_discharge = 0xEF;
      break;
    case 5:
      discharge = 0x20;
      stop_discharge = 0xDF;
      break;
    case 6:
      discharge = 0x40;
      stop_discharge = 0xBF;
      break;
    case 7:
      discharge = 0x80;
      stop_discharge = 0x7F;
      break;
    case 8:
      discharge = 0x01;
      stop_discharge = 0xFE;
      break;
    case 9:
      discharge = 0x02;
      stop_discharge = 0xFD; //??? not 0xFC?
      break;
    case 10:
      discharge = 0x04;
      stop_discharge = 0xFB;
      break;
    case 11:
      discharge = 0x08;
      stop_discharge = 0xF7;
      break;
    
  }
  if(toggle) {
    MON_configuration_register_local[local_register] = (MON_configuration_register_local[1] | discharge);
  }
  else {
    MON_configuration_register_local[local_register] = (MON_configuration_register_local[1] & stop_discharge);
  }
}

/* Set comparator duty cycle. Value is a number between 0-7. See LTC6803 datasheet. */
void MON_setComparatorDutyCycle(unsigned char level)
{
  if( 8 > level ) {
    MON_configuration_register_local[0] = (MON_configuration_register_local[0] & 0xF8);
    MON_configuration_register_local[0] = (MON_configuration_register_local[0] | level);
  }
  else if(__DEBUG__) {
    Serial.println(F("ERR: Trying to set invalid ADC conversion level!"));
  }
}







void checkVoltages()
{
  int i = 0;
  while( i < 12 ) {
    if( MON_voltages[i] > ((MON_CONF_OVERVOLTAGE - MON_CONF_START_BALANCING_THRESH) / 1.5) + 512 ) {
      voltIndicator[i] = 1;
    }
    else if( MON_voltages[i] < ((MON_CONF_UNDERVOLTAGE + MON_CONF_START_BALANCING_THRESH)/ 1.5) + 512 ) {
      voltIndicator[i] = -1;
    }
    else {
      voltIndicator[i] = 0;
    }
    i++;
  }
}
