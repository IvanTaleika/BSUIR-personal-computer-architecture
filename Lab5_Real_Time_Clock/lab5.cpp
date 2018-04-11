#include <stdio.h>
#include <conio.h>
#include <dos.h>
//Project works only on DOS system. Tested on dosBox-0.74.

#define CHOICE_PORT 0x70
#define RW_PORT 0x71

struct ClockRegisters {
  char seconds;
  char alarmSeconds;
  char minuts;
  char alarmMinuts;
  char hours;
  char alarmHours;
  char weekDay;
  char monthDay;
  char month;
  char year;
  char stateA;
  char stateB;


  ClockRegisters() {
	seconds = 0x0;
	alarmSeconds = 0x1;
	minuts = 0x2;
	alarmMinuts = 0x3;
	hours = 0x4;
	alarmHours = 0x5;
	weekDay = 0x6;
	monthDay = 0x7;
	month = 0x8;
	year = 0x9;
	stateA = 0xA;
	stateB = 0xB;
  }
} registersAddresses;

struct Time {
  char seconds;
  char minuts;
  char hours;
  char weekDay;
  char monthDay;
  char month;
  char year;

  void toDec() {
	seconds = (seconds >> 4) * 10 + (seconds & 0xF);
	minuts = (minuts >> 4) * 10 + (minuts & 0xF);
	hours = (hours >> 4) * 10 + (hours & 0xF);
	monthDay = (monthDay >> 4) * 10 + (monthDay & 0xF);
	month = (month >> 4) * 10 + (month & 0xF);
	year = (year >> 4) * 10 + (year & 0xF);
  }

  void timeToBCD() {
	seconds = ((seconds / 10) << 4) + (seconds % 10);
	minuts = ((minuts / 10) << 4) + (minuts % 10);
	hours = ((hours / 10) << 4) + (hours % 10);
  }

  void dateToBCD() {
	monthDay = ((monthDay / 10) << 4) + (monthDay % 10);
	month = ((month / 10) << 4) + (month % 10);
	year = ((year / 10) << 4) + (year % 10);
  }
};

char* dayToCStr[] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

long long int programDelay;

void interrupt(*old70)(...);
void interrupt new70(...) {
	programDelay--;
	old70();
}

Time getTime();
Time inputTime();
Time inputDate();
void setTime(Time time);
void setDate(Time time);
void printTime(Time time);
void setprogramDelay();

void main() {
  do {
	clrscr();
	printTime(getTime());
	printf("1 - Set time;\n"
		   "2 - Set date;\n"
		   "3 - Set delay;\n"
		   "0 - Exit.\n");
	fflush(stdin);
	switch (getch()) {
	  case '1':
		setTime(inputTime());
		break;
	  case '2':
		setDate(inputDate());
		break;
	  case '3':
		printf("Enter delay (ms):\n");
		scanf("%lld", &programDelay);
		printf("Waiting...");
		setprogramDelay();
		break;
	  case '0':
		return;
	}
  } while (1);
}

Time getTime() {
  Time time;
  outp(CHOICE_PORT, registersAddresses.seconds);
  time.seconds = inp(RW_PORT);
  outp(CHOICE_PORT, registersAddresses.minuts);
  time.minuts = inp(RW_PORT);
  outp(CHOICE_PORT, registersAddresses.hours);
  time.hours = inp(RW_PORT);
  outp(CHOICE_PORT, registersAddresses.weekDay);
  time.weekDay = inp(RW_PORT);
  outp(CHOICE_PORT, registersAddresses.monthDay);
  time.monthDay = inp(RW_PORT);
  outp(CHOICE_PORT, registersAddresses.month);
  time.month = inp(RW_PORT);
  outp(CHOICE_PORT, registersAddresses.year);
  time.year = inp(RW_PORT);
  time.toDec();
  return time;
}

Time inputTime() {
  Time time;
  int value;

  printf("Enter hours:\n");
  scanf("%d", &value);
  time.hours = value;

  printf("Enter minuts:\n");
  scanf("%d", &value);
  time.minuts = value;

  printf("Enter seconds:\n");
  scanf("%d", &value);
  time.seconds = value;

  time.timeToBCD();
  return time;
}

Time inputDate() {
  Time time;
  int value;

  printf("Enter week day (1 - sunday, 7 - saturday):\n");
  scanf("%d", &value);
  time.weekDay = value;

  printf("Enter day:\n");
  scanf("%d", &value);
  time.monthDay = value;

  printf("Enter month:\n");
  scanf("%d", &value);
  time.month = value;

  printf("Enter last 2 numbers of year:\n");
  scanf("%d", &value);
  time.year = value;

  time.dateToBCD();
  return time;
}

void setTime(Time time) {
  int state;
  //Check update flag
  do {
	outp(CHOICE_PORT, registersAddresses.stateA);
	state = inp(RW_PORT);
  } while (state >= 128);

  //Set update flag (u can't read data now)
  outp(CHOICE_PORT, registersAddresses.stateB);
  outp(RW_PORT, inp(RW_PORT) | 128);

  outp(CHOICE_PORT, registersAddresses.seconds);
  outp(RW_PORT, time.seconds);
  outp(CHOICE_PORT, registersAddresses.minuts);
  outp(RW_PORT, time.minuts);
  outp(CHOICE_PORT, registersAddresses.hours);
  outp(RW_PORT, time.hours);

  //Reset update flag
  outp(CHOICE_PORT, registersAddresses.stateB);
  outp(RW_PORT, inp(RW_PORT) & 127);
}

void setDate(Time time) {
  int state;
  //Check update flag
  do {
	outp(CHOICE_PORT, registersAddresses.stateA);
	state = inp(RW_PORT);
  } while (state >= 128);
  //Set update flag (u can't read data now)
  outp(CHOICE_PORT, registersAddresses.stateB);
  outp(RW_PORT, inp(RW_PORT) | 128);

  outp(CHOICE_PORT, registersAddresses.weekDay);
  outp(RW_PORT, time.weekDay);
  outp(CHOICE_PORT, registersAddresses.monthDay);
  outp(RW_PORT, time.monthDay);
  outp(CHOICE_PORT, registersAddresses.month);
  outp(RW_PORT, time.month);
  outp(CHOICE_PORT, registersAddresses.year);
  outp(RW_PORT, time.year);

  //Reset update flag
  outp(CHOICE_PORT, registersAddresses.stateB);
  outp(RW_PORT, inp(RW_PORT) & 127);
}

void printTime(Time time) {
  printf("%d:%d:%d, ", time.hours, time.minuts, time.seconds);
  printf("%s, ", dayToCStr[time.weekDay - 1]);
  printf("%d.%d.%d\n", time.monthDay, time.month, time.year);
}

void setprogramDelay() {
  old70 = getvect(0x70);

  //Set new handler
  _disable();
  setvect(0x70, new70);
  _enable();

  char maskState = inp(0xA1);

  //Enable clock interrupt
  outp(0xA1, maskState & 254);
  //Enable interrupt every 1 ms
  outp(CHOICE_PORT, registersAddresses.stateB);
  outp(RW_PORT, inp(RW_PORT) | 64);

  while (programDelay > 0);

  //Return old state
  outp(CHOICE_PORT, registersAddresses.stateB);
  outp(RW_PORT, inp(RW_PORT) & 191);
  outp(0xA1, maskState);

  //Return old handler
  _disable();
  setvect(0x70, old70);
  _enable();
}
