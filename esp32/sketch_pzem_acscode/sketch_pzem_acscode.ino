#include <WiFi.h>
#include <HTTPClient.h>
#include <PZEM004Tv30.h>

// =====================================================
// WIFI CREDENTIALS
// =====================================================

const char* WIFI_SSID     = "OnePlus 13R 3A92";
const char* WIFI_PASSWORD = "varam123";

// =====================================================
// FLASK / RENDER API
// =====================================================

const char* API_URL =
  "https://energy-changeover-dashboard.onrender.com/api/data";

// =====================================================
// PIN DEFINITIONS
// =====================================================

// PZEM-004T v3
#define PZEM_RX 16
#define PZEM_TX 17

// ACS712
#define ACS_PIN 34

// SSR / CHANGEOVER CONTROL
#define SSR_PIN 25

// =====================================================
// PZEM
// =====================================================

HardwareSerial pzemSerial(2);

PZEM004Tv30 pzem(
  pzemSerial,
  PZEM_RX,
  PZEM_TX
);

// =====================================================
// CHANGEOVER SETTINGS
// =====================================================

// Low-voltage threshold
const float LOW_VOLTAGE = 210.0;

// Recovery voltage
const float RECOVERY_VOLTAGE = 220.0;

// Voltage must remain low this long before changeover
const unsigned long LOW_VOLTAGE_DELAY = 3000;

// Voltage must remain recovered this long before returning
const unsigned long RECOVERY_DELAY = 10000;

// =====================================================
// ACS712 SETTINGS
// =====================================================

// ESP32 ADC
const float ADC_REFERENCE = 3.3;
const int ADC_MAX = 4095;

// Change this according to your ACS712 model:
// 5A  = 0.185
// 20A = 0.100
// 30A = 0.066
const float ACS_SENSITIVITY = 0.066;

// Automatically measured zero-current voltage
float acsZeroVoltage = 0.0;

// =====================================================
// VARIABLES
// =====================================================

bool changeoverActive = false;

unsigned long lowVoltageStart = 0;
unsigned long recoveryStart = 0;

unsigned long lastUpload = 0;
const unsigned long UPLOAD_INTERVAL = 5000;

// =====================================================
// ACS712 ZERO CALIBRATION
// =====================================================

void calibrateACS712()
{
  Serial.println();
  Serial.println("--------------------------------");
  Serial.println("ACS712 CALIBRATION");
  Serial.println("KEEP LOAD CURRENT OFF");
  Serial.println("--------------------------------");

  delay(2000);

  const int samples = 1000;

  long totalADC = 0;

  for (int i = 0; i < samples; i++)
  {
    totalADC += analogRead(ACS_PIN);
    delayMicroseconds(200);
  }

  float averageADC =
    (float)totalADC / samples;

  acsZeroVoltage =
    averageADC * ADC_REFERENCE / ADC_MAX;

  Serial.print("ACS712 Zero Voltage: ");
  Serial.print(acsZeroVoltage, 4);
  Serial.println(" V");

  Serial.println("--------------------------------");
}

// =====================================================
// READ ACS712 CURRENT
// =====================================================

float readACS712Current()
{
  const int samples = 500;

  float totalVoltage = 0.0;

  for (int i = 0; i < samples; i++)
  {
    int adcValue = analogRead(ACS_PIN);

    float voltage =
      (adcValue * ADC_REFERENCE) / ADC_MAX;

    totalVoltage += voltage;

    delayMicroseconds(200);
  }

  float averageVoltage =
    totalVoltage / samples;

  float current =
    (averageVoltage - acsZeroVoltage)
    / ACS_SENSITIVITY;

  // Small noise correction
  if (fabs(current) < 0.05)
  {
    current = 0.0;
  }

  // Prevent tiny negative no-load readings
  if (current < 0.0 && fabs(current) < 0.30)
  {
    current = 0.0;
  }

  return current;
}

// =====================================================
// WIFI
// =====================================================

void connectWiFi()
{
  Serial.println();
  Serial.println("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int count = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);

    Serial.print(".");

    count++;

    if (count >= 40)
    {
      Serial.println();
      Serial.println("WiFi connection failed.");
      return;
    }
  }

  Serial.println();
  Serial.println("WiFi CONNECTED");

  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());
}

// =====================================================
// CHANGEOVER CONTROL
// =====================================================

void controlChangeover(float voltage)
{
  // ===================================================
  // NORMAL → LOW VOLTAGE
  // ===================================================

  if (!changeoverActive)
  {
    recoveryStart = 0;

    if (voltage > 0 && voltage < LOW_VOLTAGE)
    {
      if (lowVoltageStart == 0)
      {
        lowVoltageStart = millis();

        Serial.println();
        Serial.println("LOW VOLTAGE DETECTED");
        Serial.println("Starting confirmation timer...");
      }

      if (millis() - lowVoltageStart >= LOW_VOLTAGE_DELAY)
      {
        changeoverActive = true;

        digitalWrite(SSR_PIN, HIGH);

        Serial.println();
        Serial.println("==============================");
        Serial.println("CHANGEOVER ON");
        Serial.println("SSR STATUS : ON");
        Serial.println("==============================");
      }
    }
    else
    {
      lowVoltageStart = 0;
    }
  }

  // ===================================================
  // CHANGEOVER → VOLTAGE RECOVERY
  // ===================================================

  else
  {
    lowVoltageStart = 0;

    if (voltage >= RECOVERY_VOLTAGE)
    {
      if (recoveryStart == 0)
      {
        recoveryStart = millis();

        Serial.println();
        Serial.println("VOLTAGE RECOVERED");
        Serial.println("Starting recovery timer...");
      }

      if (millis() - recoveryStart >= RECOVERY_DELAY)
      {
        changeoverActive = false;

        digitalWrite(SSR_PIN, LOW);

        Serial.println();
        Serial.println("==============================");
        Serial.println("CHANGEOVER OFF");
        Serial.println("SSR STATUS : OFF");
        Serial.println("==============================");
      }
    }
    else
    {
      recoveryStart = 0;
    }
  }
}

// =====================================================
// SEND DATA TO FLASK
// =====================================================

void sendDataToFlask(
  float voltage,
  float pzemCurrent,
  float acsCurrent,
  float power,
  float energy,
  float frequency,
  float powerFactor
)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected.");
    return;
  }

  HTTPClient http;

  http.begin(API_URL);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  String systemStatus;

  if (changeoverActive)
  {
    systemStatus = "CHANGEOVER ACTIVE";
  }
  else
  {
    systemStatus = "NORMAL";
  }

  String json = "{";

  json += "\"voltage\":" +
          String(voltage, 2) + ",";

  json += "\"pzem_current\":" +
          String(pzemCurrent, 3) + ",";

  json += "\"acs712_current\":" +
          String(acsCurrent, 3) + ",";

  json += "\"power\":" +
          String(power, 2) + ",";

  json += "\"energy\":" +
          String(energy, 3) + ",";

  json += "\"frequency\":" +
          String(frequency, 2) + ",";

  json += "\"power_factor\":" +
          String(powerFactor, 2) + ",";

  json += "\"ssr\":" +
          String(changeoverActive ? "true" : "false") + ",";

  json += "\"system_status\":\"" +
          systemStatus + "\"";

  json += "}";

  int httpCode = http.POST(json);

  Serial.print("Flask HTTP: ");
  Serial.println(httpCode);

  if (httpCode > 0)
  {
    String response = http.getString();

    Serial.print("Flask response: ");
    Serial.println(response);
  }
  else
  {
    Serial.println("Flask upload failed.");
  }

  http.end();
}

// =====================================================
// SERIAL DISPLAY
// =====================================================

void printData(
  float voltage,
  float pzemCurrent,
  float acsCurrent,
  float power,
  float energy,
  float frequency,
  float powerFactor
)
{
  Serial.println();
  Serial.println("--------------------------------");

  Serial.print("Voltage          : ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  Serial.print("PZEM Current     : ");
  Serial.print(pzemCurrent, 3);
  Serial.println(" A");

  Serial.print("ACS712 Current   : ");
  Serial.print(acsCurrent, 3);
  Serial.println(" A");

  Serial.print("Power            : ");
  Serial.print(power, 2);
  Serial.println(" W");

  Serial.print("Energy           : ");
  Serial.print(energy, 3);
  Serial.println(" kWh");

  Serial.print("Frequency        : ");
  Serial.print(frequency, 2);
  Serial.println(" Hz");

  Serial.print("Power Factor     : ");
  Serial.println(powerFactor, 2);

  Serial.print("SSR STATUS       : ");

  if (changeoverActive)
    Serial.println("ON");
  else
    Serial.println("OFF");

  Serial.print("SYSTEM STATUS    : ");

  if (changeoverActive)
    Serial.println("CHANGEOVER ACTIVE");
  else
    Serial.println("NORMAL");

  Serial.println("--------------------------------");
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("======================================");
  Serial.println(" ENERGY CHANGEOVER PROJECT");
  Serial.println(" ESP32 + PZEM + ACS712 + SSR");
  Serial.println("======================================");

  // SSR initially OFF
  pinMode(SSR_PIN, OUTPUT);
  digitalWrite(SSR_PIN, LOW);

  // ACS712 ADC
  pinMode(ACS_PIN, INPUT);

  analogReadResolution(12);

  // PZEM
  pzemSerial.begin(
    9600,
    SERIAL_8N1,
    PZEM_RX,
    PZEM_TX
  );

  // ACS calibration
  calibrateACS712();

  // WiFi
  connectWiFi();

  Serial.println();
  Serial.println("SYSTEM READY");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
  // ---------------------------------------------------
  // READ PZEM
  // ---------------------------------------------------

  float voltage = pzem.voltage();

  float pzemCurrent = pzem.current();

  float power = pzem.power();

  float energy = pzem.energy();

  float frequency = pzem.frequency();

  float powerFactor = pzem.pf();

  // ---------------------------------------------------
  // CHECK INVALID PZEM VALUES
  // ---------------------------------------------------

  if (isnan(voltage))
    voltage = 0;

  if (isnan(pzemCurrent))
    pzemCurrent = 0;

  if (isnan(power))
    power = 0;

  if (isnan(energy))
    energy = 0;

  if (isnan(frequency))
    frequency = 0;

  if (isnan(powerFactor))
    powerFactor = 0;

  // ---------------------------------------------------
  // ACS712
  // ---------------------------------------------------

  float acsCurrent = readACS712Current();

  // ---------------------------------------------------
  // CHANGEOVER
  // ---------------------------------------------------

  controlChangeover(voltage);

  // ---------------------------------------------------
  // DISPLAY
  // ---------------------------------------------------

  printData(
    voltage,
    pzemCurrent,
    acsCurrent,
    power,
    energy,
    frequency,
    powerFactor
  );

  // ---------------------------------------------------
  // SEND TO FLASK
  // ---------------------------------------------------

  if (millis() - lastUpload >= UPLOAD_INTERVAL)
  {
    lastUpload = millis();

    sendDataToFlask(
      voltage,
      pzemCurrent,
      acsCurrent,
      power,
      energy,
      frequency,
      powerFactor
    );
  }

  delay(1000);
}
