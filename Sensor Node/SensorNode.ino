/*****************************************************************
WSN for monitoring gases in coal mines
Sensor Node
*****************************************************************/
// Libraries to 1-Wire protocol and Sensor Temperature DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
// Librarie SoftwareSerial to communicate with the XBee
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11);
const int pinDatosDQ = 52; // Pin where the 1-Wire bus connects
bool bypassMode = false;

// Instance OneWire and DallasTemperature classes
OneWire oneWireObjeto(pinDatosDQ);
DallasTemperature sensorDS18B20(&oneWireObjeto);

//Parameters of frame
byte start_delimiter = 0x7E;
byte length_frame[] = {0x00, 0x13};
byte frame_type = 0x00;
byte frame_id = 0x01;
byte dest_address_high [] = {0x00, 0x13, 0xA2, 0x00}; // Destination address high
byte dest_address_low [] = {0x41, 0x54, 0xD7, 0x68};  // Destination address low
byte options = 0x00;
byte tx_data [] = {0x08};
byte checksum;
int check;

byte byp[] = {0x42};

// Configure analog input to methane sensor
int sensorCH4 = A8;

// Configure analog input to oxygen sensor
int sensorO2 = A9;

// Configure analog input to methane sensor
int sensorCO = A10;

// Sensors Variables
float temperature;
int methane;
int oxygen;
int carbonMonoxide;

void setup()
{
  // Set up Xbee protocol
  XBee.begin(9600);
  
  // Set up 1-Wire protocol
  sensorDS18B20.begin();
}


void checkBypassMode()
{
  if (bypassMode == false)
  {
    XBee.write("B");
    XBee.write(byp, 1);
    bypassMode = true;
  }
}

void loop()
{

  delay(5000);
  
  // Send request to sense temperature
  sensorDS18B20.requestTemperatures();
  temperature = sensorDS18B20.getTempCByIndex(0);
  int intTemperature = temperature;
  int decTemperature = (temperature - intTemperature) * 100;

  // Obtain methane level
  methane = analogRead(sensorCH4);
  float sigCH4 = (0.03 * methane) - 2.55;
  if (sigCH4 < 0)
  {
    sigCH4 = 0; 
  }
  int intMethane = sigCH4;
  int decMethane = (sigCH4 - intMethane) * 100;

  // Obtain oxygen level
  oxygen = analogRead(sensorO2);
  int intOxygen = oxygen;

  // Obtain carbon monoxide level
  carbonMonoxide = analogRead(sensorCO);
  float sigCO = (1.8 * carbonMonoxide) - 108.1;
  if (sigCO < 0)
  {
    sigCO = 0; 
  }
  carbonMonoxide = sigCO;
  
  // build data package
  tx_data[1] = intTemperature;
  tx_data[2] = decTemperature;
  tx_data[3] = intMethane;
  tx_data[4] = decMethane;
  tx_data[5] = intOxygen;
  tx_data[6] = ((carbonMonoxide/10)%10)*10 + carbonMonoxide%10;
  tx_data[7] = ((carbonMonoxide/1000)%10)*10 + ((carbonMonoxide/100)%10);

  // Obtain Checksum
  check = frame_type + frame_id + dest_address_high[0] + dest_address_high[1] + dest_address_high[2] + dest_address_high[3] + dest_address_low[0] + dest_address_low[1] + 
  dest_address_low[2] + dest_address_low[3] + options + tx_data[0] + tx_data[1] + tx_data[2] + tx_data[3] + tx_data[4] + tx_data[5] + tx_data[6] + tx_data[7];
  check = check & 0xFF;
  check = 0xFF - check;
  checksum = (byte) check;

  // Build frame
  byte frame [] = {start_delimiter, length_frame[0], length_frame[1], frame_type, frame_id, dest_address_high[0], dest_address_high[1], dest_address_high[2], 
  dest_address_high[3], dest_address_low[0], dest_address_low[1], dest_address_low[2], dest_address_low[3], options, tx_data[0], tx_data[1], tx_data[2], tx_data[3], 
  tx_data[4], tx_data[5], tx_data[6], tx_data[7], checksum};

  // Check Bypass mode and send data package
  checkBypassMode();

  // Send data
  XBee.write(frame, 23);
  
}
