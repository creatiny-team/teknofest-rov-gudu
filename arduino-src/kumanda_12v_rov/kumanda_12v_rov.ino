#include <Servo.h>
#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg1;
struct can_frame canMsg2;

struct can_frame canRcv;
MCP2515 mcp2515(10);

float hizBoleni = 1.0;

int maxdeger = 2000; //max 2000 oluyor escler 1000-2000 arası calisir
int mindeger = 3000 - maxdeger;
int pressure;
int pinJoyStick_X_1 = 2;
int pinJoyStick_Y_1 = 3;
int pinJoyStick_X_2 = 1;
int pinJoyStick_Y_2 = 0;

int valueJoyStick_X_1 = 0;
int valueJoyStick_Y_1 = 0;
int valueJoyStick_X_2 = 0;
int valueJoyStick_Y_2 = 0;

union ArrayToInteger {
    byte array[2];
    int integer;
} converter;
union ArrayToDouble {
    byte array[4];
    double doubl;
} doubler;
void setup()
{
    Serial.begin(9600);
    SPI.begin();

    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
    mcp2515.setNormalMode();

    /*canMsg2.can_id = 0x03;
    canMsg2.can_dlc = 8;
    canMsg2.data[0] = 0x01;
    canMsg2.data[1] = 0x01;
    canMsg2.data[2] = 0x01;
    canMsg2.data[3] = 0x00;
    canMsg2.data[4] = 0x00;
    canMsg2.data[5] = 0x00;
    canMsg2.data[6] = 0x00;
    canMsg2.data[7] = 0xA0;*/

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(8, INPUT);
    pinMode(7, INPUT);
    pinMode(6, INPUT);
    pinMode(9, INPUT);

    //while (!Serial);
}

void loop()
{
    if(Serial.available())
    {
      int incomingByte = Serial.read();
      hizBoleni = incomingByte / 10.0;
    }
    if (mcp2515.readMessage(&canRcv) == MCP2515::ERROR_OK)
    {

        if (canRcv.can_id == 0x03)
        {

            doubler.array[0] = canRcv.data[0];
            doubler.array[1] = canRcv.data[1];
            doubler.array[2] = canRcv.data[2];
            doubler.array[3] = canRcv.data[3];
            Serial.print('b');
            Serial.println(doubler.doubl);
            doubler.array[0] = canRcv.data[4];
            doubler.array[1] = canRcv.data[5];
            doubler.array[2] = canRcv.data[6];
            doubler.array[3] = canRcv.data[7];
            Serial.print('s');
            Serial.println(doubler.doubl);
        }
    }

    valueJoyStick_X_1 = analogRead(pinJoyStick_X_1) + 970;
    valueJoyStick_Y_1 = analogRead(pinJoyStick_Y_1) + 970;
    valueJoyStick_X_2 = analogRead(pinJoyStick_X_2) + 970;
    valueJoyStick_Y_2 = analogRead(pinJoyStick_Y_2) + 970;

    valueJoyStick_X_2 = 3000 - valueJoyStick_X_2;
    valueJoyStick_Y_2 = 3000 - valueJoyStick_Y_2;

    valueJoyStick_X_1 = 1500 + (valueJoyStick_X_1 - 1500) / hizBoleni;
    valueJoyStick_Y_1 = 1500 + (valueJoyStick_Y_1 - 1500) / hizBoleni;
    valueJoyStick_X_2 = 1500 + (valueJoyStick_X_2 - 1500) / hizBoleni;
    valueJoyStick_Y_2 = 1500 + (valueJoyStick_Y_2 - 1500) / hizBoleni;
    valueJoyStick_X_1 = 3000 - valueJoyStick_X_1;
    if (valueJoyStick_X_1 > maxdeger)
        valueJoyStick_X_1 = maxdeger;

    if (valueJoyStick_Y_1 > maxdeger)
        valueJoyStick_Y_1 = maxdeger;

    if (valueJoyStick_X_2 > maxdeger)
        valueJoyStick_X_2 = maxdeger;

    if (valueJoyStick_X_1 < mindeger)
        valueJoyStick_X_1 = mindeger;

    if (valueJoyStick_Y_1 < mindeger)
        valueJoyStick_Y_1 = mindeger;

    if (valueJoyStick_X_2 < mindeger)
        valueJoyStick_X_2 = mindeger;

    if (valueJoyStick_X_1 < 1530 && valueJoyStick_X_1 > 1470)
        valueJoyStick_X_1 = 1500;

    if (valueJoyStick_Y_1 < 1530 && valueJoyStick_Y_1 > 1470)
        valueJoyStick_Y_1 = 1500;

    if (valueJoyStick_X_2 < 1530 && valueJoyStick_X_2 > 1470)
        valueJoyStick_X_2 = 1500;

    if (valueJoyStick_Y_2 < 1530 && valueJoyStick_Y_2 > 1470)
        valueJoyStick_Y_2 = 1500;

    canMsg1.can_id = 0x02;
    canMsg1.can_dlc = 8;
    canMsg1.data[0] = highByte(valueJoyStick_X_1);
    canMsg1.data[1] = lowByte(valueJoyStick_X_1);
    canMsg1.data[2] = highByte(valueJoyStick_Y_1);
    canMsg1.data[3] = lowByte(valueJoyStick_Y_1);
    canMsg1.data[4] = highByte(valueJoyStick_X_2);
    canMsg1.data[5] = lowByte(valueJoyStick_X_2);
    canMsg1.data[6] = highByte(valueJoyStick_Y_2);
    canMsg1.data[7] = lowByte(valueJoyStick_Y_2);

    /* canMsg2.can_id = 0x03;
    canMsg2.can_dlc = 8;
    canMsg2.data[0] = bag1;//bag1
  canMsg2.data[1] = bag2;//bag2
  canMsg2.data[2] = kol;
  /*canMsg2.data[3] = 0x01;
  canMsg2.data[4] = 0x01;
  canMsg2.data[5] = 0x01;
  canMsg2.data[6] = 0x01;
  canMsg2.data[7] = 0xA0;*/

    Serial.print('q');
    Serial.println(valueJoyStick_X_1);
    Serial.print('w');
    Serial.println(valueJoyStick_Y_1);
    Serial.print('e');
    Serial.println(valueJoyStick_X_2);
    Serial.print('r');
    Serial.println(valueJoyStick_Y_2);

    if (valueJoyStick_Y_1 < 1550 && valueJoyStick_Y_1 > 1450)
        valueJoyStick_Y_1 = 1500;

    // if(Serial.available())
    //value = Serial.parseInt();    // Parse an Integer from Serial

    mcp2515.sendMessage(&canMsg1);
    //mcp2515.sendMessage(&canMsg2);
    delay(100);
}
