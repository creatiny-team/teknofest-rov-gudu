#include <mcp2515.h>  //https://github.com/autowp/arduino-mcp2515

#define maxdeger 2000 //max 2000 oluyor escler 1000-2000 arası calisir
#define mindeger 1000
#define sabitleme_toleransi 30

#define pinJoyStick_X_1 2
#define pinJoyStick_Y_1 3
#define pinJoyStick_X_2 0
#define pinJoyStick_Y_2 1

struct can_frame canSend;
struct can_frame canRcv;

MCP2515 mcp2515(10);

float hizBoleni = 1.0;

int valueJoyStick_X_1 = 0;
int valueJoyStick_Y_1 = 0;
int valueJoyStick_X_2 = 0;
int valueJoyStick_Y_2 = 0;

union ArrayToInteger {
    byte array[2];
    int integer;
} intConverter;
union ArrayToDouble {
    byte array[4];
    double number;
} doubleConverter;

void setup()
{
    Serial.begin(9600);

    mcp2515.reset();
    mcp2515.setBitrate(CAN_40KBPS);
    mcp2515.setNormalMode();

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(8, INPUT);
    pinMode(7, INPUT);
    pinMode(6, INPUT);
    pinMode(9, INPUT);
}

void loop()
{
    // bilgisayardan gelen hiz bolucu deger, normalde gelen 10.
    if (Serial.available())
    {
        int incomingByte = Serial.read();
        hizBoleni = incomingByte / 10.0;
    }

    // rov'dan gelen basinc ve sicaklik degerleri bilgisayar okusun diye serial'den basiliyor
    if (mcp2515.readMessage(&canRcv) == MCP2515::ERROR_OK)
    {
        if (canRcv.can_id == 0x03)
        {
            doubleConverter.array[0] = canRcv.data[0];
            doubleConverter.array[1] = canRcv.data[1];
            doubleConverter.array[2] = canRcv.data[2];
            doubleConverter.array[3] = canRcv.data[3];
            Serial.print('b'); // basinc belirteci
            Serial.println(doubleConverter.number);
            doubleConverter.array[0] = canRcv.data[4];
            doubleConverter.array[1] = canRcv.data[5];
            doubleConverter.array[2] = canRcv.data[6];
            doubleConverter.array[3] = canRcv.data[7];
            Serial.print('s'); // sicaklik belirteci
            Serial.println(doubleConverter.number);
        }
    }

    // joystick'lerde ortada gelen veriyi 1500'e esitlemek icin
    valueJoyStick_X_1 = analogRead(pinJoyStick_X_1) + 970;
    valueJoyStick_Y_1 = analogRead(pinJoyStick_Y_1) + 970;
    valueJoyStick_X_2 = analogRead(pinJoyStick_X_2) + 970;
    valueJoyStick_Y_2 = analogRead(pinJoyStick_Y_2) + 970;

    // hizi merkezi bozmadan bolmek icin
    valueJoyStick_X_1 = 1500 + (valueJoyStick_X_1 - 1500) / hizBoleni;
    valueJoyStick_Y_1 = 1500 + (valueJoyStick_Y_1 - 1500) / hizBoleni;
    valueJoyStick_X_2 = 1500 + (valueJoyStick_X_2 - 1500) / hizBoleni;
    valueJoyStick_Y_2 = 1500 + (valueJoyStick_Y_2 - 1500) / hizBoleni;

    if (valueJoyStick_X_1 > maxdeger)
        valueJoyStick_X_1 = maxdeger;
    if (valueJoyStick_Y_1 > maxdeger)
        valueJoyStick_Y_1 = maxdeger;
    if (valueJoyStick_X_2 > maxdeger)
        valueJoyStick_X_2 = maxdeger;
    if (valueJoyStick_Y_2 > maxdeger)
        valueJoyStick_Y_2 = maxdeger;

    if (valueJoyStick_X_1 < mindeger)
        valueJoyStick_X_1 = mindeger;
    if (valueJoyStick_Y_1 < mindeger)
        valueJoyStick_Y_1 = mindeger;
    if (valueJoyStick_X_2 < mindeger)
        valueJoyStick_X_2 = mindeger;
    if (valueJoyStick_Y_2 < mindeger)
        valueJoyStick_Y_2 = mindeger;

    // joystick'ler belli bi toleransla ortadayken 1500'e sabitliyoruz
    if (valueJoyStick_X_1 < 1458 + sabitleme_toleransi / hizBoleni && valueJoyStick_X_1 > 1458 - sabitleme_toleransi / hizBoleni)
        valueJoyStick_X_1 = 1458;
    if (valueJoyStick_Y_1 < 1500 + sabitleme_toleransi / hizBoleni && valueJoyStick_Y_1 > 1500 - sabitleme_toleransi / hizBoleni)
        valueJoyStick_Y_1 = 1500;
    if (valueJoyStick_X_2 < 1470 + sabitleme_toleransi / hizBoleni && valueJoyStick_X_2 > 1470 - sabitleme_toleransi / hizBoleni)
        valueJoyStick_X_2 = 1470;
    if (valueJoyStick_Y_2 < 1500 + sabitleme_toleransi / hizBoleni && valueJoyStick_Y_2 > 1500 - sabitleme_toleransi / hizBoleni)
        valueJoyStick_Y_2 = 1500;

    // joystick degerleri 8 byte'a siralanip yollaniyor
    canSend.can_id = 0x02;
    canSend.can_dlc = 8;
    canSend.data[0] = highByte(valueJoyStick_X_1);
    canSend.data[1] = lowByte(valueJoyStick_X_1);
    canSend.data[2] = highByte(valueJoyStick_Y_1);
    canSend.data[3] = lowByte(valueJoyStick_Y_1);
    canSend.data[4] = highByte(valueJoyStick_X_2);
    canSend.data[5] = lowByte(valueJoyStick_X_2);
    canSend.data[6] = highByte(valueJoyStick_Y_2);
    canSend.data[7] = lowByte(valueJoyStick_Y_2);

    // q-w-e-r her degerin bilgisayardan ayirt edilmesi icin ayri kodlar
    Serial.print('q');
    Serial.println(valueJoyStick_X_1);
    Serial.print('w');
    Serial.println(valueJoyStick_Y_1);
    Serial.print('e');
    Serial.println(valueJoyStick_X_2);
    Serial.print('r');
    Serial.println(valueJoyStick_Y_2);

    mcp2515.sendMessage(&canSend);
    delay(100);
}
