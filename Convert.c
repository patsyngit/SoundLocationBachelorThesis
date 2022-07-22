#include <stdio.h>
#include <bcm2835.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
//Bajty wysylane do przetwornika
uint8_t start = 0x01;         //Bajt startu 0000 0001.
//Bajt określający kanał, którego napięcie ma zostać odczytane.
uint8_t chan = 0x00;
uint8_t end = 0x00;          //Trzeci bajt 0000 0000
int data[50000];            //Tablica przechowująca zarejestrowane dane.
int dataRead=0;             //Liczba dokonanych pomiarów napięcia sygnałów.
FILE *fp;                   //handler pliku z danymi

//cykliczna fkcja do rejestrowania probek
void readInterrupt (int sig_num);
//fkcja zwracajaca wartosc z danego kanalu ADC
int readChannel(uint8_t channel) {
//fkcja zamienajaca wartosc 0-1023 z ADC na wolty
float volts_adc(int adc);

/*Funkcja readChannel wywoływana jest z argumentem channel i zwraca słowo, 
w którym zakodowana jest wartość napięcia sygnału z danego kanału MCP3008. 
Dla wartości argumentu channel z przedziału 0-7 odczytane zostaną kanały CH0-CH7 
przetwornika. 
*/
int readChannel(uint8_t channel) {
/*Stwórz bufor writeBuf z bajtami, które mają zostać wysłane do MCP3008. Wynikiem 
operacji bitowej (0x08|channel) dla argumentu channel przyjmującego wartości 
z przedziału 0-3 są bajty: (10000000) umożliwiający odczyt pierwszego kanału 
przetwornika, (10010000) umożliwiający odczyt drugiego kanału przetwornika,
(10100000) umożliwiający odczyt trzeciego kanału przetwornika, (10110000) 
umożliwiający odczyt czwartego kanału przetwornika. */
  char writeBuf[] = {start, (0x08|channel)<<4,end};
/*Stwórz pusty bufor trzyelementowy, w którym zapisane zostaną 3 bajty wysłane 
w odpowiedzi przez MCP3008. */
  char readBuf[3];
/*Wyślij do MCP3008 bajty z buforu writeBuf i jednocześnie odbierz od MCP3008 3 bajty 
i wpisz je do buforu readBuf. */
  bcm2835_spi_transfernb(writeBuf,readBuf,3);
/*Bajt readBuf[2] składa się z 8 mniej znaczących bitów słowa 10-bitowego 
(B7|B6|B5|B4|B3|B2|B1|B0). Dwa ostatnie bity bajtu readBuf[2] są dwoma najbardziej 
znaczącymi bitami słowa 10-bitowego (B9|B8). Dokonując operacji bitowych na obu 
bajtach i otrzymane zostanie słowo (B9|B8|B7|B6|B5|B4|B3|B2|B1| B0), w którym 
zakodowana jest wartość napięcia z danego kanału przetwornika. Słowo to jest zwrócone
przez funkcję. */
  return ((int)readBuf[1] & 0x03) << 8 | (int) readBuf[2];
}

/*Funkcja readInterrupt wywoływana w ramach obsłużenia przerwania od sygnału 
SIGALRM rejestruje słowa odpowiadające napięciom sygnałów z wejść przetwornika
- sygnałów z mikrofonów. */
void readInterrupt (int sig_num) {
//Sprawdzenie czy funkcja została wywołana przez sygnał SIGALRM.
  if(sig_num == SIGALRM) {
//Wpisanie słów z odczytu sygnałów z mikrofonów jako kolejnych elementów tablicy data.
    data[dataRead ++]=readChannel(0);
    data[dataRead ++]=readChannel (1);
    data[dataRead ++]=readChannel (2);
    data[dataRead ++]=readChannel (3);
  }
}
  
//fkcja zamienajaca wartosc 0-1023 z ADC na wolty
float volts_adc(int adc) {
  return (float)adc*3.3f/1023.0f;
}

int main(int argc, char const *argv[]) {
/*Zainicjalizuj działanie biblioteki bcm2835.h i zapewnij jej dostęp do pamięci fizycznej, 
w tym do pamięci wykorzystywanej przy komunikacji SPI i portów GPIO. */
    if (!bcm2835_init()){
      printf("bcm2835_init failed. Are you running as root??\n");
      return 1;
    }
/*Zainicjalizuj komunikację SPI konfigurując odpowiednie piny GPIO do pracy jako linie
MOSI, MISO, SCLK i SS. */
    if (!bcm2835_spi_begin()){
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return 1;
    }
bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      //Ustaw przesyłanie bajtów w kolejności od najbardziej znaczącego bitu
bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   ////Ustaw tryb pracy SPI 0
/*Ustal sygnał SCLK jako sygnał taktujący procesora z preskalerem o wartości 64. 
Dla takiej konfiguracji sygnał SCLK ma częstotliwość 3.9 MHz dla Rasbperry Pi 2. */
bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); 
/*Skonfiguruj pin GPIO 8 do pracy jako SS. Komunikacja z MCP3008 będzie odbywać się 
dla SS w stanie niskim. */
bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      
bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      
//Otwórz plik w trybie pisania
fp = fopen("heh.txt", "w");
//Definicja struktur przechowujących czas trwania pomiaru
int dataLength=(int) sizeof(data) / (int) sizeof(data[0]);
struct timeval current_time;
struct timeval current_time1;
//Interakcja z użytkownikiem, oczekiwanie na input
printf("Press enter to start sampling.");
getchar();
sleep(3);
printf("CLAP!\n");
/*Ustal funkcję readInterrupt jako funkcję obsługującą przerwanie pochodzące 
od odebrania sygnału SIGALRM. */
signal(SIGALRM, readInterrupt);
/*Wyślij sygnał SIGALRM po upływie 80 mikrosekund oraz cyklicznie powtarzaj wysyłanie 
sygnału co 80 mikrosekund. */
ualarm(80,80);

//Zapisz czas rozpoczęcia pomiaru
gettimeofday(&current_time, NULL);
//Pomiar trwa dopóki tablica data nie zostanie zapełniona
while (x<dataLength)
    {
    }
//Zapisz czas zakończenia pomiaru
gettimeofday(&current_time1, NULL);
//Wyświelt czas trwania pomiaru
printf("Difference: Seconds : %ld, micro seconds : %ld\n",current_time1.tv_sec-current_time.tv_sec, current_time1.tv_usec-current_time.tv_usec);
printf("Start seconds : %ld, micro seconds : %ld\n",current_time.tv_sec, current_time.tv_usec);
printf("End seconds : %ld, micro seconds : %ld\n",current_time1.tv_sec, current_time1.tv_usec);

//Zapisz pomiar do pliku
for(int j=0;j<dataLength;j++){
  fprintf(fp,"%i\n",data[j]);
}

fclose(fp);
return 0;
}
