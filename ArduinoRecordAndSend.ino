#include <SPI.h>
SPISettings settings(3500000, MSBFIRST, SPI_MODE0); 
  
byte bx0,bx00,bx1,bx11,bx2,bx22,bx3,bx33;
unsigned int iter=0;

void setup() {
  delay(4500);
  Serial.begin(2000000);
  pinMode (10, OUTPUT);  
  PORTB = PORTB | B00000100;
  SPI.begin();
  SPI.beginTransaction(settings);
  cli();                            //stop interrupts

//Wyzeruj bajt TCCR2A.
  TCCR2A = 0;
//Wyzeruj bajt TCCR2B.
  TCCR2B = 0;
//Zresetuj wartość licznika.
  TCNT2  = 0;
//Ustaw bit WGM21, co spowoduje pracę licznika w trybie CTC (WGM22=0, WGM21=1, WGM20=0).
  TCCR2A |= (1 << WGM21);
//Ustaw wartości bitów CS22, CS21 i CS20 odpowiadające wyborowi preskalera 8.
  TCCR2B = (0 << CS22) | (1 << CS21) |(0 << CS20);   
//Ustaw maksymalną wartość licznika na 199, dla której licznik przepełnia się z częstotliwością 10 kHz (OCR2A = 16 MHz / (10 kHz * 8) - 1).
  OCR2A =199;// = (16*10^6) / (8000*8) - 1 (must be <256); 199 dla 10kHz, 159 dla 12.5kHz, 249 dla 8kHz
//Aktywuj przerwanie kiedy aktualna wartość licznika będzie równa wartości rejestru OCR2A (TCNT2 = OCR2A).
  TIMSK2 |= (1 << OCIE2A);

  sei();//allow interrupts
}



ISR(TIMER2_COMPA_vect){
  PORTB = PORTB & B11111011; //Ustaw pin 10 (linia SS) w stan niski.
  SPI.transfer(0x01); //Wyślij do MCP3008 bajt startu – 00000001.
/*Wyślij do MCP3008 bajt określający typ odczytu i wybrany numer kanału 
(SGL/DIFF|D2|D1|D0|0|0|0|0), który dla odczytu pojedynczego i kanału pierwszego 
(do którego podłączony jest mikrofon referencyjny 0) jest równy 10000000 (dla kanału 
drugiego bajt ten byłby równy 10010000, dla trzeciego 10100000, a dla czwartego 
10110000). Jednocześnie zapisz w zmiennej bx0 2 najmniej znaczące bity z bajtu
odesłanego przez MCP3008, bx0=(0|0|0|0|0|0|B9|B8). */
  bx0 = SPI.transfer(0x80)&0x3; 
/*Wyślij bajt 0000 0000 do MCP3008. Jednocześnie zapisz w zmiennej bx00 bajt odesłany 
przez MCP3008, bx00=(B7|B6|B5|B4|B3|B2|B1|B0). Amplituda sygnału z mikrofonu 
zakodowana jest w słowie 10-bitowym (B9|B8|B7|B6|B5|B4|B3|B2|B1|B0). */
  bx00= SPI.transfer(0x0); 
//Ustaw pin 10 (linia SS) w stan wysoki. Odczyt kanału został zakończony.
  PORTB = PORTB | B00000100;
//Analogicznie odczytaj słowo z kanału drugiego i zapisz je w bajtach bx1 i bx11.
  PORTB = PORTB & B11111011;
  SPI.transfer (0x01);
  bx1 = SPI.transfer (0x90)&0x3;
  bx11= SPI.transfer (0x0);
  PORTB = PORTB | B00000100;
//Analogicznie odczytaj słowo z kanału trzeciego i zapisz je w bajtach bx2 i bx22.
  PORTB = PORTB & B11111011;
  SPI.transfer (0x01);
  bx2 = SPI.transfer (0xA0)&0x3;
  bx22= SPI.transfer (0x0);
  PORTB = PORTB | B00000100;
//Analogicznie odczytaj słowo z kanału czwartego i zapisz je w bajtach bx3 i bx33.
  PORTB = PORTB & B11111011;
  SPI.transfer (0x01);
  bx3 = SPI.transfer (0xB0)&0x3;
  bx33= SPI.transfer (0x0);
  PORTB = PORTB | B00000100;
  
//Rozpocznij komunikację UART z prędkością 2 Mb/s.
/*Wyślij poprzez port szeregowy bajt bx00 zawierający 8 najmniej znaczących bitów 
słowa określającego napięcie z mikrofonu referencyjnego 0. */
  Serial.write(bx00);
  Serial.write(bx11); //Analogicznie wyślij bajt bx11 (mikrofon 1).
  Serial.write(bx22); // Analogicznie wyślij bajt bx22 (mikrofon 2).
  Serial.write(bx33); // Analogicznie wyślij bajt bx33 (mikrofon 3).
/*2 najbardziej znaczące bity słów dla każdego kanału zostają połączone w jeden bajt
(B9|B8 z CH3, B9|B8 z CH2, B9|B8 z CH1, B9|B8 z CH0) oraz wysłane poprzez port 
szeregowy. */
  Serial.write((bx3<<6) | (bx2<<4) | (bx1<<2) | (bx0));
  
  iter++;
}


void loop() {
  if (iter<10000) {
    PORTB = PORTB | B00000100;//Disable SPI
  }
  else if (iter==10000){
    cli();//stop interrupts
    iter++;
  }
  else {}
}
