/*
simple SPI-Decoder for Sigrok-captures

version 0.03 using libsr

(c) 2021-2023 by kittennbfive

https://github.com/kittennbfive

AGPLv3+ and NO WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>

#include "libsr.h"

void print_usage(void)
{
	printf("usage: $executable $filename $mode $channelname_CS $channelname_CLK $channelname_MOSI [$channelname_MISO]\n\n");
	printf("$mode:\n\t\"print\" -> print decoded values in hex\n\t\"binary\" -> dump decoded values as raw binary (for piping to another tool or saving into a file)\n");
}

typedef enum
{
	MODE_PRINT_HEX,
	MODE_PRINT_BIN
} output_mode_t;

int main(int argc, char **argv)
{
	fprintf(stderr, "This is a simple SPI-Decoder for Sigrok-captures\nversion 0.03 - (c) 2021-2023 by kittennbfive - AGPLv3+ - USE AT YOUR OWN RISK!\n\n");
	
	if(argc<6 || argc>7)
	{
		print_usage();
		return 0;
	}
	
	sr_open(argv[1]);
	
	output_mode_t mode;

	if(!strcmp(argv[2], "print"))
		mode=MODE_PRINT_HEX;
	else if(!strcmp(argv[2], "binary"))
		mode=MODE_PRINT_BIN;
	else
		errx(1, "invalid mode %s", argv[2]);
	
	uint_fast8_t channel_CS, channel_CLK, channel_MOSI, channel_MISO;
	bool has_miso=false;
	
	channel_CS=sr_get_channel_bitpos(argv[3]);
	channel_CLK=sr_get_channel_bitpos(argv[4]);
	channel_MOSI=sr_get_channel_bitpos(argv[5]);
	if(argc==7)
	{
		channel_MISO=sr_get_channel_bitpos(argv[6]);
		has_miso=true;
	}
	
	uint64_t nb_samples=sr_get_nb_samples();
	uint64_t sample;
	
	bool cs_old=1;
	bool clk_old=0;
	uint8_t counter=0;
	uint8_t byte_mosi=0;
	uint8_t byte_miso=0;
	
	for(sample=0; sample<nb_samples; sample++)
	{
		bool cs=sr_get_sample(channel_CS, sample);
		bool clk=sr_get_sample(channel_CLK, sample);
		bool mosi=sr_get_sample(channel_MOSI, sample);
		bool miso;
		if(has_miso)
			miso=sr_get_sample(channel_MISO, sample);
		
		if(!cs && cs_old) //reset
		{
			counter=8;
			byte_mosi=0;
			byte_miso=0;
		}
		else if(clk && !clk_old && !cs && counter)
		{
			byte_mosi|=mosi<<(counter-1);
			if(has_miso)
				byte_miso|=miso<<(counter-1);
			counter--;
			
			if(counter==0)
			{
				if(mode==MODE_PRINT_HEX)
					printf("MOSI: 0x%02x\n", byte_mosi);
				else
					putc(byte_mosi, stdout);
				if(has_miso)
				{
					if(mode==MODE_PRINT_HEX)
						printf("MISO: 0x%02x\n", byte_miso);
					else
						putc(byte_miso, stdout);
				}
				
				//prepare for next byte that might follow directly
				counter=8;
				byte_mosi=0;
				byte_miso=0;
			}
		}
		
		cs_old=cs;
		clk_old=clk;
	}
	
	sr_close();
	
	fprintf(stderr, "parsed %lu samples successfully. End.\n", nb_samples);

	return 0;
}
