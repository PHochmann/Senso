# Senso

Senso is the German name for the popular game "Simon" in which you have to memorize a sequence of LED colors.
The repository is divided into "Senso" and "Senso Card".
The first is meant as a full implementation with score display, light-up arcade buttons and wooden enclosure.
The latter is a EC-card sized implementation, driven by a button cell with buttons and LEDs directly on the PCB.

# Parts list

## Senso
Todo: screws are missing
| Part | Quantity | Description |
| ---- | -------- | ----------- |
| ATmega 328p (DIP) | 1 | |
| PCB       | 1 | |
| PCB Enclosure | 1 | 3D printed from two parts |
| 5V Arcade Buttons | 4 | 12V available from AliExpress, resistors need to be changed |
| Passive 12mm buzzer | 1 |
| Switch, 6mm | 1 | On/Off |
| 9V compartment | 1 | |
| 100 nF ceramic capacitors | 7 | |
| 330R resistor | 1 | For buzzer |
| 100K resistor | 1 | Pullup for reset line |
| 6 x 6mm button | 1 | Reset button |
| M3 self-tapping screws (<8mm length) | 4 | To screw PCB to enclosure |
| lm2596 module | 1 | |
| 5P jst xh 2.54, female + male | 3 | |
| 2P jst xh 2.54, female + male | 3 | |
| 6P Pinheader, female | 3 | For programming |


## SensoCard

| Part | Quantity |
| ---- | -------- |
| ATtiny 84 (DIP) | 1 |
| PCB       | 1 |
| PCB Holder | 1 |
| 12 x 12mm button | 4 |
| Colored button caps | 4 |
| Passive 12mm buzzer | 1 |
| Colored 5mm LEDs | 4 |
| SS12D00G3 Switch | 1 |
| CR2032 Holder | 1 |
| 100 nF ceramic capacitors | 5 |
| Resistors | 5 |
| M2 self-tapping screws (<8mm length) | 4 |
