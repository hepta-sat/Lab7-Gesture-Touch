#include "mbed.h"

// LPC1768: Serial(TX, RX)
Serial sensor(p9, p10);
Serial pc(USBTX, USBRX);

static const char* gesture_to_string(uint8_t code)
{
    switch (code) {
        // Gestures
        case 0x01: return "GESTURE: RIGHT";
        case 0x02: return "GESTURE: LEFT";
        case 0x03: return "GESTURE: BACK";
        case 0x04: return "GESTURE: FORWARD";
        case 0x05: return "GESTURE: PULL UP";
        case 0x06: return "GESTURE: PULL DOWN";
        case 0x07: return "GESTURE: PULL & REMOVE";

        // Touch keys (5 keys)
        case 0x21: return "TOUCH: 1";
        case 0x22: return "TOUCH: 2";
        case 0x23: return "TOUCH: 3";
        case 0x24: return "TOUCH: 4";
        case 0x25: return "TOUCH: 5";

        default:   return 0;
    }
}

int main()
{
    pc.baud(115200);

    // SEN0285 wiki: UART 9600 8N1
    sensor.baud(9600);

    pc.printf("SEN0285 Gesture & Touch (mbed OS 2, LPC1768)\r\n");
    pc.printf("Expect frame: AA <code> <chk=FF-code> 55\r\n");

    enum { WAIT_AA, READ_CODE, READ_CHK, WAIT_55 } st = WAIT_AA;
    uint8_t code = 0, chk = 0;

    while (1) {
        while (sensor.readable()) {
            uint8_t b = (uint8_t)(sensor.getc() & 0xFF);

            switch (st) {
                case WAIT_AA:
                    if (b == 0xAA) st = READ_CODE;
                    break;

                case READ_CODE:
                    code = b;
                    st = READ_CHK;
                    break;

                case READ_CHK:
                    chk = b;
                    st = WAIT_55;
                    break;

                case WAIT_55:
                    if (b == 0x55) {
                        // checksum check
                        if ((uint8_t)(0xFF - code) == chk) {
                            const char* msg = gesture_to_string(code);
                            if (msg) {
                                pc.printf("%s (0x%02X)\r\n", msg, code);
                            } else {
                                pc.printf("UNKNOWN CODE: 0x%02X\r\n", code);
                            }
                        } else {
                            pc.printf("BAD FRAME: code=0x%02X chk=0x%02X\r\n", code, chk);
                        }
                    }
                    // resync for next frame (even if footer was not 0x55)
                    st = WAIT_AA;
                    break;
            }
        }
        wait_ms(5);
    }
}
