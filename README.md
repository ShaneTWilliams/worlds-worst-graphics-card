# World's Worst Graphics Card

This board consists of an SiI1136 HDMI transmitter, driven by a dual-core 480 MHz STM32H745IIT6 microcontroller, with 64 MB of external SDRAM. It is USB-powered and uses SWD for flash and debug.

The goal of this project is to give me a custom, bare-metal platform on which I can try my hand at implementing simple graphics algorithms from scratch. The STM32 is key for this, with its dual cores, DMA2D (a DMA peripheral designed to fill frame buffers very quickly) and its LTDC peripheral, which will drive the display without any CPU load.

![Screenshot 2021-01-01 195043](https://user-images.githubusercontent.com/44215543/103459964-31b77980-4ced-11eb-8380-6e7c4df7adda.png)

![Screenshot 2021-01-02 112723](https://user-images.githubusercontent.com/44215543/103460026-a1c5ff80-4ced-11eb-8404-2e1119439eb0.png)

![Screenshot 2021-01-02 112815](https://user-images.githubusercontent.com/44215543/103460027-a25e9600-4ced-11eb-8bff-0aaf9a81aada.png)
