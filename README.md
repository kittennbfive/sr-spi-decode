# sr-spi-decode  
small and fast SPI-decoder for Sigrok-captures, written in C

## What is this?
This is a small tool that parses a [Sigrok](https://sigrok.org/)-capture (.sr) containing SPI-data, extracts all the bytes that have been sent (MOSI and optionally MISO) and spit them out as hex or binary. This is version 0.03 rewritten for libsr, see below.

## Licence and Disclaimer
AGPLv3+ and NO WARRANTY!

## How to compile
### Dependencies
You will need [libzip](https://libzip.org/). On Debian you need `libzip-dev`. You will also need my own [libsr](https://github.com/kittennbfive/libsr).

### Compiling
`gcc -Wall -Wextra -Werror -O3 -o main libsr.c main.c -lzip`

## Usage
>usage: $executable $filename $mode $channelname_CS $channelname_CLK $channelname_MOSI [$channelname_MISO]  
>  
>$mode:  
>"print" -> print decoded values in hex  
>"binary" -> dump decoded values as raw binary (for piping to another tool or saving into a file)

As you can see it is pretty basic but it gets the job done and *fast*.  
  
Beware: At this point this tool is mostly untested, expect bugs and other bad stuff.  
  
Version 0.02 drops the "info" mode, just open the sr-file in Pulseview or use sigrok-cli.

## Limitations
A lot: Linux-only, CS must be active low, SPI-mode is fixed to **0** (not 1 as written before), transfersize is fixed to 8 bits, ... However with some basic C-knowledge it should be easy and quick to adapt the code. I may upload a better version some day if i make one (=if i need one).

## FAQ
### Sigrok does have a SPI-decoder already?!
Yes and Sigrok/Pulseview is really a great tool, but there are two problems:  
-sigrok-cli eats up all the RAM and makes my computer crash. (TODO: Investigate)  
-Pulseview works fine and the new versions can save the data as binary, but it is *really* slow. I have a file with 1,3GB worth of captured data with about 7000 SPI-transactions. On my not-so-old computer Pulseview with the integrated decoder needs 30 minutes(!) and 3GB of RAM to process the entire thing. My tool, written in C and compiled with -O3, needs less than 10 seconds(!) and much less RAM too. Of course you won't get all the fancy graphical stuff that Pulseview offers. *Note: Those numbers are old, i no longer have the capture to check with newer versions of Sigrok and my tool.*

### Why not improve Sigrok?
The Sigrok-decoders are written in Python which i don't know. Pulseview is written in C++ as far as i know, same problem. sigrok-cli is written in C and i may work on it at some point, but for the moment i needed a way to quickly get my SPI-data out of the captures so i made this tool.
