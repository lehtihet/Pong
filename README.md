<h1 align="center">An implementation of the classic arcade game <a href="https://en.wikipedia.org/wiki/Pong">Pong</a> on a chipKIT Uno32 Basic Microcontroller Board </h1>


<p align="center">
  <img src="./docs/images/Pong.png" />
</p>


## About

This project was developed as part of the course Computer Organization and Components at KTH. A code skeleton was provided, and most of our changes are in the files ```mipslabmain.c``` and ```pong.c```

The point of the project was to write directly to the hardware at a low level and not use built in libraries. 

## Features


- **Multiplayer**
- **Singleplayer against an AI with two difficulty options**
- **Players are controlled through the switches on the I/O Shield**
- **A Main menu with navigation done through switches and buttons**
- **A highscore which is saved between power-outs**
- **Pixel-by-pixel updates of all items on the display**

## Implementation


- **The code takes advantage of switches, buttons, timers, polling, interrupts and more.** 
- **Writing to the OLED display was done through writing directly to the memory using the SPI protocol.**
- **A persistent highscore is implemented through writing directly to the EEPROM using the I2C protocol.** 


