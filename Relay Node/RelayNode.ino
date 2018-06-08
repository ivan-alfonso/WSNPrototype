/*****************************************************************
WSN for monitoring gases in coal mines
Relay Node
*****************************************************************/
// Libraries to 1-Wire protocol and Sensor Temperature DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
// Librarie SoftwareSerial to communicate with the XBee
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11);
bool bypassMode = false;

// Parameters of frame
byte start_delimiter = 0x7E;
byte length_frame[] = {0x00, 0x13};
byte frame_type = 0x00;
byte frame_id = 0x01;
byte dest_address_high [] = {0x00, 0x13, 0xA2, 0x00}; // Destination address high
byte dest_address_low [] = {0x41, 0x57, 0x4B, 0xA5};  // Destination address low
byte options = 0x00;
byte tx_data [8];
byte checksum;
int check;

byte byp[] = {0x42};

// Sensors Variables
float temperature;
int methane;
int oxygen;
int carbonMonoxide;
byte dataPackage;
int count = 0;

void setup()
{
  // Set up Xbee protocol
  XBee.begin(9600);
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
  delay(10);
  checkBypassMode();
  if (XBee.available())
  {
    dataPackage = XBee.read();
    if (dataPackage == 0x7E)
    {
      count = 0;
    }
    count = count + 1;
    if (count >= 16 and count <= 23)
    {
      tx_data[count-16] = dataPackage;
    }
    if (count == 24)
    {
      // Obtain Checksum
      check = frame_type + frame_id + dest_address_high[0] + dest_address_high[1] + dest_address_high[2] + dest_address_high[3] + dest_address_low[0] + dest_address_low[1] + 
      dest_address_low[2] + dest_address_low[3] + options + tx_data[0] + tx_data[1] + tx_data[2] + tx_data[3] + tx_data[4] + tx_data[5] + tx_data[6] + tx_data[7];
      check = check & 0xFF;
      check = 0xFF - check;
      checksum = (byte) check;

      byte frame [] = {start_delimiter, length_frame[0], length_frame[1], frame_type, frame_id, dest_address_high[0], dest_address_high[1], dest_address_high[2], 
      dest_address_high[3], dest_address_low[0], dest_address_low[1], dest_address_low[2], dest_address_low[3], options, tx_data[0], tx_data[1], tx_data[2], tx_data[3],
      tx_data[4], tx_data[5], tx_data[6], tx_data[7], checksum};
      checkBypassMode();
      
      // Send data
      XBee.write(frame, 23);
    }
  }
}
