#define _XOPEN_SOURCE (500)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Compatibility with ?really? old versions of libnet */
#ifndef uint
typedef unsigned int uint;
#endif

#include <libnet.h>
#include <sodium.h>

#include "authknock.h"
#include "futils.h"
#include "getopts.h"
static void parse_args(int argc, char **argv);

char *g_dev = NULL;
uint16_t g_proto = 0xFFFF;
char *g_dst = NULL;
char *g_pfilename = NULL;
char *g_sfilename = NULL;
char *g_message = NULL;

static int send_rawip(libnet_t *l, uint8_t *payload, uint16_t payload_s, uint64_t src_ip, uint64_t dst_ip)
{
	int c ;
	libnet_ptag_t ip_ptag;

	ip_ptag = libnet_build_ipv4( LIBNET_IPV4_H + payload_s,
		0,				/* TOS */
		242,			/* IP ID */
		0,				/* IP Frag */
		64,				/* TTL */
		g_proto,		/* protocol */
		0,				/* checksum */
		src_ip,			/* source IP */
		dst_ip,			/* destination IP */
		payload,		/* payload */
		payload_s,		/* payload size */
		l,				/* libnet handle */
		0);				/* libnet id */

	if(ip_ptag == -1) {
		fprintf(stderr, "Can't build IP header: %s\n", libnet_geterror(l));
		libnet_destroy(l);
		exit(1);
	}

	c = libnet_write(l);
	if(c == -1) {
		fprintf(stderr, "Write error: %s\n", libnet_geterror(l));
		libnet_destroy(l);
		exit(1);
	} else {
		fprintf(stderr, "Wrote %d byte IP packet; proto %3d\n", c, g_proto);
	}

	libnet_clear_packet(l);
	return 0;
}

int main(int argc, char *argv[])
{
	int i, p, err;
	libnet_t *l;
	char errbuf[LIBNET_ERRBUF_SIZE];
	uint64_t src_ip, dst_ip;
	uint8_t nonce[NSIZE];
	uint8_t plaintext[PTSIZE];
	uint8_t ciphertext[CTSIZE];
	unsigned char *publickey;
	unsigned char *secretkey;
	uint8_t payload[PAYLOADSIZE];

	if((NSIZE+CTSIZE) > PAYLOADSIZE) {
		fprintf(stderr, "NSIZE + CTSIZE > PAYLOADSIZE\n");
		fprintf(stderr, "%4u + %6u > %11u\n", NSIZE, CTSIZE, PAYLOADSIZE);
		exit(EXIT_FAILURE);
	}

	parse_args(argc, argv);

	publickey = (unsigned char *)get_file(g_pfilename);
	if(!publickey) {
		fprintf(stderr, "get_file(%s) failed!\n", g_pfilename);
		exit(EXIT_FAILURE);
	}

	secretkey = (unsigned char *)get_file(g_sfilename);
	if(!secretkey) {
		fprintf(stderr, "get_file(%s) failed!\n", g_sfilename);
		exit(EXIT_FAILURE);
	}

/*
 *  Initialize the library.  Root priviledges are required.
 */
	l = libnet_init(LIBNET_RAW4, g_dev, errbuf);
	if(l == NULL) {
		fprintf(stderr, "libnet_init() failed: %s\n", errbuf);
		exit(EXIT_FAILURE);
	}

/*
	if(l->device) {
		printf("Using device %s\n", l->device);
	}
	if(libnet_getdevice(l)) {
		printf("Using device %s\n", libnet_getdevice(l));
	}
*/

	dst_ip = libnet_name2addr4(l, g_dst, LIBNET_RESOLVE);
	if(dst_ip == -1) {
		fprintf(stderr, "Bad destination IP address: %s\n", g_dst);
		exit(EXIT_FAILURE);
	}

	src_ip = libnet_get_ipaddr4(l);
	if(src_ip == -1) {
		fprintf(stderr, "Couldn't get own IP address: %s\n", libnet_geterror(l));
		exit(EXIT_FAILURE);
	} else {
		printf("Using: %s\n", libnet_addr2name4(src_ip, LIBNET_DONT_RESOLVE));
	}

/*
	for(proto = 0; proto < 256; proto++) {
		send_rawip(l, &payload[0], payload_s, proto, src_ip, dst_ip);
		usleep(100000);
	}
*/

	randombytes(nonce, NSIZE);
	randombytes(plaintext, PTSIZE);
	memset(ciphertext, 0, CTSIZE);
	snprintf((char *)plaintext, PTSIZE, "%s", g_message);
	err = crypto_box_easy(ciphertext, plaintext, PTSIZE, nonce, publickey, secretkey);
	if(err != 0) {
		fprintf(stderr, "crypto_box_easy() failed!\n");
	} else {
		p = 0;
		for(i=0; i<NSIZE; i++) { payload[p++] = nonce[i]; }
		for(i=0; i<CTSIZE; i++) { payload[p++] = ciphertext[i]; }
		send_rawip(l, &payload[0], p, src_ip, dst_ip);
	}

	if(g_dst) { free(g_dst); }
	if(g_dev) { free(g_dev); }
	if(g_pfilename) { free(g_pfilename); }
	if(g_sfilename) { free(g_sfilename); }
	if(publickey) { free(publickey); }
	if(secretkey) { free(secretkey); }
	if(g_message) { free(g_message); }
	libnet_destroy(l);
	return 0;
}

struct options opts[] = 
{
	{ 1, "dst",		"Destination",	"d",  1 },
	{ 2, "proto",	"Protocol",		"p",  1 },
	{ 3, "dev",		"Device",		"i",  1 },
	{ 4, "public",	"Public Key",	NULL, 1 },
	{ 5, "secret",	"Secret Key",	NULL, 1 },
	{ 6, "message",	"Message",		"m",  1 },
	{ 0, NULL,		NULL,			NULL, 0 }
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
				g_dst = strdup(args);
				break;
			case 2:
				g_proto = atoi(args);
				break;
			case 3:
				g_dev = strdup(args);
				break;
			case 4:
				g_pfilename = strdup(args);
				break;
			case 5:
				g_sfilename = strdup(args);
				break;
			case 6:
				g_message = strdup(args);
				break;
			default:
				fprintf(stderr, "Unexpected getopts Error! (%d)\n", c);
				break;
		}

		/* This free() is required since getopts() automagically allocates space for "args" everytime it's called. */
		free(args);
	}

	if(!g_dst) {
		fprintf(stderr, "I need a destination! (Fix with -d)\n");
		exit(EXIT_FAILURE);
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

	if(!g_message) {
		fprintf(stderr, "I need a message! (Fix with -m)\n");
		exit(EXIT_FAILURE);
	}

	if(strlen(g_message) > PTSIZE-1) {
		fprintf(stderr, "Message too large!\n");
		exit(EXIT_FAILURE);
	}
}
