/*
SPI-Decoder for Sigrok-captures

version 0.01

hacked together by kitten_nb_five

https://github.com/kittennbfive

AGPLv3+ and NO WARRANTY!
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <err.h>
#include <zip.h>

#define LENGTH_NAME_MAX 10
#define NB_CHANNEL_MAX 5

#define SZ_MALLOC_MAX (100*1024*1024) //just as a sanity check to avoid nasty surprises

bool get_file_size(zip_t * zip, char const * const fname, zip_uint64_t * const filesize)
{
	zip_stat_t stat;
	if(zip_stat(zip, fname, 0, &stat))
	{
		zip_error_t * error=zip_get_error(zip);
		if(error->zip_err==ZIP_ER_NOENT)
		{
			*filesize=0;
			zip_error_fini(error);
			return false;
		}
		else
			errx(1, "zip_stat(%s) failed", fname);
	}
	
	if(!(stat.valid&ZIP_STAT_SIZE))
		errx(1, "zip_stat(%s) did not return a valid filesize", fname);

	*filesize=stat.size;

	return true;
}

bool get_file(zip_t * zip, char const * const fname, uint8_t **buffer, zip_uint64_t * const filesize, const bool c_string)
{
	zip_uint64_t sz;
	
	if(!get_file_size(zip, fname, &sz))
		return false;
	
	if(sz>SZ_MALLOC_MAX)
		errx(1, "file %s is bigger than hardcoded limit", fname);
	
	if(sz<*filesize)
		*filesize=sz;
	
	zip_file_t * file=zip_fopen(zip, fname, 0);
	if(!file)
		errx(1, "zip_open(%s) failed", fname);
	
	if(!*buffer)
	{
		*buffer=malloc(sz+(c_string?1:0));
		if(!(*buffer))
			err(1, "malloc failed for %s", fname);
		*filesize=sz;
	}
	else
	{
		if(*filesize+(c_string?1:0)<sz)
		{
			warnx("buffer to small for file %s, calling realloc", fname);
			uint8_t *tmp=realloc(*buffer, sz);
			if(!tmp)
				err(1, "realloc failed for %s", fname);
			*buffer=tmp;
			*filesize=sz;
		}
	}
	
	zip_int64_t bytes_read=zip_fread(file, *buffer, *filesize);
	if(bytes_read<0 || (zip_uint64_t)bytes_read!=*filesize)
		errx(1, "zip_fread(%s) failed", fname);
	
	if(zip_fclose(file))
		errx(1, "zip_fclose(%s) failed", fname);
	
	if(c_string)
		(*buffer)[*filesize]='\0';
	
	return true;
}

bool get_logic_file(zip_t * zip, const unsigned int nb_logic_file, uint8_t **buffer, zip_uint64_t * const filesize)
{
	char name_logic_file[20];
	sprintf(name_logic_file, "logic-1-%u", nb_logic_file);
	
	return get_file(zip, name_logic_file, buffer, filesize, false);
}

void check_version(zip_t * zip)
{
	uint8_t * version=NULL;
	zip_uint64_t sz_version;
	
	if(!get_file(zip, "version", &version, &sz_version, false) ||sz_version!=1 || version[0]!='2')
		errx(1, "version %u not supported!", version[0]);
	
	free(version);
}

typedef struct
{
	unsigned int number;
	char name[LENGTH_NAME_MAX+1];
} channel_t;

channel_t channels[NB_CHANNEL_MAX];
unsigned int nb_channels=0;

void parse_line(char * l)
{
	if(!strlen(l))
		return;
	
	if(strstr(l, "probe")!=l)
		return;
	
	unsigned int number;
	char name[10];
	
	if(sscanf(l, "probe%u=%s", &number, name)!=2)
		warnx("could not parse line \"%s\"", l);
	else
	{
		channels[nb_channels].number=number;
		strcpy(channels[nb_channels].name, name);
		nb_channels++;
	}
}

void parse_metadata(zip_t * zip)
{
	char * metadata=NULL;
	zip_uint64_t sz_metadata;
	
	if(!get_file(zip, "metadata", (uint8_t**)&metadata, &sz_metadata, true))
		errx(1, "could not read metadata");
	
	(void)sz_metadata;
	
	char *l;
	l=strtok(metadata, "\n");
	parse_line(l);
	while((l=strtok(NULL, "\n")))
		parse_line(l);
	
	free(metadata);
}

unsigned int get_channel_bitpos(char const * const name)
{
	unsigned int i;
	for(i=0; i<nb_channels; i++)
	{
		if(!strcmp(channels[i].name, name))
			return channels[i].number-1;
	}
	
	errx(1, "could not find channel %s", name);
}

void print_usage(void)
{
	printf("usage: $executable $filename $command [$channelname_CS $channelname_CLK $channelname_MOSI [$channelname_MISO]]\n\n");
	printf("$command:\n  \"info\" -> print channelnames found in file and exit\n  \"print\" -> print decoded values in hex\n  \"binary\" -> dump decoded values as raw binary (for piping to another tool or saving into a file)\n");
}

typedef enum
{
	info,
	print,
	binary
} output_mode_t;

int main(int argc, char **argv)
{
	fprintf(stderr, "This is an SPI-Decoder version 0.01 for Sigrok-captures.\n(c) 2021 by kitten_nb_five\nUSE AT YOUR OWN RISK!\n");
	
	if(argc<3)
	{
		print_usage();
		return 0;
	}
	
	zip_t * zip;
	int err;
	zip=zip_open(argv[1], ZIP_RDONLY, &err);
	if(!zip)
		errx(1, "zip_open(%s) failed with error %d", argv[1], err);
	
	check_version(zip);
	
	parse_metadata(zip);
	
	output_mode_t mode;
	
	if(!strcmp(argv[2], "info"))
		mode=info;
	else if(!strcmp(argv[2], "print"))
		mode=print;
	else if(!strcmp(argv[2], "binary"))
		mode=binary;
	else
		errx(1, "invalid command %s", argv[2]);
	
	if(mode==info)
	{
		printf("found %u channels in file %s:\n", nb_channels, argv[1]);
		unsigned int i;
		for(i=0; i<nb_channels; i++)
			printf("%u: %s\n", channels[i].number, channels[i].name);
		return 0;
	}
		
	if(argc!=6 && argc!=7)
	{
		print_usage();
		return 0;
	}
	
	unsigned int channel_CS, channel_CLK, channel_MOSI, channel_MISO;
	bool has_miso=false;
	
	channel_CS=get_channel_bitpos(argv[3]);
	channel_CLK=get_channel_bitpos(argv[4]);
	channel_MOSI=get_channel_bitpos(argv[5]);
	if(argc==7)
	{
		channel_MISO=get_channel_bitpos(argv[6]);
		has_miso=true;
	}
	
	unsigned int nb_logic_file=1;
	uint8_t * data=NULL;
	zip_uint64_t sz_buffer=0;
	zip_uint64_t pos_total=0;
	zip_uint64_t pos_in_chunk=0;
	
	uint8_t cs_old=1;
	uint8_t clk_old=0;
	uint8_t counter=0;
	uint8_t byte_mosi=0;
	uint8_t byte_miso=0;
	
	while(get_logic_file(zip, nb_logic_file, &data, &sz_buffer))
	{
		for(pos_in_chunk=0; pos_in_chunk<sz_buffer; )
		{
			uint8_t cs=(data[pos_in_chunk]>>channel_CS)&1;
			uint8_t clk=(data[pos_in_chunk]>>channel_CLK)&1;
			uint8_t mosi=(data[pos_in_chunk]>>channel_MOSI)&1;
			uint8_t miso;
			if(has_miso)
				miso=(data[pos_in_chunk]>>channel_MISO)&1;
						
			if(!cs && cs_old)
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
					if(mode==print)
						printf("MOSI: 0x%02x\n", byte_mosi);
					else
						putc(byte_mosi, stdout);
					if(has_miso)
					{
						if(mode==print)
							printf("MISO: 0x%02x\n", byte_miso);
						else
							putc(byte_miso, stdout);
					}
				}
			}
			
			cs_old=cs;
			clk_old=clk;
			pos_in_chunk++;
			pos_total++;
		}
		
		nb_logic_file++;
	}
	
	free(data);
	
	zip_close(zip);
	
	fprintf(stderr, "parsed %lu samples successfully. End.\n", pos_total);
	return 0;
}
