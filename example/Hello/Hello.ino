#include <FutabaVFD.h>

FutabaVFD vfd(5, 6, 7, 8); // CS, DIN, CLK, RST

void setup() {
    vfd.begin();
    vfd.setBrightness(180);
    vfd.showString(0, "HELLO");
}

void loop() {}