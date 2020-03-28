//.$file${.::bsp.hpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: dpp_bsp.qm
// File:  ${.::bsp.hpp}
//
// This code has been generated by QM 5.0.0 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::bsp.hpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef BSP_HPP
#define BSP_HPP

class BSP {
public:
    enum { TICKS_PER_SEC = 100} ;
    static void init(void);
    static void displayPaused(uint8_t const paused);
    static void displayPhilStat(uint8_t const n, char_t const *stat);
    static void terminate(int16_t const result);

    static void randomSeed(uint32_t const seed); // random seed
    static uint32_t random(void);                // pseudo-random generator
    static QP::QTimeEvtCtr think_rnd_time();
    static QP::QTimeEvtCtr eat_rnd_time();

    static void ledOff(void);
    static void ledOn(void);
};

#endif // BSP_HPP
