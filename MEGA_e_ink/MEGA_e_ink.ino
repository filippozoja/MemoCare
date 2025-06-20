#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <SPI.h>

#define CS   10
#define DC    9
#define RST   8
#define BUSY  7

GxEPD2_BW<GxEPD2_154_D67,200> display(GxEPD2_154_D67(CS,DC,RST,BUSY));

const int16_t M = 10; // margine
int16_t winX, winY;   // origine finestra
uint16_t winW, winH;  // dimensioni finestra

String lastDisplayed = "";

void setup() {
  SPI.begin();
  display.init();
  display.setRotation(0);
  display.setFont(&FreeMonoBold24pt7b);
  display.setTextSize(3);
  display.setTextColor(GxEPD_BLACK);

  Serial.begin(115200);
  Serial1.begin(115200);
  while(!Serial);
  Serial.println("Mega pronto!");

  // --- calcolo finestra statica su "24" ---
  int16_t x1,y1; uint16_t w,h;
  display.getTextBounds("24", 0, 0, &x1, &y1, &w, &h);
  winW = w + 2*M;
  winH = h + 2*M;
  winX = (display.width()  - winW) / 2;
  winY = (display.height() - winH) / 2;

  showText("Pronto");
}

void showText(const String& txt) {
  // ricalcolo soltanto per centrare il testo
  int16_t x1,y1; uint16_t w,h;
  display.getTextBounds(txt.c_str(), 0, 0, &x1, &y1, &w, &h);

  int16_t x = winX + (winW - w)/2 - x1;
  int16_t y = winY + (winH + h)/2;

  display.setPartialWindow(winX, winY, winW, winH);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(txt);
  } while (display.nextPage());
}

void loop() {
  if (Serial1.available()) {
    String s = Serial1.readStringUntil('\n');
    s.trim();
    if (s.length() && s != lastDisplayed) {
      showText(s);
      lastDisplayed = s;
      Serial.println("Mostrato: " + s);
    }
  }
}