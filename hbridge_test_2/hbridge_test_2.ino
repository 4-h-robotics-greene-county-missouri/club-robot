void setup() 
{
  
}

void loop() 
{
  digitalWrite(2, HIGH);  
  digitalWrite(3, HIGH);            
  digitalWrite(4, LOW);
  
  delay(1000)
  
  digitalWrite(2, LOW);  
  digitalWrite(3, HIGH);            
  digitalWrite(4, HIGH);
  
  delay(1000);
}
