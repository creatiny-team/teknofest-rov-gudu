#include <SPI.h>
#include <Servo.h>
#include <mcp2515.h>
#include <Wire.h>
#include <SparkFun_MS5803_I2C.h>

struct can_frame canMsg; // gelen mesaj
struct can_frame canSnd; // giden mesaj

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
} converter;

union ArrayToDouble {
    byte array[4];
    double number;
} doubler;

MS5803 prsensor(ADDRESS_HIGH); // basinc - sicaklik sensoru

//Create variables to store results
float temperature_c;
double pressure_abs;

void setup()
{
    Serial.begin(9600);
    Serial.println("Basladi.");
    SPI.begin();
    startMillis = millis(); // guncel zaman

    mcp2515.reset();
    mcp2515.setBitrate(CAN_125KBPS);
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

        /*
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
*/
        if (canMsg.can_id == 0x02)
        {
            for (int i = 0; i < canMsg.can_dlc; i += 2)
            {
                converter.array[1] = canMsg.data[i];
                converter.array[0] = canMsg.data[i + 1];
                if (i == 0)
                    valueJoyStick_X_1 = converter.integer;
                else if (i == 2)
                    valueJoyStick_Y_1 = converter.integer;
                else if (i == 4)
                    valueJoyStick_X_2 = converter.integer;
                else if (i == 6)
                    valueJoyStick_Y_2 = converter.integer;
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
            
            Serial.print(valueJoyStick_X_1);
            Serial.print("--");
            Serial.print(valueJoyStick_Y_1);
            Serial.print("--");
            Serial.print(valueJoyStick_X_2);
            Serial.print("--");
            Serial.print(valueJoyStick_Y_2);
            Serial.println("--");
        }
    }
    
    if (currentMillis - startMillis >= period) //test whether the period has elapsed
    {
        pressure_abs = prsensor.getPressure(ADC_4096);
        temperature_c = prsensor.getTemperature(CELSIUS, ADC_4096);

        Serial.println(pressure_abs);
        Serial.println(temperature_c);

        canSnd.can_id = 0x03;
        canSnd.can_dlc = 8;
        doubler.number = pressure_abs;
        canSnd.data[0] = doubler.array[0];
        canSnd.data[1] = doubler.array[1];
        canSnd.data[2] = doubler.array[2];
        canSnd.data[3] = doubler.array[3];
        doubler.number = temperature_c;
        canSnd.data[4] = doubler.array[0];
        canSnd.data[5] = doubler.array[1];
        canSnd.data[6] = doubler.array[2];
        canSnd.data[7] = doubler.array[3];

        startMillis = currentMillis; //IMPORTANT to save the start time of the current LED state.
        mcp2515.sendMessage(&canSnd);
    }
}
