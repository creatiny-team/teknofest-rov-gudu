#include <Servo.h>
#include <Wire.h>
#include <mcp2515.h>             //https://github.com/autowp/arduino-mcp2515
#include <SparkFun_MS5803_I2C.h> //https://github.com/sparkfun/SparkFun_MS5803-14BA_Breakout_Arduino_Library

struct can_frame canMsg; // gelen mesaj icin can struct
struct can_frame canSnd; // giden mesaj icin can struct

MCP2515 mcp2515(10);

int valueJoyStick_X_1 = 0;
int valueJoyStick_Y_1 = 0;
int valueJoyStick_X_2 = 0;
int valueJoyStick_Y_2 = 0;

unsigned long startMillis; //program süre sayıcısının başlangıç zaman değişkeni
unsigned long currentMillis;
const unsigned long period = 1000; // basınç sensörü ölçüm aralığı

Servo on, arka, onsa, onso, arsa, arso;

int on_deger, arka_deger, onsa_deger, onso_deger, arsa_deger, arso_deger;

union ArrayToInteger {
    byte array[2];
    int integer;
} intConverter;

union ArrayToDouble {
    byte array[4];
    double number;
} doubleConverter;

MS5803 prsensor(ADDRESS_HIGH); // basinc - sicaklik sensoru

float sicaklik;
double basinc;

void setup()
{
    Serial.begin(9600);
    Serial.println("Basladi");
    SPI.begin();
    startMillis = millis(); // guncel zaman

    mcp2515.reset();
    mcp2515.setBitrate(CAN_40KBPS);
    mcp2515.setNormalMode();

    on.attach(7, 1000, 2000);
    arka.attach(3, 1000, 2000);
    onsa.attach(4, 1000, 2000);
    onso.attach(6, 1000, 2000);
    arsa.attach(8, 1000, 2000);
    arso.attach(5, 1000, 2000);

    prsensor.reset();
    prsensor.begin();
}

void loop()
{
    currentMillis = millis();

    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
    {
        // Serial.print(canMsg.can_id, HEX);
        // Serial.print(" "); 
        // Serial.print(canMsg.can_dlc, HEX);
        // Serial.print(" ");
        if (canMsg.can_id == 0x02)
        {
            for (int i = 0; i < canMsg.can_dlc; i += 2)
            {
                intConverter.array[1] = canMsg.data[i];
                intConverter.array[0] = canMsg.data[i + 1];
                if (i == 0)
                    valueJoyStick_X_1 = intConverter.integer;
                else if (i == 2)
                    valueJoyStick_Y_1 = intConverter.integer;
                else if (i == 4)
                    valueJoyStick_X_2 = intConverter.integer;
                else if (i == 6)
                    valueJoyStick_Y_2 = intConverter.integer;
            }

            on.writeMicroseconds(valueJoyStick_X_1);
            arka.writeMicroseconds(valueJoyStick_X_1);
            onsa_deger = 1500 + (valueJoyStick_X_2 - 1500) - (valueJoyStick_Y_2 - 1500) + (valueJoyStick_Y_1 - 1500);
            onso_deger = 1500 + (valueJoyStick_X_2 - 1500) + (valueJoyStick_Y_2 - 1500) - (valueJoyStick_Y_1 - 1500);
            arsa_deger = 1500 + (valueJoyStick_X_2 - 1500) + (valueJoyStick_Y_2 - 1500) + (valueJoyStick_Y_1 - 1500);
            arso_deger = 1500 + (valueJoyStick_X_2 - 1500) - (valueJoyStick_Y_2 - 1500) - (valueJoyStick_Y_1 - 1500);

            Serial.println(onsa_deger);
            Serial.println(onso_deger);
            Serial.println(arsa_deger);
            Serial.println(arso_deger);

            if (onsa_deger >= 2000)
                onsa_deger = 2000;
            else if (onsa_deger <= 1000)
                onsa_deger = 1000;
            if (arsa_deger >= 2000)
                arsa_deger = 2000;
            else if (arsa_deger <= 1000)
                onsa_deger = 1000;
            if (onso_deger >= 2000)
                onso_deger = 2000;
            else if (onso_deger <= 1000)
                onsa_deger = 1000;
            if (arso_deger >= 2000)
                arso_deger = 2000;
            else if (arso_deger <= 1000)
                onsa_deger = 1000;
            onsa.writeMicroseconds(onsa_deger);
            onso.writeMicroseconds(onso_deger);
            arsa.writeMicroseconds(arsa_deger);
            arso.writeMicroseconds(arso_deger);
            delay(100);
            /*
            Serial.print(valueJoyStick_X_1);
            Serial.print("--");
            Serial.print(valueJoyStick_Y_1);
            Serial.print("--");
            Serial.print(valueJoyStick_X_2);
            Serial.print("--");
            Serial.print(valueJoyStick_Y_2);
            Serial.println("--"); */
        }
    }

    if (currentMillis - startMillis >= period)
    {
        basinc = prsensor.getPressure(ADC_4096);
        sicaklik = prsensor.getTemperature(CELSIUS, ADC_4096);

        Serial.println(basinc);
        Serial.println(sicaklik);

        canSnd.can_id = 0x03;
        canSnd.can_dlc = 8;
        doubleConverter.number = basinc;
        canSnd.data[0] = doubleConverter.array[0];
        canSnd.data[1] = doubleConverter.array[1];
        canSnd.data[2] = doubleConverter.array[2];
        canSnd.data[3] = doubleConverter.array[3];
        doubleConverter.number = sicaklik;
        canSnd.data[4] = doubleConverter.array[0];
        canSnd.data[5] = doubleConverter.array[1];
        canSnd.data[6] = doubleConverter.array[2];
        canSnd.data[7] = doubleConverter.array[3];

        startMillis = currentMillis;
        mcp2515.sendMessage(&canSnd);
    }
}
