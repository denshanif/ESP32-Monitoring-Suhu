#include <Adafruit_AHT10.h>
 
Adafruit_AHT10 aht10;
 
void setup() {
  Serial.begin(9600);
  aht10.begin();
}
 
void loop() {
  sensors_event_t humidity, temp;
  aht10.getEvent(&humidity, &temp);
  if (temp.temperature) {
    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    Serial.println(" *C");
  }

  if (humidity.relative_humidity) {
    Serial.print("Humidity: ");
    Serial.print(humidity.relative_humidity);
    Serial.println(" %");
  }
 
  delay(1000);
}