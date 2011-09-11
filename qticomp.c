#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "image.h"
#include "qti.h"
#include "qtc.h"
#include "ppm.h"

int main( int argc, char *argv[] )
{
	struct image image;
	struct qti compimage;

	int opt, verbose;
	unsigned long int insize, bsize, outsize;
	int transform;
	int rangecomp;
	int minsize;
	int maxdepth;
	int maxerror;
	int lazyness;
	char mode;
	char *infile, *outfile;

	verbose = 0;
	transform = 0;
	rangecomp = 0;
	minsize = 2;
	maxdepth = 16;
	maxerror = 0;
	lazyness = 0;
	mode = 'c';
	infile = NULL;
	outfile = NULL;

	while( ( opt = getopt( argc, argv, "cvt:s:d:l:e:m:i:o:" ) ) != -1 )
	{
		switch( opt )
		{
			case 't':
				if( sscanf( optarg, "%i", &transform ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'c':
				rangecomp = 1;
			break;

			case 'v':
				verbose = 1;
			break;

			case 's':
				if( sscanf( optarg, "%i", &minsize ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'd':
				if( sscanf( optarg, "%i", &maxdepth ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'l':
				if( sscanf( optarg, "%i", &lazyness ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'e':
				if( sscanf( optarg, "%i", &maxerror ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'm':
				if( sscanf( optarg, "%c", &mode ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'i':
				infile = strdup( optarg );
			break;

			case 'o':
				outfile = strdup( optarg );
			break;

			default:
			case '?':
				fputs( "ERROR: Can not parse command line\n", stderr );
				return 1;
			break;
		}
	}

	if( mode == 'c' )
	{
		if( ! read_ppm( &image, infile ) )
			return 0;

		insize = image.width * image.height * 3;

		if( transform == 1 )
			transform_image_fast( &image );
		else if( transform == 2 )
			transform_image( &image );

		if( ! compress( &image, NULL, &compimage, maxerror, minsize, maxdepth, lazyness ) )
			return 0;

		bsize = qti_getsize( &compimage );

		compimage.transform = transform;
		
		if( ! ( outsize = write_qti( &compimage, rangecomp, outfile ) ) )
			return 0;
		
		free_image( &image );
		free_qti( &compimage );
		
		if( verbose )
			fprintf( stderr, "In:%lub Buff:%lub,%f%% Out:%lub,%f%%\n", insize, bsize/8, (bsize/8)*100.0/insize, outsize, outsize*100.0/insize );
	}
	else if( mode == 'd' )
	{
		if( ! read_qti( &compimage, infile ) )
			return 0;

		if( ! decompress( &compimage, NULL, &image ) )
			return 0;
	
		if( compimage.transform == 1 )
			transform_image_fast_rev( &image );
		else if( compimage.transform == 2 )
			transform_image_rev( &image );
		
		if( ! write_ppm( &image, outfile ) )
			return 0;
		
		free_image( &image );
		free_qti( &compimage );
	}
	else if( mode == 'a' )
	{
		if( ! read_qti( &compimage, infile ) )
			return 0;

		if( ! decompress_ccode( &compimage, &image, 0 ) )
			return 0;

		if( ! write_ppm( &image, outfile ) )
			return 0;
		
		free_image( &image );
		free_qti( &compimage );
	}
	else
	{
		fputs( "ERROR: Unknown mode\n", stderr );
		return 1;
	}

	free( infile );
	free( outfile );

	return 0;
}

