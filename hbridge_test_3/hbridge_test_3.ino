void setup() 
{
  
}

void loop() 
{
  digitalWrite(2, LOW);  
  analogWrite(3, 80);            
  digitalWrite(4, HIGH);
  
  delay(1000);
  
  digitalWrite(4, LOW);  
  analogWrite(3, 80);            
  digitalWrite(2, HIGH);
  
  delay(1000);
}
