#include <stdio.h>
#include <conio.h>
#include <dos.h>
//Project works only on DOS system. Tested on dosBox-0.74.
#define BASE_FREQUENCY 1193180
#define NUMBER_OF_NOTES 9
#define PORT_0 0x40
#define PORT_1 0x41
#define PORT_2 0x42
#define COMMAND_REGISTER 0x43
#define INPUT_REGISTER 0x61
#define BYTE 8
#define BYTE_VALUE 256

void playSound(void);
void showDivisionCoefficients(void);
void showStatusWords(void);
void randomNumber(void);

void main() {
  clrscr();
  do {
    printf("1 - Play sound;\n"
           "2 - Get division coefficients;\n"
           "3 - Get status words;\n"
           "4 - Get random number;\n"
           "0 - Exit.\n");
    fflush(stdin);
    switch (getch()) {
      case '1':
        playSound();
        clrscr();
        break;
      case '2':
        clrscr();
        showDivisionCoefficients();
        break;
      case '3':
        clrscr();
        showStatusWords();
        break;
      case '4':
        clrscr();
        randomNumber();
        break;
      case '0':
        clrscr();
        return;
      default:
        clrscr();
    }
  } while (1);
}


void playSound() {
  int frequency[NUMBER_OF_NOTES] = {293, 392, 293, 392, 523, 659, 880, 784, 698};
  int longevity[NUMBER_OF_NOTES] = {200, 400, 200, 400, 200, 200, 400, 200, 200};
  int convertedFrequency;
  outp(INPUT_REGISTER, inp(INPUT_REGISTER) | 3);  //turn ON
  for (int i = 0; i < NUMBER_OF_NOTES; i++) {
    outp(COMMAND_REGISTER, 0xB6); //10110110. Where 0 - binary, 011 - 3d mode, 11 - r/w low byte, then high byte, 10 - 2 chanel
    convertedFrequency = BASE_FREQUENCY / frequency[i];
    outp(PORT_2, convertedFrequency % BYTE_VALUE); //Low
    convertedFrequency /= BYTE_VALUE;
    outp(PORT_2, convertedFrequency); //High

    delay(longevity[i]);
  }
  outp(INPUT_REGISTER, inp(INPUT_REGISTER) & 0xFC); //turn OFF (1111 1000)
}

void showDivisionCoefficients(void) {
  int ports[] = {PORT_0, PORT_1, PORT_2};
  int controlWords[] = {0, 64, 128};
  unsigned  divisionCoefficient, max;

  printf("Division coefficients:\n");

  for (int iChannel = 0; iChannel < 3; iChannel++) {
    divisionCoefficient = 0;
    max = 0;
	if (iChannel == 2)
	  outp(INPUT_REGISTER, inp(INPUT_REGISTER) | 1); //turn ON for kd2

    for (unsigned long j = 0; j < 65000; j++) {
      outp(COMMAND_REGISTER, controlWords[iChannel]); //select a channel
      divisionCoefficient  = inp(ports[iChannel]);
      divisionCoefficient = (inp(ports[iChannel]) << BYTE)
                            + divisionCoefficient;

      if (divisionCoefficient > max)
	max = divisionCoefficient;
    }

	if (iChannel == 2)
	  outp(INPUT_REGISTER, inp(INPUT_REGISTER) & 0xFC); ////turn OFF for kd2
    printf("Channel %d - %4X\n", iChannel, max);
  }
}

void showStatusWords(void) {
  unsigned char state, stateWord[9];
  int ports[] = {PORT_0, PORT_1, PORT_2};
  //11100010: 0 - always 0, 001 - first port, 0 - read state, 1 - don't read CE, 11 - read command. And so on.
  int controlWords[] = {226, 228, 232};
  stateWord[8] = '\0';
  for (int iChannel = 0; iChannel < 3; iChannel++) {
    outp(COMMAND_REGISTER, controlWords[iChannel]); //select a channel
    state = inp(ports[iChannel]);
    for (int i = 7; i >= 0; i --) {
      stateWord[i] = state % 2 + '0';
      state /= 2;
    }
    printf("Channel %d: %s \n", iChannel, stateWord);
  }
}


void randomNumber(void) {
  static unsigned int limit = 0;
  unsigned int number;
  clrscr();
  outp(INPUT_REGISTER, inp(INPUT_REGISTER) | 1);  //turn ON without sound
  do {
    printf("1 - set random limit;\n"
           "2 - get a number;\n"
           "0 - exit.\n");
    fflush(stdin);
    switch (getch()) {
      case '1':
        do {
          printf("Enter number limit: from 1 to 65535\n");
          fflush(stdin);
        } while (!scanf("%u", &limit));
        //10110110: 0 - binary, 011 - 3d mode, 11 - r/w low and high bytes, 10 - 2nd port.
        outp(COMMAND_REGISTER, 0xB6);
        outp(PORT_2, limit % BYTE_VALUE); //Low
        limit /= BYTE_VALUE;
        outp(PORT_2, limit); //High
        break;

      case '2':
        printf("Random number:\n");
        outp(COMMAND_REGISTER, 0x86); //Read from 2nd channel, 3d mode
        number = inp(PORT_2);
        number = (inp(PORT_2) << BYTE) + number;
        printf("Number %u\n\n", number);
        break;
      case '0':
        outp(INPUT_REGISTER, inp(INPUT_REGISTER) & 0xFC); //turn OFF. Equal to 0b11111100
        clrscr();
        return;
    }
  } while (1);
}
