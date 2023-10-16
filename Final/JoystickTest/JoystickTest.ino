int joyX = A0;
int joyY = A1;

int xValue = 0;
int yValue = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  xValue = analogRead(joyX);
  yValue = analogRead(joyY);

  Serial.println("The x value is = ");
  Serial.println(xValue);
  Serial.println("The y value is = ");
  Serial.println(yValue);
  delay(200);
}