// Declaration and initialization of the input pin
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#define PIN 12
boolean statuslamp;
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);
const int aP = 2;
const char* ssid = "123";
const char* password = "12345670";
AsyncWebServer server(80);


void setup() {
  strip.begin();
  strip.setBrightness(40); //adjust brightness here
  strip.show(); // Initialize all pixels to 'off'
  pinMode(aP,INPUT); // к аналоговому входу подключим датчик (англ. «intput»)
  statuslamp=false; // начальное состояние - лампа выключена
  
  Serial.begin(115200); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi подключено");

   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Данные звука: " + String(analogRead(aP)));
  });

  // Запуск веб-сервера
  server.begin();
}

// Fill the dots one after the other with a color

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}
// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

void loop() {
  // Some example procedures showing how to display to the pixels:
  //Serial.println(analogRead(aP));
  if(analogRead(aP)>500) {
      unsigned long currentTime = millis();
      server.on("/sound_event", HTTP_GET, [currentTime](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Нарушение тишины! Время: " + String(currentTime));
      });
      statuslamp=!statuslamp; // меняем статус лампы при регистрации хлопка
      digitalWrite(PIN,statuslamp); // переключаем светодиод на выходе 12
      if(analogRead(aP)<520) {
        colorWipe(strip.Color(255, 255, 255), 100); // Red
        colorWipe(strip.Color(0, 0, 255), 100); // Green
        colorWipe(strip.Color(255, 0, 0), 100); // Blue
      }
      
      else if(analogRead(aP)>520) {
        rainbow(6);
        rainbowCycle(6);
      }
      //delay(20);
      delay(1500);
      colorWipe(strip.Color(0, 0, 0), 30);
  }

  delay(500);
}
