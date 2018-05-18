/*
 * Serial.h
 *
 *  Created on: May 17, 2018
 *      Author: student
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdbool.h>

/*
	struct termios: bemeneti/kimeneti beállítások nagyrészt
	tcflag_t c_iflag (input flagek)
	tcflag_t c_oflag (output flagek)
	tcflag_t c_cflag (control flagek)
	tcflag_t c_lflag (local flagek)
	cc_t c_cc[NCCS] (speciális karakterek) ?
	ezerféle flag van amivel be lehet állítani a dolgokat
	ezeket tcsetattr() és tcgetattr()-el lehet beállítani és kiolvasni


		dmesg | grep tty utasítást a terminálba írva megkapjuk, hogy mik vannak csatlakoztatva
		az stm a ttyACM0 ( putty-val kipróbálva )

 */



bool Init_SerialPort();

#endif /* SERIAL_H_ */

