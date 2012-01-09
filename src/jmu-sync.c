/*
 ============================================================================
 Name        : jmu-sync.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include <jack/jack.h>
#include <jack/transport.h>

jack_client_t *client;

static void
showtime ()
{
	jack_position_t current;
	jack_transport_state_t transport_state;
	jack_nframes_t frame_time;

	transport_state = jack_transport_query (client, &current);
	frame_time = jack_frame_time (client);

	float t=(float)current.frame/(float)current.frame_rate;

	printf ("frame = %u  frame_time = %u usecs b= %lld fr:%i time: %f\t",  current.frame, frame_time, current.usecs,current.frame_rate,t);

	switch (transport_state) {
	case JackTransportStopped:
		printf ("state: Stopped");
		break;
	case JackTransportRolling:
		printf ("state: Rolling");
		break;
	case JackTransportStarting:
		printf ("state: Starting");
		break;
	default:
		printf ("state: [unknown]");
		break;
	}

	if (current.valid & JackPositionBBT)
		printf ("\tBBT: %3" PRIi32 "|%" PRIi32 "|%04"
			PRIi32, current.bar, current.beat, current.tick);

	if (current.valid & JackPositionTimecode)
		printf ("\tTC: (%.6f, %.6f)",
			current.frame_time, current.next_time);
	printf ("\n");
}

static void
jack_shutdown (void *arg)
{
    fprintf(stderr, "JACK shut down, exiting ...\n");
	exit (1);
}

void
signal_handler (int sig)
{
	jack_client_close (client);
	fprintf (stderr, "signal received, exiting ...\n");
	exit (0);
}

int
main (int argc, char *argv[])
{
	/* try to become a client of the JACK server */

	if ((client = jack_client_open ("showtime", JackNullOption, NULL)) == 0) {
		fprintf (stderr, "JACK server not running?\n");
		return 1;
	}

	signal (SIGQUIT, signal_handler);
	signal (SIGTERM, signal_handler);
	signal (SIGHUP, signal_handler);
	signal (SIGINT, signal_handler);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* tell the JACK server that we are ready to roll */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		return 1;
	}

	while (1) {
		usleep (20);
		showtime ();
	}

	jack_client_close (client);
	exit (0);
}
