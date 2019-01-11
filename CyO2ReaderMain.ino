#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_ADS1015.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

Adafruit_ADS1115 ads(0x48);// I2C Addr is wired at 0x48l

//define running status
bool bLoaded = false;
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
float fInitReading = -1.0;
float fMaxFactor = 4.76;
float fMaxExptectedVal = 0;
String sMainVersion = "v0.1.7beta"; //Versioning set here
String sLogoString = "CY O2 Analyzer";

void setup()
{
  //Setup Oled
  u8g2.begin();
  //Setup ADC
  ads.setGain(GAIN_SIXTEEN); //PGA set here 
  ads.begin();
  ads.startComparator_SingleEnded(0, 1000);
}

void loop()
{
  //Ensure run log and init proce just at boot up 
  if (!bLoaded)
  {
    intProc();      //init every thing let user know the status.
    bLoaded = true; //Set state so won't run again
  }
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_7x13B_tf); // choose a suitable font
  String sVMsg = "Init: " + String(fInitReading) + "mV";
  //u8g2.drawStr(0,10,sVMsg.c_str());
  sVMsg = "Current: " + String(getScenorReading()) + "mV";
  //u8g2.drawStr(0,20,sVMsg.c_str());
  sVMsg = "Max: " + String(fMaxExptectedVal) + "mV";
  //u8g2.drawStr(0,30,sVMsg.c_str());
  //GetCurrent O2%
  float fO2Reading = getMeanReading() * getKVal();
  sVMsg = "O2 " + String(fO2Reading) + "%";
  //Consider if the sensor get value under more than 1 ata. the converted value wloud greater than 100%
  if (fO2Reading > 100)
  {
    sVMsg = "O2 100% -> Max";
  }

  u8g2.drawStr(30, 15, sVMsg.c_str());
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_t0_14_tr);
  sVMsg = "Mod 1.2->" + String((1.2 / (fO2Reading * 0.001)) - 10.0) + "m";
  u8g2.drawStr(8, 28, sVMsg.c_str());
  sVMsg = "Mod 1.4->" + String((1.4 / (fO2Reading * 0.001)) - 10.0) + "m";
  u8g2.drawStr(8, 38, sVMsg.c_str());
  sVMsg = "Mod 1.6->" + String((1.6 / (fO2Reading * 0.001)) - 10.0) + "m";
  u8g2.drawStr(8, 48, sVMsg.c_str());
  u8g2.setFont(u8g2_font_5x7_t_cyrillic);
  u8g2.drawStr(75, 60, sMainVersion.c_str());
  u8g2.sendBuffer();
  delay(2000);
}
//Slope is calculated
float getKVal()
{
  float fKval = 0;
  fKval = (100 - 21) / (fMaxExptectedVal - fInitReading);
  return fKval;
}

float getMeanReading()
{
  float fReadings[5];
  float fAveReading = 0;
  for (int i = 0; i < 5; i++)
  {
    fAveReading += getScenorReading();
    delay(50);
  }
  fAveReading = fAveReading / 5;
  return fAveReading;
}
float getScenorReading()
{
  int adc0 = ads.getLastConversionResults();
  float intData = adc0 * 0.0078125; //Covert PGA ratio
  return intData;
}
void intProc()
{
  fInitReading = getScenorReading();
  u8g2.clearBuffer();                           // clear the internal memory
  u8g2.setFont(u8g2_font_prospero_bold_nbp_tf); // choose a suitable font
  u8g2.drawStr(25, 30, sLogoString.c_str());       // write something to the internal memory
  u8g2.sendBuffer();                            // transfer internal memory to the display
  delay(800);
  u8g2.setFont(u8g2_font_5x7_t_cyrillic);
  u8g2.drawStr(70, 60, "Loading...");
  u8g2.sendBuffer();
  delay(1000);
  fInitReading = getScenorReading();
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_5x7_t_cyrillic); // choose a suitable font
  String sVMsg = "statablizing sensor /";
  fInitReading = getScenorReading();
  delay(800);
  u8g2.clearBuffer();
  u8g2.drawStr(5, 22, sVMsg.c_str());
  u8g2.sendBuffer();
  sVMsg = "statablizing sensor | ";
  fInitReading = getScenorReading();
  delay(800);
  u8g2.clearBuffer();
  u8g2.drawStr(5, 22, sVMsg.c_str());
  u8g2.sendBuffer();
  delay(800);
  sVMsg = "statablizing sensor \\";
  fInitReading = getScenorReading();
  u8g2.clearBuffer();
  u8g2.drawStr(5, 22, sVMsg.c_str());
  u8g2.sendBuffer();
  delay(800);
  u8g2.clearBuffer();
  sVMsg = "statablizing sensor -";
  fInitReading = getScenorReading();
  u8g2.clearBuffer();
  u8g2.drawStr(5, 22, sVMsg.c_str());
  u8g2.sendBuffer();
  fInitReading = getMeanReading();
  delay(250);
  sVMsg = "Baseline: " + String(fInitReading) + "mV";
  u8g2.drawStr(5, 60, sVMsg.c_str());
  u8g2.sendBuffer();
  fMaxExptectedVal = fInitReading * fMaxFactor;
  delay(1000);
}
