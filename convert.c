#include <stdio.h>
#include <bcm2835.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
//bajty wysylane do przetwornika
uint8_t start = 0x01;
uint8_t end = 0x00;
uint8_t chan = 0x00;
//wektor z zarejestrowanymi danymi
int data[50000];
int x=0;
FILE *fp;

//cykliczna fkcja do rejestrowania probek
void alarmWakeup(int sig_num);
//fkcja zwracajaca wartosc z danego kanalu ADC
int readADC(uint8_t chan);
//fkcja zamienajaca wartosc 0-1023 z ADC na wolty
float volts_adc(int adc);

//fkcja wysyla i dostaje bajty od ADC, wartosci bajtow zgodne z dokumentacja ADC
int readADC(uint8_t chan){
  char buf[] = {start, (0x08|chan)<<4,end};
  char readBuf[3];
  bcm2835_spi_transfernb(buf,readBuf,3);
  return (int)readBuf[1] & 0x03) << 8 | (int) readBuf[2];
}

//fkcja cykliczna odczytuje wartsci wejsc 0-4 ADC
void alarmWakeup(int sig_num)
{
if(sig_num == SIGALRM)
    {
data[x++]=readADC(0);
data[x++]=readADC(1);
data[x++]=readADC(2);
data[x++]=readADC(3);
    }
}

//float volts_adc(int adc) {
//  return (float)adc*3.3f/1023.0f;
//}

int main(int argc, char const *argv[]) {
//inicjalizacja komunikacji SPI
    if (!bcm2835_init())
    {
      printf("bcm2835_init failed. Are you running as root??\n");
      return 1;
    }

    if (!bcm2835_spi_begin())
    {
      printf("bcm2835_spi_begin failed. Are you running as root??\n");
      return 1;
    }
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
fp = fopen("heh.txt", "w");

//C12500Hz_200Hz_full.txt
//czestotliwosc probkowania i czas
//int freq=16666;
//int duration=1;
//int timestamp=1/freq*1000000;

int dataLength=(int) sizeof(data) / (int) sizeof(data[0]);
struct timeval current_time;
struct timeval current_time1;
printf("Press enter to start sampling.");
getchar();
sleep(3);
printf("CLAP!\n");
signal(SIGALRM, alarmWakeup);   
ualarm(80,80);


gettimeofday(&current_time, NULL);
    while (x<dataLength)
    {
    }
gettimeofday(&current_time1, NULL);

printf("Difference: Seconds : %ld, micro seconds : %ld\n",
    current_time1.tv_sec-current_time.tv_sec, current_time1.tv_usec-current_time.tv_usec);
    printf("Start seconds : %ld, micro seconds : %ld\n",
    current_time.tv_sec, current_time.tv_usec);
        printf("End seconds : %ld, micro seconds : %ld\n",
    current_time1.tv_sec, current_time1.tv_usec);

//for(int j=0;j<1000;j++)
//{
//printf("%i\n",data[j]);
//}

for(int j=0;j<dataLength;j++)
{
fprintf(fp,"%i\n",data[j]);
}

fclose(fp);
    return 0;
}
