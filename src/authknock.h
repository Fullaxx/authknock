#ifndef __AUTHKNOCK_H__
#define __AUTHKNOCK_H__

#include <sodium.h>

/* MAX payload size if wrapped in UDP */
#define PAYLOADSIZE (1500-20-8)

#define NSIZE  (crypto_box_NONCEBYTES)
#define CTSIZE (1400)
#define PTSIZE (CTSIZE-crypto_box_MACBYTES)

#endif
