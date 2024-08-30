// library utama
#include <Arduino.h>

#define MAX_ITERATIONS 10

// Global variable and declaration
//-----------------------------------------------------------------------------------------------
// declare ldr
int ldrA1_pin = A0;
int ldrA2_pin = A1;
int ldrA3_pin = A2;
int ldrA4_pin = A3;
int ldrA5_pin = A4;
int ldrB1_pin = A5;
int ldrB2_pin = A6;
int ldrB3_pin = A7;
int ldrB4_pin = A8;
int ldrB5_pin = A9;
int ldrC1_pin = A10;
int ldrC2_pin = A11;
int ldrC3_pin = A12;
int ldrC4_pin = A13;
int ldrC5_pin = A14;

// global variable Utama
unsigned int timer = 0, timerinterval = 1000;

// global variable ldr
int ldrA1, ldrA2, ldrA3, ldrA4, ldrA5;
int ldrB1, ldrB2, ldrB3, ldrB4, ldrB5;
int ldrC1, ldrC2, ldrC3, ldrC4, ldrC5;

int rerataAkhirLDRTray1 = 0;
int rerataAkhirLDRTray2 = 0;
int rerataAkhirLDRTray3 = 0;

int nilaitengahldr1 = 549;
int nilaitengahldr2 = 427;
int nilaitengahldr3 = 488;

bool LDR1_terang = false;
bool LDR2_terang = false;
bool LDR3_terang = false;

// fungsi dari komponen
//-----------------------------------------------------------------------------------------------
void readldr()
{
    int rerataAwalLDRTray1 = 0;
    int rerataAwalLDRTray2 = 0;
    int rerataAwalLDRTray3 = 0;

    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        rerataAwalLDRTray1 += analogRead(ldrA1_pin);
        rerataAwalLDRTray1 += analogRead(ldrA2_pin);
        rerataAwalLDRTray1 += analogRead(ldrA3_pin);
        rerataAwalLDRTray1 += analogRead(ldrA4_pin);
        rerataAwalLDRTray1 += analogRead(ldrA5_pin);

        rerataAwalLDRTray2 += analogRead(ldrB1_pin);
        rerataAwalLDRTray2 += analogRead(ldrB2_pin);
        rerataAwalLDRTray2 += analogRead(ldrB3_pin);
        rerataAwalLDRTray2 += analogRead(ldrB4_pin);
        rerataAwalLDRTray2 += analogRead(ldrB5_pin);

        rerataAwalLDRTray3 += analogRead(ldrC1_pin);
        rerataAwalLDRTray3 += analogRead(ldrC2_pin);
        rerataAwalLDRTray3 += analogRead(ldrC3_pin);
        rerataAwalLDRTray3 += analogRead(ldrC4_pin);
        rerataAwalLDRTray3 += analogRead(ldrC5_pin);

        rerataAkhirLDRTray1 += rerataAwalLDRTray1 / 5;
        rerataAkhirLDRTray2 += rerataAwalLDRTray2 / 5;
        rerataAkhirLDRTray3 += rerataAwalLDRTray3 / 5;

        rerataAwalLDRTray1 = 0;
        rerataAwalLDRTray2 = 0;
        rerataAwalLDRTray3 = 0;
    }

    rerataAkhirLDRTray1 = rerataAkhirLDRTray1 / MAX_ITERATIONS;
    rerataAkhirLDRTray2 = rerataAkhirLDRTray2 / MAX_ITERATIONS;
    rerataAkhirLDRTray3 = rerataAkhirLDRTray3 / MAX_ITERATIONS;
}

// fungsi pendukung
//-----------------------------------------------------------------------------------------------

void monitorSerial()
{
    // Serial.print("\nnilai ADC LDR Tray 1 : ");
    // Serial.println(rerataAkhirLDRTray1);
    // Serial.print("nilai ADC LDR Tray 2 : ");
    // Serial.println(rerataAkhirLDRTray2);
    // Serial.print("nilai ADC LDR Tray 3 : ");
    // Serial.println(rerataAkhirLDRTray3);

    Serial.print("\nnilai ADC LDR Tray 1 : ");
    Serial.print(rerataAkhirLDRTray1);
    if (rerataAkhirLDRTray1 < nilaitengahldr1)
    {
        Serial.println(" || gelap");
        LDR1_terang = false;
    }
    else
    {
        Serial.println(" || terang");
        LDR1_terang = true;
    }

    Serial.print("nilai ADC LDR Tray 2 : ");
    Serial.print(rerataAkhirLDRTray2);
    if (rerataAkhirLDRTray2 < nilaitengahldr2)
    {
        Serial.println(" || gelap");
        LDR2_terang = false;
    }
    else
    {
        Serial.println(" || terang");
        LDR2_terang = true;
    }

    Serial.print("nilai ADC LDR Tray 3 : ");
    Serial.print(rerataAkhirLDRTray3);
    if (rerataAkhirLDRTray3 < nilaitengahldr3)
    {
        Serial.println(" || gelap");
        LDR3_terang = false;
    }
    else
    {
        Serial.println(" || terang");
        LDR3_terang = true;
    }

    if (LDR1_terang && LDR2_terang && LDR3_terang) // 111
    {
        Serial.println("AB+");
    }
    if (LDR1_terang && LDR2_terang && !LDR3_terang) // 110
    {
        Serial.println("AB-");
    }
    if (LDR1_terang && !LDR2_terang && LDR3_terang) // 101
    {
        Serial.println("A+");
    }
    if (LDR1_terang && !LDR2_terang && !LDR3_terang) // 100
    {
        Serial.println("A-");
    }

    if (!LDR1_terang && LDR2_terang && LDR3_terang) // 011
    {
        Serial.println("B+");
    }
    if (!LDR1_terang && LDR2_terang && !LDR3_terang) // 010
    {
        Serial.println("B-");
    }
    if (!LDR1_terang && !LDR2_terang && LDR3_terang) // 001
    {
        Serial.println("O+");
    }
    if (!LDR1_terang && !LDR2_terang && !LDR3_terang) // 000
    {
        Serial.println("O-");
    }
}
// fungsi utama
//-----------------------------------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);
}

void loop()
{
    // function to read along
    rerataAkhirLDRTray1 = 0;
    rerataAkhirLDRTray2 = 0;
    rerataAkhirLDRTray3 = 0;
    readldr();
    monitorSerial();

    delay(1000);
}