// List Library 
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Ganti sesuai nama wifi/hotspot
const char *ssid = "3344"; // Ganti nama wifi apa
const char *password = "123454321"; // Ganti password wifi apa

// WIRING KABEL
#define DHTPIN 14       // Pin DHT di DS
#define RelayPIN 13     // Pin Relay di 07
#define ButtonIP 15     // Pin Button IP di 08
#define ButtonMode 16   // Pin Button Mode di De

// Pilih Sensor DHT yang digunakan
#define DHTTYPE DHT11   // DHT 11
// #define DHTTYPE DHT22  // DHT 22 (AM2382)
// #define DHTTYPE DHT21  // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// Current temperature & humidity, updated in loop()
float t = 0.0;
float f = 0.0;
float h = 0.0;
float reamur = 0;
float kelvin = 0;
float rankie = 0;  // Corrected 'rankie' to 'rankine'
int mode = 0;
int x = 0;
int y;
int off = 0;
int varA = 0;
int varB = 0;  // Added missing variable

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // Will store last time DHT was updated
unsigned long lastMillis = 0;     // Corrected typo from 'lastmilis'
// Updates DHT readings every 10 seconds
const long interval = 1000;
// put your main code here, to run repeatedly:

bool relayManual = false; // false = auto mode, true = manual mode
bool relayStatus = false; // false = off, true = on

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.css" integrity="sha384-Qboy170N70195" crossorigin="anonymous">
    <style>
        /* Default Light Theme */
        html {
            font-family: Comic Sans MS;
            display: flex;
            flex-direction: column; /* Mengatur arah kolom */
            margin: 0;
            text-align: center;
            background-color: white;
            color: black;
        }

        body.dark-mode {
            background-color: #121212;
            color: white;
        }

        h2 { font-size: 1.5rem; width: 100%; }
        .units { font-size: 0.7rem; }
        .dht-labels {
            font-size: 1.5rem;
            vertical-align: middle;
            padding-bottom: 15px;
            color: black; /* Mengubah warna teks menjadi hitam */
        }
        .box {
            padding: 20px;
            margin: 10px;
            border-radius: 15px;
            color: black; /* Mengubah warna teks dalam box menjadi hitam */
            font-size: 2rem;
            box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.2); /* Tambahkan shadow */
            width: calc(100% - 20px); /* Satu kolom penuh */
        }
        .celcius-box, .fahrenheit-box, .reamur-box, .kelvin-box, .rankie-box {
            background-color: #b0b0b0; /* Warna abu-abu */
        }
        .humidity-box { background-color: #3333ff; }
        .relay-box { background-color: #f0f0f0; }

        /* Button styling */
        .relay-button {
            padding: 10px 20px;
            margin: 5px;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            font-size: 1.2rem;
            transition: background-color 0.3s;
        }

        /* Responsive Design */
        @media (max-width: 600px) {
            .box {
                width: 100%; /* Satu kolom di layar kecil */
            }
        }
    </style>
</head>
<body>
    <h2>ESP8266 DHT Server</h2>

    <div class="box celcius-box">
        <span class="dht-labels">Celcius</span>: 
        <span id="temperature1">%TEMPERATURE1%</span>
        <sup class="units">&deg;C</sup>
    </div>

    <div class="box fahrenheit-box">
        <span class="dht-labels">Fahrenheit</span>: 
        <span id="temperature2">%TEMPERATURE2%</span>
        <sup class="units">&deg;F</sup>
    </div>

    <div class="box reamur-box">
        <span class="dht-labels">Reamur</span>: 
        <span id="reamur">%REAMUR%</span>
        <sup class="units">&deg;Re</sup>
    </div>

    <div class="box kelvin-box">
        <span class="dht-labels">Kelvin</span>: 
        <span id="kelvin">%KELVIN%</span>
        <sup class="units">K</sup>
    </div>

    <div class="box rankie-box">
        <span class="dht-labels">Rankie</span>: 
        <span id="rankie">%RANKIE%</span>
        <sup class="units">&deg;R</sup>
    </div>

    <div class="box humidity-box">
        <i class="fas fa-tint" style="color:#00add6;"></i>
        <span class="dht-labels">Humidity</span>: 
        <span id="humidity">%HUMIDITY%</span>
        <sup class="units">%</sup>
    </div>

    <div class="box relay-box">
        <h3 class="dht-labels">Relay Control</h3>
        <button class="relay-button">Manual</button>
        <button class="relay-button">Otomatis</button>
        <button class="relay-button">On</button>
        <button class="relay-button">Off</button>
    </div>

    <h5>by. Kelompok 3</h5>

    <script>
        function toggleDarkMode() {
            document.body.classList.toggle('dark-mode');
        }

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("temperature1").innerHTML = this.responseText;
                }
            };
            xhttp.open("GET", "/temperature1", true);
            xhttp.send();
        }, 10000);

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("temperature2").innerHTML = this.responseText;
                }
            };
            xhttp.open("GET", "/temperature2", true);
            xhttp.send();
        }, 10000);

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("kelvin").innerHTML = this.responseText;
                }
            };
            xhttp.open("GET", "/kelvin", true);
            xhttp.send();
        }, 10000);

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("reamur").innerHTML = this.responseText;
                }
            };
            xhttp.open("GET", "/reamur", true);
            xhttp.send();
        }, 10000);

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("rankie").innerHTML = this.responseText;
                }
            };
            xhttp.open("GET", "/rankie", true);
            xhttp.send();
        }, 10000);

        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    document.getElementById("humidity").innerHTML = this.responseText;
                }
            };
            xhttp.open("GET", "/humidity", true);
            xhttp.send();
        }, 10000);
    </script>
</body>
</html>)rawliteral";

// Replaces placeholder with values
String processor(const String &var) {
    // Serial.println(var); // Uncomment for debugging
    if (var == "TEMPERATURE1") {
        return String(t);
    } else if (var == "TEMPERATURE2") {
        return String(f);
    } else if (var == "KELVIN") {
        return String(kelvin);
    } else if (var == "REAMUR") {
        return String(reamur);
    } else if (var == "RANKIE") {
        return String(rankie);
    } else if (var == "HUMIDITY") {
        return String(h);
    }
    return String();
}
  
void setup() {
    // Pin Modes
    pinMode(RelayPIN, OUTPUT);
    pinMode(ButtonIP, INPUT);
    pinMode(ButtonMode, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    // Initialize Serial Port
    Serial.begin(115200);

    // Initialize LCD
    lcd.init();
    lcd.backlight();

    // Initialize DHT Sensor
    dht.begin();

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    Serial.println("Connecting to WiFi");
    lcd.setCursor(0, 0);
    lcd.print("Connecting........");
    lcd.setCursor(0, 1);
    lcd.print("......... To WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
        x = digitalRead(ButtonMode);
        y = digitalRead(ButtonIP);
        if (y == HIGH && x == HIGH) {
            break;
        }
    }

    // Print IP Address
    Serial.println();
    Serial.println(WiFi.localIP());

    // Setup for root/webpage
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html, processor);
    });
    server.on("/temperature1", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", String(t).c_str());
    });
    server.on("/temperature2", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", String(t).c_str());
    });
    server.on("/kelvin", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", String(kelvin).c_str());
    });

    server.on("/reamur", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", String(reamur).c_str());
    });

    server.on("/rankie", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", String(rankie).c_str());
    });

    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/plain", String(h).c_str());
    });

    

    // Clear LCD and display IP address
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("IP Server: ");

    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(1000);
    lcd.clear();
    
    // Start server
    server.begin();
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis > interval) {
        // Save the last time you updated the DHT values
        previousMillis = currentMillis;

        // Read temperature as Celsius (the default)
        float newT = dht.readTemperature();
        
        // Read temperature as Fahrenheit (isFahrenheit = true)
        float newF = dht.readTemperature(true);
        
        // If temperature read failed, don't change t value
        if (isnan(newT)) {
            Serial.println("Failed to read from DHT sensor");
           // lcd.setCursor(0, 1);
           // lcd.print("Error read sensor");
        } else {
            t = newT;
            f = newF;
            reamur = t * 0.8;
            kelvin = t + 273.15;
            rankie = t * 1.8 + 491.67;
            Serial.println(t);
            
            if (mode == 0) {
                lcd.setCursor(7, 0);
                lcd.print(t);
                lcd.print("C");
            } else if (mode == 1) {
                lcd.setCursor(7, 0);
                lcd.print(f);
                lcd.print(" F");
            } else if (mode == 2) {
                lcd.setCursor(7, 0);
                lcd.print(reamur);
                lcd.print(" R");
            } else if (mode == 3) {
                lcd.setCursor(7, 0);
                lcd.print(kelvin);
                lcd.print(" K");
            } else if (mode == 4) {
                lcd.setCursor(7, 0);
                lcd.print(rankie);
                lcd.print(" Ra");
            }
        }

        // Read Humidity
        float newH = dht.readHumidity();
        
        // If humidity read failed, don't change h value
        if (isnan(newH)) {
            Serial.println("Failed to read from DHT sensor!");
        } else {
            h = newH;
            Serial.println(h);
            lcd.setCursor(7, 1);
            lcd.print(h);
            lcd.print(" %");
        }
    lcd.setCursor(0, 0);
    lcd.print("Suhu ");
    lcd.setCursor(0, 1);
    lcd.print("Hum ");
    }
    x = digitalRead(ButtonMode);
    y = digitalRead(ButtonIP);
    // Serial.println(x);
    off = 0;
    
    if (y == HIGH && x == HIGH) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Relay :");
        lcd.setCursor(0, 1);
        lcd.print("Manual");
        delay(1000);
        varB = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Relay :");
        lcd.setCursor(0, 1);
        if (t > 29) {
            lcd.print("ON");
        } else {
            lcd.print("OFF");
        }
        while (varB == 1) {
            x = digitalRead(ButtonMode);
            y = digitalRead(ButtonIP);
            if (y == HIGH && x == HIGH) {
                varB = 0;
            } else if (x == HIGH) {
                digitalWrite(RelayPIN, HIGH);
                lcd.setCursor(0, 1);
                lcd.print("ON");
            } else if (y == HIGH) {
                digitalWrite(RelayPIN, LOW);
                lcd.setCursor(0, 1);
                lcd.print("OFF");
            }
            delay(50);
        }
            lcd.setCursor(0, 0);
            lcd.print("Relay :");
            lcd.setCursor(0, 1);
            lcd.print("AUTO");
            delay(1000);
            lcd.clear();
    } else if (y == HIGH) {
    
        if (varA == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Mode: ");
            mode++;

            if (mode == 5) {
              mode = 0;
            }

            lcd.setCursor(0, 1);
            if (mode == 0) {
                lcd.print("Celcius");
            } else if (mode == 1) {
                lcd.print("Fahrenheit");
            } else if (mode == 2) {
                lcd.print("Reamur");
            } else if (mode == 3) {
                lcd.print("Kelvin");
            } else if (mode == 4) {
                lcd.print("Rankie");
            }
                delay(500);
                lcd.clear();
                varA = 1;
                lastMillis = millis();
            } else if (varA == 1 && millis() - lastMillis > 3000) {
                lcd.clear();
                lcd.print("Restart.....");
                delay(500);
                ESP.restart();
            }
          } 
          else if (x == HIGH) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("IP Server: ");
            lcd.setCursor(0, 1);
            lcd.print(WiFi.localIP());
            delay(1000);
            lcd.clear();
           } 
            else if (y == LOW) {
                varA = 0;
            }
                // Mengendalikan relay
                if (t > 29) {
                    digitalWrite(RelayPIN, HIGH);
                    // Serial.println("Hidup");
                } else {
                    digitalWrite(RelayPIN, LOW);
                  //  Serial.println("Mati");
                }
            }