#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include <pcap.h>

#include "dissect.h"
#include "authknock.h"
#include "futils.h"
#include "getopts.h"
static void parse_args(int argc, char **argv);

int g_shutdown = 0;
uint64_t g_pktcount = 0;
char *g_dev = NULL;
uint16_t g_proto = 0xFFFF;
int g_linktype = 0;
char *g_pfilename = NULL;
char *g_sfilename = NULL;
unsigned char *g_publickey = NULL;
unsigned char *g_secretkey = NULL;

void sig_handler(int signum)
{
	switch(signum) {
		case SIGHUP:
		case SIGINT:
		case SIGTERM:
		case SIGQUIT:
			g_shutdown = 1;
			break;
	}
}

int main(int argc, char *argv[])
{
	pcap_t *ph;
	int cnt, err;
	char filter[256];
	struct bpf_program fp;
	char pcap_errbuf[PCAP_ERRBUF_SIZE];

	parse_args(argc, argv);

	g_publickey = (unsigned char *)get_file(g_pfilename);
	if(!g_publickey) {
		fprintf(stderr, "get_file(%s) failed!\n", g_pfilename);
		exit(EXIT_FAILURE);
	}

	g_secretkey = (unsigned char *)get_file(g_sfilename);
	if(!g_secretkey) {
		fprintf(stderr, "get_file(%s) failed!\n", g_sfilename);
		exit(EXIT_FAILURE);
	}

/*
	pcap_dev = pcap_findalldevs(pcap_errbuf);
	if(!pcap_dev) {
		fprintf(stderr, "Couldnt get an interface with pcap_findalldevs: %s\n", pcap_errbuf);
        return 1;
	}
*/

	ph = pcap_open_live(g_dev, 65535, 1, 1, pcap_errbuf);
	if(!ph) {
        fprintf(stderr, "pcap_open_live(%s, 65535, 1, 1): %s\n", g_dev, pcap_errbuf);
        return 2;
	}

	g_linktype = pcap_datalink(ph);

	snprintf(filter, sizeof(filter), "ip[9] == %u", g_proto);
	err = pcap_compile(ph, &fp, filter, 1, PCAP_NETMASK_UNKNOWN);
	if(err == PCAP_ERROR) {
		fprintf(stderr, "pcap_compile(%s): %s\n", filter, pcap_errbuf);
		return 3;
	}

	err = pcap_setfilter(ph, &fp);
	if(err == PCAP_ERROR) {
		fprintf(stderr, "pcap_setfilter(%s): %s\n", filter, pcap_errbuf);
		return 4;
	}

	err = pcap_setnonblock(ph, 1, pcap_errbuf);
	if(err == PCAP_ERROR) {
		fprintf(stderr, "pcap_setnonblock(1): %s\n", pcap_errbuf);
		return 5;
	}

	printf("Starting capture on %s (%s)\n", (g_dev ? g_dev : "all"), "LINKTYPE");
	printf("Looking for IP Protocol %u\n", g_proto);

	signal(SIGINT,	sig_handler);
	signal(SIGTERM, sig_handler);
	signal(SIGQUIT,	sig_handler);
	signal(SIGHUP,	sig_handler);
	//signal(SIGALRM, alarm_handler);
	//(void) alarm(1);

	while(!g_shutdown) {
		cnt = pcap_dispatch(ph, 1, &handle_packet, NULL);
		if(cnt > 0) {
			g_pktcount += cnt;
		} else if(cnt == 0) {
			usleep(25);
		} else {
			fprintf(stderr, "pcap_dispatch() error: %d\n", cnt);
		}
	}

	pcap_close(ph);
	if(g_dev) { free(g_dev); }
	if(g_pfilename) { free(g_pfilename); }
	if(g_sfilename) { free(g_sfilename); }
	if(g_publickey) { free(g_publickey); }
	if(g_secretkey) { free(g_secretkey); }
	printf("Packet Count: %lu\n", g_pktcount);
	return 0;
}

struct options opts[] = 
{
	{ 1, "proto",	"IP Protocol to listen for",	"p",	1 },
	{ 2, "dev",		"Interface to listen on",		"i",	1 },
	{ 3, "public",	"Public Key",					NULL, 1 },
	{ 4, "secret",	"Secret Key",					NULL, 1 },
	{ 0, NULL,		NULL,							NULL,	0 }
};

static void parse_args(int argc, char **argv)
{
	char *args;
	int c;

	while ((c = getopts(argc, argv, opts, &args)) != 0) {
		switch(c) {
			case -2:
				/* Special Case: Recognize options that we didn't set above. */
				fprintf(stderr, "Unknown Getopts Option: %s\n", args);
				break;
			case -1:
				/* Special Case: getopts() can't allocate memory. */
				fprintf(stderr, "Unable to allocate memory for getopts().\n");
				exit(EXIT_FAILURE);
				break;
			case 1:
				g_proto = atoi(args);
				break;
			case 2:
				g_dev = strdup(args);
				break;
			case 3:
				g_pfilename = strdup(args);
				break;
			case 4:
				g_sfilename = strdup(args);
				break;
			default:
				fprintf(stderr, "Unexpected getopts Error! (%d)\n", c);
				break;
		}

		/* This free() is required since getopts() automagically allocates space for "args" everytime it's called. */
		free(args);
	}

	if(g_proto > 255) {
		fprintf(stderr, "I need a protocol! (Fix with -p)\n");
		exit(EXIT_FAILURE);
	}

	if(!g_pfilename) {
		fprintf(stderr, "I need a Public Key! (Fix with --public)\n");
		exit(EXIT_FAILURE);
	}

	if(!g_sfilename) {
		fprintf(stderr, "I need a Secret Key! (Fix with --secret)\n");
		exit(EXIT_FAILURE);
	}
}
