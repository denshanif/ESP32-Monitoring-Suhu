#include <LiquidCrystal_I2C.h>
#include <Adafruit_AHT10.h>

int lcdColumns = 16;
int lcdRows = 2;

Adafruit_AHT10 aht10;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

byte derajat_simbol = B11011111;

void setup()
{
  Serial.begin(9600);
  aht10.begin();
  lcd.init();
  lcd.backlight();
}

void loop()
{
  sensors_event_t humidity, temp;
  aht10.getEvent(&humidity, &temp);

  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.setCursor(5,0);
  lcd.print(temp.temperature);
  lcd.setCursor(9,0);
  lcd.write(derajat_simbol);
  lcd.setCursor(10,0);
  lcd.print("C");
  lcd.setCursor(12,0);
  lcd.print("   ");
  
  lcd.setCursor(0,1);
  lcd.print("Humd:");
  lcd.setCursor(5,1);
  lcd.print(humidity.relative_humidity);
  lcd.setCursor(9,1);
  lcd.print("% rH");

  delay(500);
}
