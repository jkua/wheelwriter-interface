# Wheelwriter 3 Hardware

                           ┌─────────────────┐         ┌────────────┐   ┌──────────────┐
                           │                 │         │            │   │              │
                           │                 │         │  Platen    │   │              │
                           │    Typehead     │         │  Position  │   │    Power     │
                           │    Carrier      │◄────┐   │  Stepper   │   │    Supply    │
                           │                 │     │   │  Motor     │   │    Board     │
                           │                 │     │   │            │   │              │
                           │                 │     │   └─────────▲──┘   │              │
                           └─────────────────┘     │             │      └──────┬───────┘
                                                   │             │             │
                                                   │             │             │
                                                   │             │             │
                           ┌──────────────────┐    │       ┌─────┴─────────────▼───────┐         ┌───────────────┐
    ┌──────────────┐       │                  │    │       │    J3P           J6P      │         │               │
    │              │       │                  │    └───────┤J4P                        │         │               │
    │              ├──────►│J1F               │            │         Motor             │         │               │
    │   Keyboard   │       │      Logic       │            │         Control           │         │   Option      │
    │              │       │      Board    J8F├───────────►│J5P      Board          J1P│◄────────┤   Connector   │
    │              ├──────►│J2F               │            │                           │         │               │
    └──────────────┘       │                  │            │                           │         │               │
                           │   J5F            │            │          J2P              │         │               │
                           └────┬─────────────┘            └───────────┬───────────────┘         └───────────────┘
    ┌──────────────┐            │                                      │
    │              │            │                                      │
    │ Panel Lights │◄───────────┘                                ┌─────▼──────┐
    │              │                                             │            │
    └──────────────┘                                             │  Carrier   │
                                                                 │  Position  │
                                                                 │  Stepper   │
                                                                 │  Motor     │
                                                                 │            │
                                                                 └────────────┘

## Logic board (1356782-05)
This is powered by an Intel P8051AH operating at 11.9 MHz. This interfaces with 
the keyboard via a pair of flex cables, the panel lights, and the motor control 
board. This is where the central processing occurs, accepting input from the 
keyboard and outputting commands to the motor control board via serial bus.

Logic board to Motor control board (J8F, 7-pin AMP cable, black)
1. VRM - Voltage regulator module? 5.1VDC
2. PDI - ?
3. BRQ - Bus request?
4. BUS - Serial communication bus (187500 bps, 9-N-1, 5V, idle high)
5. GND - Ground
6. POR - Power on reset?
7. +5  - 5VDC

Logic board to Panel lights (J5F, 10-pin AMP cable, black)
1. +5
2. LTB
3. LTC
4. LTD
5. LTE
6. LTF
7. LTG
8. LTH
9. LTI
10. LTJ

Keyboard to Logic board (J1F, 13 pin flex)
Keyboard to Logic board (J2F, 15 pin flex, 9 pins not connected)

## Motor control board (FRONT BRD - 1356776-09, attached )
This is powered by an Intel P8051AH operating at 12 MHz. This receives commands 
from the logic and option boards to drive the typehead carrier, the carrier 
position stepper motor, and the platen position stepper motor. It also 
distributes power from the power supply board.

Motor control board to Option (J1P, 10-pin)
1. +5  - +5VDC
2. +5  - +5VDC
3. GND - Ground
4. GND - Ground
5. BUS - Serial communication bus (187500 bps, 9-N-1, 5V, idle high)
6. *NC* - Not connected 
7. BRQ - Bus request?
8. POR - Power on reset?
9. VRM - Voltage regulator module? 
10. PDI - ?

Motor control board to Carrier position stepper motor (J2P, 6-pin AMP, black)
1. EA+
2. EC
3. EB+
4. EA
5. EC+
6. EB

Motor control board to Platen position stepper motor (J3P, 6-pin AMP, multicolor)
1. ID
2. IB
3. IC
4. IA
5. IBD
6. IAC

Motor control board to Typehead carrier (J4P, 22-pin (11x2))
1. HMR
2. HMR
3. SA+
4. SC
5. SB+
6. SA
7. SC+
8. SB
9. LEDG
10. LEDA
11. GND
12. LA
13. LC
14. LB
15. LD
16. FA
17. FC
18. FB
19. FD
20. SENG
21. SENS
22. SE+5

Motor control board to Logic board (J5P, 9-pin, AMP)
1. +34
2. GND
3. VRM
4. PDI
5. BRQ
6. BUS
7. GND
8. FPOR
9. +5

Motor control board to Power supply (J6P, 8-pin)
1. VRM
2. GND
3. -5
4. GND
5. +5
6. +15
7. GND
8. +34

