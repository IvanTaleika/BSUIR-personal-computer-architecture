#include <dos.h>
//Project works only on DOS system. Tested on dosBox-0.74.
enum CONTROLLER {
  MASTER,
  SLAVE
};

enum REGISTER {
  MASK,
  REQUEST,
  SERVICE
};

struct VIDEO {
  unsigned char symb;
  unsigned char attr;
};

#define FIRST_COLOR 0x12 //blue background and green font
#define LAST_COLOR 0x1f //blue background and bright white font
#define REGISTER_SIZE 8
int symbolAttribute = FIRST_COLOR;

void getRegistersState();           //get data from registres
void print(char state, CONTROLLER controller, REGISTER registerName);    //fill in the screen
void changeSybmolAttribute(); //change font color

//IRQ0-7 - master
void interrupt(*int08)(...);
void interrupt(*int09)(...);
void interrupt(*int0A)(...);
void interrupt(*int0B)(...);
void interrupt(*int0C)(...);
void interrupt(*int0D)(...);
void interrupt(*int0E)(...);
void interrupt(*int0F)(...);

//IRQ8-15 - slave
void interrupt(*int70)(...);
void interrupt(*int71)(...);
void interrupt(*int72)(...);
void interrupt(*int73)(...);
void interrupt(*int74)(...);
void interrupt(*int75)(...);
void interrupt(*int76)(...);
void interrupt(*int77)(...);

// new interrupt handlers
void interrupt new08(...) { getRegistersState(); int08(); }
void interrupt new09(...) {changeSybmolAttribute(); getRegistersState(); int09(); }
void interrupt new0A(...) { getRegistersState(); int0A(); }
void interrupt new0B(...) { getRegistersState(); int0B(); }
void interrupt new0C(...) { getRegistersState(); int0C(); }
void interrupt new0D(...) { getRegistersState(); int0D(); }
void interrupt new0E(...) { getRegistersState(); int0E(); }
void interrupt new0F(...) { getRegistersState(); int0F(); }

void interrupt new88(...) { getRegistersState(); int70(); }
void interrupt new89(...) { getRegistersState(); int71(); }
void interrupt new8A(...) { getRegistersState(); int72(); }
void interrupt new8B(...) { getRegistersState(); int73(); }
void interrupt new8C(...) { getRegistersState(); int74(); }
void interrupt new8D(...) { getRegistersState(); int75(); }
void interrupt new8E(...) { getRegistersState(); int76(); }
void interrupt new8F(...) { getRegistersState(); int77(); }

void changeSybmolAttribute() {
  if (symbolAttribute == LAST_COLOR) {
	symbolAttribute = FIRST_COLOR;
  } else {
	symbolAttribute++;
  }
}

void print(char state, CONTROLLER controller, REGISTER registerName) {
 // unsigned place = controller * 160 + registerName * 2 * REGISTER_SIZE + registerName; //Formula gets input place
  VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);
  screen += (controller * 80 + registerName * REGISTER_SIZE + registerName);
  int i;
  for (i = 7; i >= 0; i--) { //find bits
	screen->symb = state % 2 + '0'; //if last bit of state == 1, state % 2 == 1
	screen->attr = symbolAttribute;
	screen++;
	state /= 2;
  }
}

// get data from registers
void getRegistersState() {
  print(inp(0x21), MASTER, MASK); //read master IMR

  outp(0x20, 0x0A); //OCW3 - read master IRR
  print(inp(0x20), MASTER, REQUEST); //Result of OCW3

  outp(0x20, 0x0B); //OCW3 - read master ISR
  print(inp(0x20), MASTER, SERVICE); //Result of OCW3

  print(inp(0xA1), SLAVE, MASK); //read slave IMR

  outp(0x20, 0x0A); //OCW3 - read slave IRR
  print(inp(0xA0), SLAVE, REQUEST); //Result of OCW3

  outp(0x20, 0x0B); //OCW3 - read slave ISR
  print(inp(0xA0), SLAVE, SERVICE); //Result of OCW3
}

void init() {
  //Getting master handler pointers
  int08 = getvect(0x08); // Timer
  int09 = getvect(0x09); // Keyboard
  int0A = getvect(0x0A); // For slave controller
  int0B = getvect(0x0B); // COM2 interrupt
  int0C = getvect(0x0C); // COM1 interrupt
  int0D = getvect(0x0D); // Hard disk interrupt for XT
  int0E = getvect(0x0E); // Floppy interrupt
  int0F = getvect(0x0F); // Printer interrupt

  //Getting slave handler pointers
  int70 = getvect(0x70); // Clock
  int71 = getvect(0x71); // EGA controller
  int72 = getvect(0x72); // Reserved
  int73 = getvect(0x73); // Reserved
  int74 = getvect(0x74); // Reserved
  int75 = getvect(0x75); // Math soprocessor
  int76 = getvect(0x76); // Hard disk interrupt
  int77 = getvect(0x77); // Reserved

  //Setting new master handlers
  setvect(0x08, new08);
  setvect(0x09, new09);
  setvect(0x0A, new0A);
  setvect(0x0B, new0B);
  setvect(0x0C, new0C);
  setvect(0x0D, new0D);
  setvect(0x0E, new0E);
  setvect(0x0F, new0F);

  //88h-8fh are used by BASIC
  //Setting new slave handlers
  setvect(0x88, new88);
  setvect(0x89, new89);
  setvect(0x8A, new8A);
  setvect(0x8B, new8B);
  setvect(0x8C, new8C);
  setvect(0x8D, new8D);
  setvect(0x8E, new8E);
  setvect(0x8F, new8F);

  _disable(); // disable interrupts handling (cli)

  // Master initialization
  outp(0x20, 0x11); //ICW1
  outp(0x21, 0x08); //ICW2
  outp(0x21, 0x04); //ICW3
  outp(0x21, 0x01); //ICW4

  // Slave initialization
  outp(0xA0, 0x11); //ICW1
  outp(0xA1, 0x88); //ICW2
  outp(0xA1, 0x02); //ICW3
  outp(0xA1, 0x01); //ICW4

  _enable(); // enable interrupt handling (sti)
}

int main() {
  unsigned far* fp;
  init();

  FP_SEG(fp) = _psp;  // segment
  FP_OFF(fp) = 0x2c;  // offset
  _dos_freemem(*fp);

  _dos_keep(0, (_DS - _CS) + (_SP / 16) + 1); //Terminate and stay resident program (TSR)
  return 0;
}
