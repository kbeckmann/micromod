#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "micromod.h"

/*
	Simple command-line test player for micromod using SDL.
*/

#define SAMPLING_FREQ  48000  /* 48khz. */
#define NUM_CHANNELS   2      /* Stereo. */
#define BUFFER_SAMPLES 16384  /* 64k buffer. */

static long samples_remaining;
static short mix_buffer[ BUFFER_SAMPLES * NUM_CHANNELS * 4 ];
static long get_audio( int16_t *stream, int len ) {
	long count;
	count = len / 4;
	if( samples_remaining < count ) {
		/* Clear output.*/
		memset( stream, 0, len );
		count = samples_remaining;
	}
	if( count > 0 ) {
		/* Get audio from replay.*/
		memset( mix_buffer, 0, count * NUM_CHANNELS * sizeof( short ) );
		micromod_get_audio( stream, count );
		samples_remaining -= count;
	} else {
	}
        return count;
}
static long read_file( char *filename, void *buffer, long length ) {
	FILE *file;
	long count;
	count = -1;
	file = fopen( filename, "rb" );
	if( file != NULL ) {
		count = fread( buffer, 1, length, file );
		if( count < length && !feof( file ) ) {
			fprintf( stderr, "Unable to read file '%s'.\n", filename );
			count = -1;
		}
		if( fclose( file ) != 0 ) {
			fprintf( stderr, "Unable to close file '%s'.\n", filename );
		}
	}
	return count;
}

static void print_module_info() {
	int inst;
	char string[ 23 ];
	for( inst = 0; inst < 16; inst++ ) {
		micromod_get_string( inst, string );
		printf( "%02i - %-22s ", inst, string );
		micromod_get_string( inst + 16, string );
		printf( "%02i - %-22s\n", inst + 16, string );
	}
}

static long read_module_length( char *filename ) {
	long length;
	signed char header[ 1084 ];
	length = read_file( filename, header, 1084 );
	if( length == 1084 ) {
		length = micromod_calculate_mod_file_len( header );
		if( length < 0 ) {
			fprintf( stderr, "Module file type not recognised.\n");
		}
	} else {
		fprintf( stderr, "Unable to read module file '%s'.\n", filename );
		length = -1;
	}
	return length;
}

static long play_module( signed char *module ) {
	long result;
	/* Initialise replay.*/
	FILE *out;
        out = fopen("out.wav", "wb");
	result = micromod_initialise( module, SAMPLING_FREQ );
	if( result == 0 ) {
                int n;
		print_module_info();
		/* Calculate song length. */
		samples_remaining = micromod_calculate_song_duration();
		printf( "Song Duration: %li seconds.\n", samples_remaining / ( SAMPLING_FREQ ) );
		fflush( NULL );
                while(n = get_audio(mix_buffer, sizeof(mix_buffer))) {
                        fwrite(mix_buffer, n, 4, out);
                }
                fclose(out);

        }
}

int main( int argc, char **argv ) {
	int arg, result;
	long count, length;
	char *filename;
	signed char *module;
        filename = NULL;

	for( arg = 1; arg < argc; arg++ ) {
            filename = argv[ arg ];
	}
	result = EXIT_FAILURE;
	if( filename == NULL ) {
		fprintf( stderr, "Usage: %s [-reverb] filename\n", argv[ 0 ] );
	} else {
		/* Read module file.*/
		length = read_module_length( filename );
		if( length > 0 ) {
			printf( "Module Data Length: %li bytes.\n", length );
			module = calloc( length, 1 );
			if( module != NULL ) {
				count = read_file( filename, module, length );
				if( count < length ) {
					fprintf( stderr, "Module file is truncated. %li bytes missing.\n", length - count );
				}
				/* Play.*/
				if( play_module( module ) == 0 ) {
					result = EXIT_SUCCESS;
				}
				free( module );
			}
		}
	}
	return result;
}
