#ifndef __DISSECT_PACKET_H__
#define __DISSECT_PACKET_H__

#include <pcap.h>

void handle_packet(u_char *, const struct pcap_pkthdr *, const u_char *);

#endif
