/*.$file${.::pelican.ino} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: pelican.qm
* File:  ${.::pelican.ino}
*
* This code has been generated by QM 5.0.0 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*.$endhead${.::pelican.ino} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qpn.h" // QP-nano framework for Arduino

//============================================================================
// events used in this application...
enum PelicanSignals {
    PEDS_WAITING_SIG = Q_USER_SIG, // PEDestrians-waiting button press event
    OFF_SIG,                       // OFF-button press event
    ON_SIG                         // ON-button press event
};

//============================================================================
// declare all AO classes...
/*.$declare${AOs::Pelican} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::Pelican} .........................................................*/
typedef struct Pelican {
/* protected: */
    QActive super;

/* private: */
    uint8_t flashCtr;
} Pelican;

/* protected: */
static QState Pelican_initial(Pelican * const me);
static QState Pelican_operational(Pelican * const me);
static QState Pelican_carsEnabled(Pelican * const me);
static QState Pelican_carsGreen(Pelican * const me);
static QState Pelican_carsGreenNoPed(Pelican * const me);
static QState Pelican_carsGreenInt(Pelican * const me);
static QState Pelican_carsGreenPedWait(Pelican * const me);
static QState Pelican_carsYellow(Pelican * const me);
static QState Pelican_pedsEnabled(Pelican * const me);
static QState Pelican_pedsWalk(Pelican * const me);
static QState Pelican_pedsFlash(Pelican * const me);
static QState Pelican_offline(Pelican * const me);
/*.$enddecl${AOs::Pelican} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
//...

// define all AO instances and event queue buffers for them...
Pelican AO_Pelican;
static QEvt l_pelicanQSto[10]; // Event queue storage for Pelican
//...

//============================================================================
// QF_active[] array defines all active object control blocks ----------------
QActiveCB const Q_ROM QF_active[] = {
    { (QActive *)0,           (QEvt *)0,        0U                      },
    { (QActive *)&AO_Pelican, l_pelicanQSto,    Q_DIM(l_pelicanQSto)    }
};

//============================================================================
// Board Support Package (BSP)
enum BSP_CarsSignal { // street signals ...
    CARS_BLANK, CARS_RED, CARS_YELLOW, CARS_GREEN
};

enum BSP_PedsSignal {
    PEDS_BLANK, PEDS_DONT_WALK, PEDS_WALK
};

// various other constants for the application...
enum {
    BSP_TICKS_PER_SEC   = 100, // number of system clock ticks in one second
    LED_L               = 13,  // the pin number of the on-board LED (L)
    CARS_GREEN_MIN_TOUT = (BSP_TICKS_PER_SEC * 8U),
    CARS_YELLOW_TOUT    = (BSP_TICKS_PER_SEC * 3U),
    PEDS_WALK_TOUT      = (BSP_TICKS_PER_SEC * 3U),
    PEDS_FLASH_TOUT     = (BSP_TICKS_PER_SEC / 5U),
    PEDS_FLASH_NUM      = (5U * 2U),
    OFF_FLASH_TOUT      = (BSP_TICKS_PER_SEC / 2U)
};

//............................................................................
void BSP_signalCars(enum BSP_CarsSignal sig) {
    switch (sig) {
        case CARS_BLANK:
            Serial.println(F("Cars: BLANK"));
            break;
        case CARS_RED:
            Serial.println(F("Cars: RED"));
            break;
        case CARS_YELLOW:
            Serial.println(F("Cars: YELLOW"));
            break;
        case CARS_GREEN:
            Serial.println(F("Cars: GREEN"));
            break;
    }
}
//............................................................................
void BSP_signalPeds(enum BSP_PedsSignal sig) {
    switch (sig) {
        case PEDS_BLANK:
            Serial.println(F("Peds: BLANK"));
            break;
        case PEDS_DONT_WALK:
            Serial.println(F("Peds: DON'T WALK"));
            break;
        case PEDS_WALK:
            Serial.println(F("Peds: WALK"));
            break;
    }
}
//............................................................................
void BSP_showState(char const *state) {
    Serial.print(F("State->"));
    Serial.println(state);
}

//............................................................................
void setup() {
    // initialize the QF-nano framework
    QF_init(Q_DIM(QF_active));

    // initialize all AOs...
    QActive_ctor(&AO_Pelican.super, Q_STATE_CAST(&Pelican_initial));

    // initialize the hardware used in this sketch...
    pinMode(LED_L, OUTPUT); // set the LED-L pin to output

    Serial.begin(115200);   // set the highest stanard baud rate of 115200 bps
    Serial.print(F("Start, QP-nano: "));
    Serial.print(F(QP_VERSION_STR));
    Serial.println("");
}
//............................................................................
void loop() {
    QF_run(); // run the QP-nano application
}

//============================================================================
// interrupts...
ISR(TIMER2_COMPA_vect) {
    QF_tickXISR(0); // process time events for tick rate 0

    if (Serial.available() > 0) {
        switch (Serial.read()) { // read the incoming byte
            case 'p':
            case 'P':
                QACTIVE_POST_ISR((QMActive *)&AO_Pelican, PEDS_WAITING_SIG, 0U);
                break;
            case 'o':
            case 'O':
                QACTIVE_POST_ISR((QMActive *)&AO_Pelican, ON_SIG, 0U);
                break;
            case 'f':
            case 'F':
                QACTIVE_POST_ISR((QMActive *)&AO_Pelican, OFF_SIG, 0U);
                break;
        }
    }
}

//============================================================================
// QP-nano callbacks...
void QF_onStartup(void) {
    // set Timer2 in CTC mode, 1/1024 prescaler, start the timer ticking...
    TCCR2A = (1U << WGM21) | (0U << WGM20);
    TCCR2B = (1U << CS22 ) | (1U << CS21) | (1U << CS20); // 1/2^10
    ASSR  &= ~(1U << AS2);
    TIMSK2 = (1U << OCIE2A); // enable TIMER2 compare Interrupt
    TCNT2  = 0U;

    // set the output-compare register based on the desired tick frequency
    OCR2A  = (F_CPU / BSP_TICKS_PER_SEC / 1024U) - 1U;
}
//............................................................................
void QV_onIdle(void) {   // called with interrupts DISABLED
    // Put the CPU and peripherals to the low-power mode. You might
    // need to customize the clock management for your application,
    // see the datasheet for your particular AVR MCU.
    SMCR = (0 << SM0) | (1 << SE); // idle mode, adjust to your project
    QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
}
//............................................................................
Q_NORETURN Q_onAssert(char const Q_ROM * const module, int location) {
    // implement the error-handling policy for your application!!!
    (void)module;
    (void)location;
    QF_INT_DISABLE(); // disable all interrupts
    QF_RESET();  // reset the CPU
    for (;;) {
    }
}

//============================================================================
// define all AO classes (state machines)...
/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 670U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpn version 6.7.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${AOs::Pelican} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::Pelican} .........................................................*/
/*.${AOs::Pelican::SM} .....................................................*/
static QState Pelican_initial(Pelican * const me) {
    /*.${AOs::Pelican::SM::initial} */
    return Q_TRAN(&Pelican_operational);
}
/*.${AOs::Pelican::SM::operational} ........................................*/
static QState Pelican_operational(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational} */
        case Q_ENTRY_SIG: {
            BSP_signalCars(CARS_RED);
            BSP_signalPeds(PEDS_DONT_WALK);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&Pelican_carsEnabled);
            break;
        }
        /*.${AOs::Pelican::SM::operational::OFF} */
        case OFF_SIG: {
            status_ = Q_TRAN(&Pelican_offline);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::carsEnabled} ...........................*/
static QState Pelican_carsEnabled(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::carsEnabled} */
        case Q_EXIT_SIG: {
            BSP_signalCars(CARS_RED);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&Pelican_carsGreen);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_operational);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen} ................*/
static QState Pelican_carsGreen(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen} */
        case Q_ENTRY_SIG: {
            BSP_signalCars(CARS_GREEN);
            /* one-shot timeout in CARS_GREEN_MIN_TOUT ticks */
            QActive_armX(&me->super, 0U, CARS_GREEN_MIN_TOUT, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen} */
        case Q_EXIT_SIG: {
            QActive_disarmX(&me->super, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&Pelican_carsGreenNoPed);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_carsEnabled);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenNoPed} */
static QState Pelican_carsGreenNoPed(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenNoPed} */
        case Q_ENTRY_SIG: {
            BSP_showState("carsGreenNoPed");
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenNoPed::PEDS_WAITING} */
        case PEDS_WAITING_SIG: {
            status_ = Q_TRAN(&Pelican_carsGreenPedWait);
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenNoPed::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            status_ = Q_TRAN(&Pelican_carsGreenInt);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_carsGreen);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenInt} ..*/
static QState Pelican_carsGreenInt(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenInt} */
        case Q_ENTRY_SIG: {
            BSP_showState("carsGreenInt");
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenInt::PEDS_WAITING} */
        case PEDS_WAITING_SIG: {
            status_ = Q_TRAN(&Pelican_carsYellow);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_carsGreen);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenPedWait} */
static QState Pelican_carsGreenPedWait(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenPedWait} */
        case Q_ENTRY_SIG: {
            BSP_showState("carsGreenPedWait");
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsGreen::carsGreenPedWait::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            status_ = Q_TRAN(&Pelican_carsYellow);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_carsGreen);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::carsEnabled::carsYellow} ...............*/
static QState Pelican_carsYellow(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsYellow} */
        case Q_ENTRY_SIG: {
            BSP_showState("carsYellow");
            BSP_signalCars(CARS_YELLOW);

            /* one-shot timeout in CARS_YELLOW_TOUT ticks */
            QActive_armX(&me->super, 0U, CARS_YELLOW_TOUT, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsYellow} */
        case Q_EXIT_SIG: {
            QActive_disarmX(&me->super, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::carsEnabled::carsYellow::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            status_ = Q_TRAN(&Pelican_pedsEnabled);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_carsEnabled);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::pedsEnabled} ...........................*/
static QState Pelican_pedsEnabled(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::pedsEnabled} */
        case Q_EXIT_SIG: {
            BSP_signalPeds(PEDS_DONT_WALK);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::pedsEnabled::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&Pelican_pedsWalk);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_operational);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::pedsEnabled::pedsWalk} .................*/
static QState Pelican_pedsWalk(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsWalk} */
        case Q_ENTRY_SIG: {
            BSP_showState("pedsWalk");
            BSP_signalPeds(PEDS_WALK);
            /* one-shot timeout in PEDS_WALK_TOUT ticks */
            QActive_armX(&me->super, 0U, PEDS_WALK_TOUT, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsWalk} */
        case Q_EXIT_SIG: {
            QActive_disarmX(&me->super, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsWalk::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            status_ = Q_TRAN(&Pelican_pedsFlash);
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_pedsEnabled);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash} ................*/
static QState Pelican_pedsFlash(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash} */
        case Q_ENTRY_SIG: {
            BSP_showState("pedsFlash");
            /* periodic timeout in PEDS_FLASH_TOUT and every PEDS_FLASH_TOUT ticks */
            QActive_armX(&me->super, 0U, PEDS_FLASH_TOUT, PEDS_FLASH_TOUT);
            me->flashCtr = (PEDS_FLASH_NUM * 2U) + 1U;
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash} */
        case Q_EXIT_SIG: {
            QActive_disarmX(&me->super, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash::Q_TIMEOUT::[me->flashCtr!=0U]} */
            if (me->flashCtr != 0U) {
                --me->flashCtr;
                /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash::Q_TIMEOUT::[me->flashCtr!=0~::[(me->flashCtr&1U)==0U]} */
                if ((me->flashCtr & 1U) == 0U) {
                    BSP_signalPeds(PEDS_DONT_WALK);
                    status_ = Q_HANDLED();
                }
                /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash::Q_TIMEOUT::[me->flashCtr!=0~::[else]} */
                else {
                    BSP_signalPeds(PEDS_BLANK);
                    status_ = Q_HANDLED();
                }
            }
            /*.${AOs::Pelican::SM::operational::pedsEnabled::pedsFlash::Q_TIMEOUT::[else]} */
            else {
                status_ = Q_TRAN(&Pelican_carsEnabled);
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&Pelican_pedsEnabled);
            break;
        }
    }
    return status_;
}
/*.${AOs::Pelican::SM::offline} ............................................*/
static QState Pelican_offline(Pelican * const me) {
    QState status_;
    switch (Q_SIG(me)) {
        /*.${AOs::Pelican::SM::offline} */
        case Q_ENTRY_SIG: {
            BSP_showState("offline");
            /* periodic timeout in OFF_FLASH_TOUT and every OFF_FLASH_TOUT ticks */
            QActive_armX(&me->super, 0U, OFF_FLASH_TOUT, OFF_FLASH_TOUT);
            me->flashCtr = 0U;
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::offline} */
        case Q_EXIT_SIG: {
            QActive_disarmX(&me->super, 0U);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::Pelican::SM::offline::Q_TIMEOUT} */
        case Q_TIMEOUT_SIG: {
            me->flashCtr ^= 1U;
            /*.${AOs::Pelican::SM::offline::Q_TIMEOUT::[(me->flashCtr&1U)==0U]} */
            if ((me->flashCtr & 1U) == 0U) {
                BSP_signalCars(CARS_RED);
                BSP_signalPeds(PEDS_DONT_WALK);
                status_ = Q_HANDLED();
            }
            /*.${AOs::Pelican::SM::offline::Q_TIMEOUT::[else]} */
            else {
                BSP_signalCars(CARS_BLANK);
                BSP_signalPeds(PEDS_BLANK);
                status_ = Q_HANDLED();
            }
            break;
        }
        /*.${AOs::Pelican::SM::offline::ON} */
        case ON_SIG: {
            status_ = Q_TRAN(&Pelican_operational);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*.$enddef${AOs::Pelican} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
//...
