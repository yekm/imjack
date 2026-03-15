//  -----------------------------------------------------------------------------
//
//  Copyright (C) 2006-2018 Fons Adriaensen <fons@linuxaudio.org>
//  
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  -----------------------------------------------------------------------------


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include "jclient.h"
#include "config.h"


#if ZITA_CONVOLVER_MAJOR_VERSION < 4
#error "This program requires zita-convolver version 4 or higher."
#endif


static const char   *clopt = "hvL:MVZN:s:";
static bool          M_opt = false;
static bool          V_opt = false;
static bool          Z_opt = false;
static bool          v_opt = false;
static bool          stop  = false;


Convproc      *convproc = 0;
Jclient       *jclient = 0;
char           jackname [NAMELEN];
char           jackserv [NAMELEN];
unsigned int   latency = 0;
unsigned int   options = 0;
unsigned int   fsamp = 0;
unsigned int   fragm = 0;
unsigned int   ninp = 0;
unsigned int   nout = 0;
unsigned int   size = 0;


static void help (void)
{
    fprintf (stderr, "\nJconvolver %s\n", VERSION);
    fprintf (stderr, "(C) 2006-2018 Fons Adriaensen  <fons@linuxaudio.org>\n\n");
    fprintf (stderr, "Usage: jconvolver <options> <config file> {<connect file>}\n");
    fprintf (stderr, "Options:\n");
    fprintf (stderr, "  -h                 Display this text\n");
    fprintf (stderr, "  -v                 Print partition list to stdout [off]\n");
    fprintf (stderr, "  -L <nframes>       Try to compensate <nframes> latency\n");
    fprintf (stderr, "  -M                 Use the FFTW_MEASURE option [off]\n");   
    fprintf (stderr, "  -V                 Use vector mode processing [off]\n");   
    fprintf (stderr, "  -Z                 Add noise for denormal protection [off]\n");   
    fprintf (stderr, "  -N <name>          Name to use as JACK client [jconvolver]\n");   
    fprintf (stderr, "  -s <server>        Jack server name\n");
    exit (1);
}


static void procoptions (int ac, char *av [], const char *where)
{
    int k;
    
    optind = 1;
    opterr = 0;
    while ((k = getopt (ac, av, (char *) clopt)) != -1)
    {
        if (optarg && (*optarg == '-'))
        {
            fprintf (stderr, "\n%s\n", where);
	    fprintf (stderr, "  Missing argument for '-%c' option.\n", k); 
            fprintf (stderr, "  Use '-h' to see all options.\n");
            exit (1);
        }
	switch (k)
	{
        case 'h' : help (); exit (0);
        case 'v' : v_opt = true; break;
        case 'L' : latency = atoi (optarg); break;
        case 'M' : M_opt = true; break;
        case 'V' : V_opt = true; break;
        case 'Z' : Z_opt = true; break;
        case 'N' : strncpy (jackname, optarg, NAMELEN - 1); break;
        case 's' : strncpy (jackserv, optarg, NAMELEN - 1); break;
        case '?':
            fprintf (stderr, "\n%s\n", where);
            if (optopt != ':' && strchr (clopt, optopt))
	    {
                fprintf (stderr, "  Missing argument for '-%c' option.\n", optopt); 
	    }
            else if (isprint (optopt))
	    {
                fprintf (stderr, "  Unknown option '-%c'.\n", optopt);
	    }
            else
	    {
                fprintf (stderr, "  Unknown option character '0x%02x'.\n", optopt & 255);
	    }
            fprintf (stderr, "  Use '-h' to see all options.\n");
            exit (1);
        default:
            abort ();
 	}
    }
}


static void sigint_handler (int)
{
    stop = true;
}


int main (int ac, char *av [])
{
    unsigned int k;
     
    if (zita_convolver_major_version () != ZITA_CONVOLVER_MAJOR_VERSION)
    {
	fprintf (stderr, "Zita-convolver does not match compile-time version.\n");
	return 1;
    }

    if (mlockall (MCL_CURRENT | MCL_FUTURE))
    {
        fprintf (stderr, "Warning: ");
        perror ("mlockall:");
    }

    memset (jackname, 0, NAMELEN);
    memset (jackserv, 0, NAMELEN);
    procoptions (ac, av, "On command line:");
    if (ac <= optind) help ();
    if (M_opt) options |= Convproc::OPT_FFTW_MEASURE;
    if (V_opt) options |= Convproc::OPT_VECTOR_MODE;
    if (config (av [optind]))
    {
	delete jclient;
	delete convproc;
        return 1;
    }
    jclient->denorm_protect (Z_opt);
    if (v_opt) convproc->print ();

    makeports ();
    jclient->start ();
    signal (SIGINT, sigint_handler); 
    while (! stop)
    {    
	usleep (100000);
	k = jclient->flags ();

	if (k & Jclient::FL_EXIT)
	{
	    puts ("Zombified by Jack, exiting.");
	    stop = true;
	}
	if (k & Jclient::FL_BUFF)
	{
	    puts ("Jack buffer size change, exiting.");
	    stop = true;
	}
	if (k & Convproc::FL_LOAD)
	{
	    puts ("CPU overload, exiting.");
	    stop = true;
	}
	if (k & Convproc::FL_LATE)
	{
	    printf ("Computation threads are late (%04x).\n", k & Convproc::FL_LATE);
	}
    }

    jclient->stop ();
    delete jclient;
    delete convproc;

    return 0;
}

