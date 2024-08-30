/*
start -> [init] -> open -> close -> run
*/
// library utama
#include <Arduino.h>
#include <StepperMotor.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include "Adafruit_Thermal.h"
#include <DFRobotDFPlayerMini.h>
#define MAX_ITERATIONS 10

// Global variable and declaration
//-----------------------------------------------------------------------------------------------
// declare indicatorLed
int redLed = 28;
int greenLed = 24;
int blueLed = 22;

// declare VoltageAvailable
int vavailPin = 26;

// declare Stepper
#define STEPPER_PIN_1 2
#define STEPPER_PIN_2 3
#define STEPPER_PIN_3 4
#define STEPPER_PIN_4 5
#define STEPPER_LIMIT_PIN 23
StepperMotor motor(STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);

// declare Servo
int servoStirPin = 6;     // Pin Arduino yang terhubung ke pin sinyal servo pertama
int servoMountingPin = 7; // Pin Arduino yang terhubung ke pin sinyal servo kedua

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

// declare printer
#define TX_PIN 9 // Pin untuk TX ke RX printer
#define RX_PIN 8 // Pin untuk RX dari TX printer

// declare sound
SoftwareSerial mySoftwareSerial(11, 10); // RX, TX

// global variable Utama
String GolonganDarah = "";
int kodeGolonganDarah = 0;
bool startStatus = false, processStatus = false;
unsigned int currentTimer = 0, timer = 0, timerinterval = 1000;
unsigned int currentTimerProcess = 0, timerProcess = 0, processTime = 8000; // TODO: atur sesuai kebutuhan

byte errorByte = 0;

// global variable indicatorLed
int delayblink = 100;

// global variable VoltageAvailable
bool vavailStatus = false;

// global variable Stepper
bool closeFull = false, hasSample = false;

// global variable Servo
Servo servoStir;     // Membuat objek untuk servo pertama
Servo servoMounting; // Membuat objek untuk servo kedua
unsigned int currentTimerServo = 0, timerServo = 0, timerIntervalServo = 20;
bool stirring = false, stirringCW = true;
int servoStirPos = 0;

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

// global variable printer
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX
Adafruit_Thermal printer(&mySerial);

// global variable sound
DFRobotDFPlayerMini myDFPlayer;

// fungsi dari komponen
//-----------------------------------------------------------------------------------------------
void setError(int errorNumber)
{
    if (errorNumber >= 1 && errorNumber <= 8)
    {
        errorByte |= (1 << (errorNumber - 1));
    }
}

void unsetError(int errorNumber)
{
    if (errorNumber >= 1 && errorNumber <= 8)
    {
        errorByte &= ~(1 << (errorNumber - 1));
    }
}

int hasError()
{
    for (int i = 0; i < 8; i++)
    {
        if (errorByte & (1 << i))
        {
            return i + 1;
        }
    }
    return 0;
}

void init_blink()
{
    pinMode(redLed, OUTPUT);
    pinMode(greenLed, OUTPUT);
    pinMode(blueLed, OUTPUT);
}

void vavail()
{
    vavailStatus = digitalRead(vavailPin);
    if (!vavailStatus)
    {
        setError(1);
    }
    else
    {
        unsetError(1);
    }
}

void checkTrayCLose()
{
    closeFull = digitalRead(STEPPER_LIMIT_PIN);
}

void motorMove(int command)
{
    switch (command)
    {
    case 0:
        Serial.println("Close Tray Full");
        closeFull = false;
        while (!closeFull)
        {
            checkTrayCLose();
            motor.stepMotor(10, false);
        }
        break;

    case 1:
        if (closeFull)
        {
            Serial.println("Open Tray Full");
            motor.stepMotor(14000, true); // TODO: HARUSNYA 12000
            hasSample = !hasSample;
            if (hasSample)
            {
                Serial.println("sample dimasukkan");
            }
            else
            {
                Serial.println("Sample dikeluarkan");
                GolonganDarah = "";
                kodeGolonganDarah = 0;
            }
        }
        break;

    case 2:
        if (closeFull)
        {
            Serial.println("Open Tray half");
            motor.stepMotor(5000, true); // TODO: HARUSNYA 5000
        }

    default:
        break;
    }
}

void initServo()
{
    servoStir.attach(servoStirPin);         // Menghubungkan objek servo pertama dengan pin servo
    servoMounting.attach(servoMountingPin); // Menghubungkan objek servo kedua dengan pin servo
    servoStir.write(0);
    servoMounting.write(180); // servo untuk mount
}

void readldr()
{
    int rerataAwalLDRTray1 = 0;
    int rerataAwalLDRTray2 = 0;
    int rerataAwalLDRTray3 = 0;

    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        // Serial.print("Iterasi ke-");
        // Serial.println(i + 1);
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

void initPrinter()
{
    mySerial.begin(9600); // Sesuaikan baud rate dengan printer Anda
    printer.begin();      // Memulai printer
}

void print(String bloodType)
{

    delay(2000);

    printer.setSize('M');
    printer.justify('C'); // Menyelaraskan teks ke tengah
    printer.println("KARTU TEST");
    printer.println("GOLONGAN DARAH");

    printer.feed(2); // Beri jarak 2 baris kosong setelah teks

    printer.justify('M'); // Kembali ke penyelarasan kiri
    printer.println("NAMA :");
    printer.println("UMUR :");

    printer.feed(2); // Beri jarak 2 baris kosong setelah teks

    printer.justify('C'); // Menyelaraskan teks ke tengah
    printer.setSize('S');
    printer.println("HASIL");
    printer.setSize('L');
    printer.println(bloodType);

    printer.feed(3); // Beri jarak 2 baris kosong setelah teks
}

void initSound()
{
    mySoftwareSerial.begin(9600);

    if (!myDFPlayer.begin(mySoftwareSerial))
    {
        setError(2);
    }
    else
    {
        unsetError(2);
    }

    Serial.println(F("DFPlayer Mini online."));
    myDFPlayer.play(1); // suara alat siap

    myDFPlayer.volume(30); // Set volume value. From 0 to 30
}

void sound(int numberMusic)
{

    myDFPlayer.play(numberMusic); // a+
    delay(2000);
}

// fungsi pendukung
//-----------------------------------------------------------------------------------------------
typedef struct lpf_config_t
{
    float value;
    float valueNew;
    float valueOld;
    float factor;
} lpf_config_t;

lpf_config_t filterLDR1;
lpf_config_t filterLDR2;
lpf_config_t filterLDR3;
void lpf_init(lpf_config_t *lpf, float filter_factor)
{
    lpf->value = lpf->valueNew = lpf->valueOld = 0.0;
    lpf->factor = filter_factor;
}
float lpf_get_filter(lpf_config_t *lpf, float input)
{
    lpf->value = input;
    lpf->valueNew = ((1.0 - lpf->factor) * lpf->valueOld) + (lpf->factor * lpf->value);
    lpf->valueOld = lpf->valueNew;

    return (lpf->valueNew);
}

void getGoldar()
{
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
        GolonganDarah = "AB+";
        kodeGolonganDarah = 6;
    }
    if (LDR1_terang && LDR2_terang && !LDR3_terang) // 110
    {
        GolonganDarah = "AB-";
        kodeGolonganDarah = 7;
    }
    if (LDR1_terang && !LDR2_terang && LDR3_terang) // 101
    {
        GolonganDarah = "A+";
        kodeGolonganDarah = 2;
    }
    if (LDR1_terang && !LDR2_terang && !LDR3_terang) // 100
    {
        GolonganDarah = "A-";
        kodeGolonganDarah = 3;
    }

    if (!LDR1_terang && LDR2_terang && LDR3_terang) // 011
    {
        GolonganDarah = "B+";
        kodeGolonganDarah = 4;
    }
    if (!LDR1_terang && LDR2_terang && !LDR3_terang) // 010
    {
        GolonganDarah = "B-";
        kodeGolonganDarah = 5;
    }
    if (!LDR1_terang && !LDR2_terang && LDR3_terang) // 001
    {
        GolonganDarah = "O+";
        kodeGolonganDarah = 8;
    }
    if (!LDR1_terang && !LDR2_terang && !LDR3_terang) // 000
    {
        GolonganDarah = "O-";
        kodeGolonganDarah = 9;
    }

    Serial.print("Golongan Darah : ");
    Serial.println(GolonganDarah);
}

void process()
{
    timerServo = millis();
    timerProcess = millis();

    while (processStatus)
    {
        currentTimerProcess = millis();
        currentTimer = millis();
        currentTimerServo = millis();

        if ((currentTimerProcess - timerProcess) > processTime)
        {
            timerProcess = currentTimerProcess;
            processStatus = false;
            stirring = false;
            servoMounting.write(180); // servo umount
            motorMove(0);             // close tray full
            readldr();                // baca ldr
            getGoldar();              // get golongan darah
            delay(200);
            Serial.println("Process done");
        }

        if ((currentTimer - timer) > timerinterval)
        {
            timer = currentTimer;
        }

        if (stirring && (currentTimerServo - timerServo) > timerIntervalServo)
        {
            timerServo = currentTimerServo;
            if (stirringCW)
            {
                servoStirPos += 5;
                if (servoStirPos >= 180)
                {
                    stirringCW = false;
                }
            }
            else
            {
                servoStirPos -= 5;
                if (servoStirPos <= 0)
                {
                    stirringCW = true;
                }
            }
            servoStir.write(servoStirPos);
        }
    }
}

void blinkNotification(int led, int times)
{
    for (int i = 0; i < times; i++)
    {
        digitalWrite(led, HIGH);
        delay(delayblink);
        digitalWrite(led, LOW);
        delay(delayblink);
    }
}

void handleCommand(char command)
{
    switch (command)
    {
    case '1':                           // Jika bt0.val == 1 (bt0 ditekan)
        Serial.println("Start button"); // Debug message
        startStatus = true;
        blinkNotification(greenLed, 1);

        break;
    case '2':                                // Jika bt0.val == 1 (bt0 ditekan)
        Serial.println("maintenance");       // Debug message
        Serial.println("opening tray full"); // Debug message
        motor.stepMotor(12000, true);
        blinkNotification(greenLed, 1);
        break;

    case 'A':                          // Jika bt0.val == 1 (bt0 ditekan)
        Serial.println("open button"); // Debug message
        Serial.println(millis());
        motorMove(1); // open tray full
        blinkNotification(greenLed, 2);
        break;

    case 'B':                           // Jika bt0.val == 0 (bt0 dilepas)
        Serial.println("close button"); // Debug message
        Serial.println(millis());
        motorMove(0); // close tray full
        blinkNotification(greenLed, 2);
        break;

    case 'C':                         // Jika bt1.val == 1 (bt1 ditekan)
        Serial.println("run button"); // Debug message
        if (hasSample)
        {
            Serial.println(millis());
            motorMove(2);            // run tray half
            servoMounting.write(90); // servo mount
            delay(200);
            processStatus = true;
            stirring = true;
            process();

            // baca sample
            blinkNotification(greenLed, 2);
        }
        break;
    case 'D':                          // Jika bt1.val == 0 (bt1 dilepas)
        Serial.println("show button"); // Debug message
        Serial.println(millis());

        if (kodeGolonganDarah != 0)
        {
            myDFPlayer.play(kodeGolonganDarah);
            Serial1.print("t0.txt=\"");
            Serial1.println(GolonganDarah);
            Serial1.print("\"");
            Serial1.write(0xff);
            Serial1.write(0xff);
            Serial1.write(0xff);
            delay(3000);
        }
        break;

    case 'E':                           // Jika bt1.val == 0 (bt1 dilepas)
        Serial.println("print button"); // Debug message
        Serial.println(millis());
        if (kodeGolonganDarah != 0)
        {
            print(GolonganDarah);
            delay(3000);
        }
        break;

    default:
        break;
    }
}

void getReady()
{
    Serial.println("Get Ready!!");

    digitalWrite(redLed, HIGH);
    delay(delayblink);
    digitalWrite(redLed, LOW);
    delay(delayblink);
    digitalWrite(greenLed, HIGH);
    delay(delayblink);
    digitalWrite(greenLed, LOW);
    delay(delayblink);
    digitalWrite(blueLed, HIGH);
    delay(delayblink);
    digitalWrite(blueLed, LOW);
    delay(delayblink);
}

void handleError()
{
    while (errorByte > 0)
    {
        int errorIndicator = hasError();
        currentTimer = millis();
        if ((currentTimer - timer) > 2000)
        {
            timer = currentTimer;
            vavail();
            Serial.print("Error pada posisi: ");
            Serial.println(errorIndicator);

            for (int i = 0; i < errorIndicator; i++)
            {
                digitalWrite(redLed, HIGH);
                delay(delayblink);
                digitalWrite(redLed, LOW);
                delay(delayblink);
            }
        }
    }
}

void monitorSerial()
{
    Serial.println("Running");
}
// fungsi utama
//-----------------------------------------------------------------------------------------------
void setup()
{
    // serial
    Serial.begin(115200);
    Serial1.begin(9600);

    // init component
    init_blink();
    initServo();
    initPrinter();
    initSound();
    lpf_init(&filterLDR1, 1);
    lpf_init(&filterLDR2, 1);
    lpf_init(&filterLDR3, 1);

    // run once
    vavail();
    handleError();
    getReady();
    motorMove(0);

    startStatus = true;
    blinkNotification(greenLed, 1);
    delay(2000);

    // tell the world device ready
    Serial.println("All System Ready");
    printer.setSize('M');
    printer.justify('C'); // Menyelaraskan teks ke tengah
    printer.println("ALAT SIAP!");
    printer.feed(2); // Beri jarak 2 baris kosong setelah teks
}

void loop()
{
    // Baca serial Dari LCD
    if (Serial1.available())
    {
        char command = Serial1.read();
        // Serial.println(command);
        handleCommand(command);
    }
    // Baca serial Dari LCD
    if (Serial.available())
    {
        char command = Serial.read();
        // Serial.println(command);
        handleCommand(command);
    }

    // function to read along
    readldr();

    handleError();
    vavail();
    checkTrayCLose();
    rerataAkhirLDRTray1 = 0;
    rerataAkhirLDRTray2 = 0;
    rerataAkhirLDRTray3 = 0;

    // fuction to run at interval
    currentTimer = millis();
    if ((currentTimer - timer) > timerinterval)
    {
        timer = currentTimer;
        if (startStatus)
        {
            monitorSerial();
        }
        else
        {
            Serial.println("Device not started yet.");
        }
    }
}