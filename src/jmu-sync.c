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

#if !HAVE_WINSOCK2_H
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <signal.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif /* HAVE_WINSOCK2_H */

#include <jack/jack.h>
#include <jack/transport.h>

jack_client_t *client;

jack_nframes_t lastframe = NULL;
int udp_port = 23867;
const char *udp_ip = "127.0.0.1"; // where the master sends datagrams
// (can be a broadcast address)

static void send_udp(const char *send_to_ip, int port, char *mesg) {
	static int sockfd = -1;
	static struct sockaddr_in socketinfo;

	if (sockfd == -1) {
		static const int one = 1;
		int ip_valid = 0;

		//  startup();
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == -1)
//            exit_player(EXIT_ERROR);
			perror("Socket error");

		// Enable broadcast
		setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));

#if HAVE_WINSOCK2_H
		socketinfo.sin_addr.s_addr = inet_addr(send_to_ip);
		ip_valid = socketinfo.sin_addr.s_addr != INADDR_NONE;
#else
		ip_valid = inet_aton(send_to_ip, &socketinfo.sin_addr);
#endif
//
//        if (!ip_valid) {
//            mp_msg(MSGT_CPLAYER, MSGL_FATAL, MSGTR_InvalidIP);
//            exit_player(EXIT_ERROR);
//        }

		socketinfo.sin_family = AF_INET;
		socketinfo.sin_port = htons(port);
	}
	int broadcastPermission;
	broadcastPermission = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
			(void *) &broadcastPermission, sizeof(broadcastPermission)) < 0)
		perror("setsockopt() failed");

	sendto(sockfd, mesg, strlen(mesg), 0, (struct sockaddr *) &socketinfo,
			sizeof(socketinfo));
}

static void showtime() {
	jack_position_t current;
	jack_transport_state_t transport_state;
	jack_nframes_t frame_time;

	transport_state = jack_transport_query(client, &current);
	frame_time = jack_frame_time(client);

	float time = (float) current.frame / (float) current.frame_rate;
	char current_time[256];
	snprintf(current_time, sizeof(current_time), "%f", time);
	send_udp(udp_ip, udp_port, current_time);
	if (lastframe != current.frame) {
		printf(
				"\x1b[Aframe = %u  frame_time = %u usecs b= %lld fr:%i time: %f\t",
				current.frame, frame_time, current.usecs, current.frame_rate,
				time);
		lastframe = current.frame;

		switch (transport_state) {
		case JackTransportStopped:
			printf("state: Stopped");
			break;
		case JackTransportRolling:
			printf("state: Rolling");
			break;
		case JackTransportStarting:
			printf("state: Starting");
			break;
		default:
			printf("state: [unknown]");
			break;
		}

		if (current.valid & JackPositionBBT)
			printf("\tBBT: %3" PRIi32 "|%" PRIi32 "|%04"
			PRIi32, current.bar, current.beat, current.tick);

		if (current.valid & JackPositionTimecode)
			printf("\tTC: (%.6f, %.6f)", current.frame_time, current.next_time);
		printf("\n");
	}
}

static void jack_shutdown(void *arg) {
	fprintf(stderr, "JACK shut down, exiting ...\n");
	exit(1);
}

void signal_handler(int sig) {
	jack_client_close(client);
	fprintf(stderr, "signal received, exiting ...\n");
	exit(0);
}

int main(int argc, char *argv[]) {
	/* try to become a client of the JACK server */
	int c;
	while ((c = getopt(argc, argv, "a:p:")) != -1)
		switch (c) {
		case 'a':
			udp_ip = optarg;
			break;
		case 'p':
			udp_port = optarg;
			break;
		default:
			abort();
			break;
		}

	printf("Starting..\n");

	if ((client = jack_client_open("showtime", JackNullOption, NULL)) == 0) {
		fprintf(stderr, "JACK server not running?\n");
		return 1;
	}

	signal(SIGQUIT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);

	/* tell the JACK server to call `jack_shutdown()' if
	 it ever shuts down, either entirely, or if it
	 just decides to stop calling us.
	 */

	jack_on_shutdown(client, jack_shutdown, 0);

	/* tell the JACK server that we are ready to roll */

	if (jack_activate(client)) {
		fprintf(stderr, "cannot activate client");
		return 1;
	}

	while (1) {
		usleep(20);
		showtime();
	}

	jack_client_close(client);
	exit(0);
}
