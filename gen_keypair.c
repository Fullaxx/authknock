#define _ISOC99_SOURCE

#include <stdio.h>
#include <stdint.h>

#include <sodium.h>

/* for libnacl
#include "nacl/crypto_box.h"
*/

static int savefile(char *filename, void *bytes, size_t size)
{
	FILE *f;
	size_t written;

	f = fopen(filename, "w");
	if(!f) {
		fprintf(stderr, "fopen(%s, w) failed!\n", filename);
		return 1;
	}

	written = fwrite(bytes, 1, size, f);
	if(written != size) {
		fprintf(stderr, "fwrite() failed!\n");
		return 2;
	}

	fclose(f);
	return 0;
}

int main(int argc, char *argv[])
{
	int err;
	char filename[1024];
	uint8_t public_key[crypto_box_PUBLICKEYBYTES];
	uint8_t secret_key[crypto_box_SECRETKEYBYTES];
/*
	int i, n;
	char public_str[(2*crypto_box_PUBLICKEYBYTES)+1];
	char secret_str[(2*crypto_box_SECRETKEYBYTES)+1];
*/

	if(argc != 2) {
		fprintf(stderr, "%s: <NAME>\n", argv[0]);
		return 1;
	}

	/* Generate Key Pair */
	crypto_box_keypair(&public_key[0], &secret_key[0]);

/*
	n = 0;
	for(i=0; i<crypto_box_PUBLICKEYBYTES; i++) {
		n += snprintf(&public_str[n], sizeof(public_str)-n, "%02x", public_key[i]);
	}

	n = 0;
	for(i=0; i<crypto_box_SECRETKEYBYTES; i++) {
		n += snprintf(&secret_str[n], sizeof(secret_str)-n, "%02x", secret_key[i]);
	}

	printf("public key: %s\n", public_str);
	printf("secret key: %s\n", secret_str);
*/

	snprintf(filename, sizeof(filename), "%s.pub", argv[1]);
	err = savefile(filename, &public_key[0], crypto_box_PUBLICKEYBYTES);
	if(err) {
		printf("ERROR saving public key: %s\n", filename);
	} else {
		printf("Saved public key: %s\n", filename);
	}

	snprintf(filename, sizeof(filename), "%s.key", argv[1]);
	err = savefile(filename, &secret_key[0], crypto_box_SECRETKEYBYTES);
	if(err) {
		printf("ERROR saving secret key: %s\n", filename);
	} else {
		printf("Saved secret key: %s\n", filename);
	}

	return 0;
}
