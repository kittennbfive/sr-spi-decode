# sr-spi-decode  
small and fast SPI-decoder for Sigrok-captures, written in C

## What is this?
This is a small tool that parses a [Sigrok](https://sigrok.org/)-capture (.sr) containing SPI-data, extracts all the bytes that have been sent (MOSI and optionally MISO) and spit them out as hex or binary.

## Licence and Disclaimer
AGPLv3+ and NO WARRANTY!

## Sigrok does have a SPI-decoder already?!
Yes and Sigrok/Pulseview is really a great tool, but there are two problems:  
-sigrok-cli eats up all the RAM and makes my computer crash. (TODO: Investigate)  
-Pulseview works fine and the new versions can save the data as binary, but it is *really* slow. I have a file with 1,3GB worth of captured data with about 7000 SPI-transactions. On my not-so-old computer Pulseview with the integrated decoder needs 30 minutes(!) and 3GB of RAM to process the entire thing. My tool, written in C and compiled with -O3, needs less than 10 seconds(!) and much less RAM too. Of course you won't get all the fancy graphical stuff that Pulseview offers.

## Why not improve Sigrok?
The Sigrok-decoders are written in Python which i don't know. Pulseview is written in C++ as far as i know, same problem. sigrok-cli is written in C and i may work on it at some point, but for the moment i needed a way to quickly get my SPI-data out of the captures so i made this tool.

## External depencies?
Yes, [libzip](https://libzip.org/). On Debian you need ```libzip-dev```.

## How to compile?  
```gcc -Wall -Wextra -O3 -lzip -o main main.c```

## How to use?
>usage: $executable $filename $command \[$channelname_CS $channelname_CLK $channelname_MOSI \[$channelname_MISO\]\]  
>  
>$command:  
>  "info" -> print channelnames found in file and exit  
>  "print" -> print decoded values in hex  
>  "binary" -> dump decoded values as raw binary (for piping to another tool or saving into a file)  

As you can see it is pretty basic but it gets the job done and *fast*. I sometimes have huge captures to process and waiting like 30 minutes is just not acceptable.
  
Beware: At this point this tool is mostly untested, expect bugs and other bad stuff.

## Limitations?  
A lot: Linux-only, CS must be active low, SPI-mode is fixed to 1, transfersize is fixed to 8 bits, ... However with some basic C-knowledge it should be easy and quick to adapt the code. I may upload a better version some day if i make one (=if i need one).

