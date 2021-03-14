#include <stdio.h>
#include <stdlib.h>
#include <sodium.h>

#include "authknock.h"

extern unsigned char *g_publickey;
extern unsigned char *g_secretkey;

void handle_payload(char *cmd)
{
	int z;
	z = system(cmd);
	printf("%s (%d)", cmd, z);
}

void process_payload(const unsigned char *buf, int len)
{
	int i, err;
	uint8_t nonce[NSIZE];
	uint8_t ct[CTSIZE];
	uint8_t pt[PTSIZE];

	if(len < (NSIZE + CTSIZE)) { return; }

	for(i=0; i<NSIZE; i++) {
		nonce[i] = buf[i];
	}

/*
#ifdef DEBUG
	printf("nonce: ");
	for(i=0; i<NSIZE; i++) {
		printf("%02X", nonce[i]);
	}
	printf("\n");
#endif
*/

	buf += NSIZE;
	len -= NSIZE;

	for(i=0; i<CTSIZE; i++) {
		ct[i] = buf[i];
	}

/*
#ifdef DEBUG
	printf("ct: ");
	for(i=0; i<CTSIZE; i++) {
		printf("%02X", ct[i]);
	}
	printf("\n\n");
#endif
*/

	err = crypto_box_open_easy(pt, ct, CTSIZE, nonce, g_publickey, g_secretkey);
	if (err != 0) { fprintf(stderr, "crypto_box_open_easy() failed!\n"); }
	else { handle_payload((char *)pt); }
}
