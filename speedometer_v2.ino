//#include <GyverTimer.h>

#include <Adafruit_BMP280.h>

#include <EncButton.h>

#include <Wire.h>

#include <LiquidCrystal_I2C.h>
#include "Forecaster.h"
Forecaster cond;
//GTimer stopTimer(MS);
Adafruit_BMP280 bmp280;
EncButton < EB_TICK, 5 > enc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
float curcle = 2 * 3.14159 * 0.33; // длина окружности в метрах
bool flag_race_time = 0;
byte stop_timer = 0;
float pressure_array[6];
float change_pressure[5];
unsigned long last_time, stopTime, forecastUpdate = 0;
volatile float speed, dist = 0;
class Time {
  public:
    byte hour, min, sec;
    byte Save;
    bool flagClear,flagSaveTime;
  unsigned long timer_delay;
  Time(byte hourRace = 0, byte minRace = 0, byte secRace = 0) {
    hour = hourRace;
    min = minRace;
    sec = secRace;
    timer_delay = 0;
    flagClear =0 ;
    flagSaveTime = 0;
  }
  byte getHour() {
    hour = ((millis() - timer_delay) / 3600000) % 24;
    return hour;
  }
  byte getMin() {
    min = ((millis() - timer_delay) / 60000) % 60;
    return min;
  }
  byte getSec() {
    sec = ((millis() - timer_delay) / 1000) % 60;
    return sec;
  }

  void timerRace() {
    if (flag_race_time == 0) {
      timer_delay = millis();
    }

    if (stop_timer >= 30) {
      speed = 0;  
    }

    if (stop_timer >= 150) {
      flag_race_time = 0;
      if(flagClear ==0){
        lcd.clear();
        flagClear = flagClear+1;
      } 
    }
    else{
      flagClear = 0;
    
    }
  }
};

class SpeedValues{
  public:
    float av_speed=0;
    float str_speed [6];
    unsigned long speedAvDelay=0;

    void setAvSpeedMassive(){
      for (byte i = 0; i < 5; i++) {                  
      str_speed[i] = 0;
      }
    }
    void AvSpeed(){
      if(millis()- speedAvDelay >= 10000)
      {
        speedAvDelay = millis();
        for (byte i = 0; i < 5; i++) {                   // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
        str_speed[i] = str_speed[i + 1];     // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ ЯЧЕЙКИ на шаг назад
        }
        str_speed[5] = speed;
        for(int i=0; i<5 ;i++){
        av_speed = av_speed + str_speed[i];
        }
        av_speed = av_speed /5;
      }
    }
    float getAvSpeed(){
      return av_speed;
    }
};
class Forecast{
  public:
  byte forecastValue =0;
  unsigned long timer_press=0;
  float av_change_pressure =0.0;
  void forecast(){
    cond.addPmm(bmp280.readPressure(),bmp280.readTemperature());
    forecastValue = cond.getCast();
  }
  void avChange(){
    if(millis() - timer_press >= 300000){
    timer_press = millis();
    //Serial.println("pressure_array[i]");
    // Добавление нового давления
    for (byte i=0; i<5;i++){
      pressure_array[i] = pressure_array[i+1];
    }
    pressure_array[5] = bmp280.readPressure()* 0.0075;
    // Изменение давления
    av_change_pressure=0.0;
    //Serial.println("change_pressure[i]");
    for (byte i = 0; i<5; i++){
      change_pressure[i]= pressure_array[i] - pressure_array[i+1];
    //Serial.println(change_pressure[i]);
    }
    //Serial.println("av_change_pressure job");
    for(byte i = 0; i<5; i++){
      av_change_pressure = av_change_pressure + change_pressure[i];
      //Serial.println(av_change_pressure );
    }
    //Serial.println("av_change_pressure total" );
    //Serial.println(av_change_pressure );
  }
  }
  byte getForecast(){
    return forecastValue;
  }
  float getAvChange(){
    return av_change_pressure;
  }
};

Forecast castMatch;
Time raceTime(0, 0, 0);
SpeedValues avspeed;

class Display {
  public:
  byte selectDisp;
  unsigned long dispUpdate=0;
  bool light_lcd = 1;
  byte getForcast=0;
  byte timer = 0;
  Display() {
    selectDisp = 0;
  }

  void speedDisplay() {
    if(millis() - dispUpdate >= 500){
      dispUpdate = millis();
      lcd.setCursor(0, 0);
      lcd.print("Speed(km):");
      lcd.setCursor(10, 0);
      lcd.print(speed, 4);
      lcd.setCursor(0, 1);
      lcd.print("Dist(km):");
      lcd.setCursor(9, 1);
      lcd.print(dist, 3);
    }
  }
  void timeDisplay() {
    if(millis() - dispUpdate >= 500){
      dispUpdate = millis();
      lcd.setCursor(0, 0);
      lcd.print("Time:");
      lcd.setCursor(5, 0);
      lcd.print("H");
      lcd.setCursor(6, 0);
      lcd.print(raceTime.getHour());
      lcd.setCursor(8, 0);
      lcd.print("M");
      lcd.setCursor(9, 0);
      lcd.print(raceTime.getMin());
      lcd.setCursor(11, 0);
      lcd.print("S");
      if((float)raceTime.getSec() /10 <=0.9)
      {
        lcd.setCursor(12, 0);
        lcd.print("0");
        lcd.setCursor(13, 0);
        lcd.print(raceTime.getSec());
      }
      else{
        lcd.setCursor(12, 0);
        lcd.print(raceTime.getSec());
      }
      lcd.setCursor(0, 1);
      lcd.print("Av speed:");
      lcd.setCursor(9, 1);
      lcd.print(avspeed.getAvSpeed(),2);
    }
  }
  void bmpDisplay(){
    if(millis() - dispUpdate >= 500){
      dispUpdate = millis();
      timer ++;
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.setCursor(5, 0);
      lcd.print(bmp280.readTemperature(), 2);
      lcd.setCursor(0, 1);
      lcd.print("Pres:");
      lcd.setCursor(5, 1);
      lcd.print(bmp280.readPressure()*0.0075, 1);
      if (timer >= 21){
        timer=0;
        lcd.clear();
      }
      
    }
  }
  
  void forecastDisp(){
    if(millis() - dispUpdate >= 500){
      dispUpdate = millis();
      timer ++;
      /*if (timer == 10){
        lcd.clear();
      }*/
      lcd.setCursor(0, 0);
      lcd.print("Forecast:");
      lcd.setCursor(9, 0);
      lcd.print(castMatch.getForecast());
      /*switch (castMatch.getForecast()){
        case 0:
        lcd.setCursor(11, 0);
        lcd.print("Sun"); 
        case 1:
        lcd.setCursor(11, 0);
        lcd.print("Sun"); 
        case 2:
        lcd.setCursor(11, 0);
        lcd.print("Sun"); 
        case 3:
        lcd.setCursor(11, 0);
        lcd.print("Sun"); 
        case 4:
        lcd.setCursor(11, 0);
        lcd.print("Cloud"); 
        case 5:
        lcd.setCursor(11, 0);
        lcd.print("Cloud");
        case 6:
        lcd.setCursor(11, 0);
        lcd.print("Cloud");
        case 7:
        lcd.setCursor(11, 0);
        lcd.print("Rain");
        case 8:
        lcd.setCursor(11, 0);
        lcd.print("Rain");
        case 9:
        lcd.setCursor(11, 0);
        lcd.print("Rain");
        case 10:
        lcd.setCursor(11, 0);
        lcd.print("Storm");

      }*/
      lcd.setCursor(0, 1);
      lcd.print("Trand:");
      lcd.setCursor(6, 1);
      if (castMatch.getAvChange() < 0.0){
        lcd.print("Rise");
      }
      if(castMatch.getAvChange() > 0.0){
        lcd.print("Down");
      }
      if (castMatch.getAvChange() == 0.0){
        lcd.print("Stay");
      }
      lcd.setCursor(11, 1);
      lcd.print(castMatch.getAvChange()* (-1));
      if (timer >= 21){
        timer=0;
        lcd.clear();
      }
      
    }
  }
  void changeDisplay() {
    if (enc.click()) {
      lcd.clear();
      selectDisp++;
      //Serial.println(selectDisp);
      if (selectDisp >= 3) {
        selectDisp = 0;
      }
    }
    if(selectDisp ==0){
      speedDisplay();
    }
    if(selectDisp ==1){
      timeDisplay();
    }
    if(selectDisp ==2){
      if (timer <=10 ){
        bmpDisplay();
        //Serial.println(timer);
      }
      if (timer >= 11 ){
        forecastDisp();
        //Serial.println(timer);
      }
    }
  }
  void lightDisp() {
    if (enc.held(1)) {
     if (light_lcd == 1) {
    light_lcd = 0;
  } else {
    light_lcd = 1;
  }
  }
  if (light_lcd == 1) {
    lcd.backlight();
  }
  if (light_lcd == 0) {
    lcd.noBacklight();
  }
  }
  
};

Display disp;

void setup() {
  lcd.init();
  lcd.backlight();
  enc.setHoldTimeout(500);
  enc.setButtonLevel(LOW);
  avspeed.setAvSpeedMassive();
  cond.setMonth(0);
  attachInterrupt(1, sens, RISING);
  pinMode(3, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println(F("BMP280"));
  while (!bmp280.begin(BMP280_ADDRESS - 1)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL, // Режим работы
    Adafruit_BMP280::SAMPLING_X16, // Точность изм. температуры
    Adafruit_BMP280::SAMPLING_X16, // Точность изм. давления
    Adafruit_BMP280::FILTER_X16, // Уровень фильтрации
    Adafruit_BMP280::STANDBY_MS_1000); // Период просыпания, мСек
  for (byte i = 0; i <= 6; i++) {
    pressure_array[i] = bmp280.readPressure() * 0.0075; // забить весь массив текущим давлением
  }
}

void sens() {
  flag_race_time = 1;
  stop_timer = 0;
  speed = curcle / ((float)(millis() - last_time) / 1000) * 3.6; //формула = длина окружности / на период
  //делим на тысячу так как переводим в секунды. получаем м/с. Умнож на 3.6, чтобы получить км/ч
  last_time = millis();
  dist = dist + curcle / 1000;
  Serial.println("speed");
  Serial.println(speed);
  Serial.println("dist");
  Serial.println(dist, 3);
}
void loop() {
  enc.tick();
   if ((millis() - stopTime) >= 1000) {
    stopTime = millis();
    if (stop_timer < 150) {
      stop_timer++;
    }
  }
  raceTime.timerRace();
  disp.lightDisp();
  avspeed.AvSpeed();
  disp.changeDisplay();
  castMatch.avChange();
  if (millis() - forecastUpdate >=300000){
    forecastUpdate= millis();
    castMatch.forecast();
  }
}
