#include <stdio.h>
#include <arpa/inet.h>
#include <pcap.h>

#include "payload.h"

extern int g_shutdown;
extern uint16_t g_proto;
extern int g_linktype;

#include <netinet/ip.h>
#define SIZE_IPV4 (sizeof(struct ip))
static void process_ipv4(const u_char *buf, int len)
{
#ifdef USE_IPHDR
	struct iphdr *ip4 = (struct iphdr *)buf;
#else
	struct ip *ip4 = (struct ip *)buf;
#endif
	unsigned char hl;
	unsigned short tl;
	unsigned char proto;
	char src_addr[INET_ADDRSTRLEN];
	char dst_addr[INET_ADDRSTRLEN];

	if(len < SIZE_IPV4) { return; }

#ifdef USE_IPHDR
	hl = ip4->ihl << 2;
	tl = ntohs(ip4->tot_len);
	proto = ip4->protocol;
	inet_ntop(AF_INET, &ip4->saddr, &src_addr[0], INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &ip4->daddr, &dst_addr[0], INET_ADDRSTRLEN);
#else
	hl = ip4->ip_hl << 2;
	tl = ntohs(ip4->ip_len);
	proto = ip4->ip_p;
	inet_ntop(AF_INET, &ip4->ip_src, &src_addr[0], INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &ip4->ip_dst, &dst_addr[0], INET_ADDRSTRLEN);
#endif

	if(proto != g_proto) {
		fprintf(stderr, "WRONG PROTO - DID THE BPF FAIL US??\n");
		g_shutdown = 1;
		return;
	}

	printf("%15s -> %15s %4u %3u ", &src_addr[0], &dst_addr[0], tl, proto);

	buf += hl;
	len -= hl;

	process_payload(buf, len);
	printf("\n");
}

#include <net/ethernet.h>
#define SIZE_ETHERNET (sizeof(struct ether_header))
static void process_eth(const u_char *buf, int len)
{
	unsigned short *pp, proto;

	if(len < SIZE_ETHERNET) { return; }

	pp = (unsigned short *) (buf+12);
	proto = ntohs(*pp);

	buf += SIZE_ETHERNET;
	len -= SIZE_ETHERNET;

	if(proto == ETHERTYPE_IP) {
		process_ipv4(buf, len);
	}
}

#include <pcap/sll.h>
#define SIZE_SLL (sizeof(struct sll_header))
static void process_sll(const u_char *buf, int len)
{
	struct sll_header *sll = (struct sll_header *)buf;
	unsigned short type, proto;

	if(len < SIZE_SLL) { return; }

	//we are only concerned with packets coming to us
	type = ntohs(sll->sll_pkttype);
	if(type != LINUX_SLL_HOST) { return; }

	proto = ntohs(sll->sll_protocol);
	buf += SIZE_SLL;
	len -= SIZE_SLL;

	if(proto == ETHERTYPE_IP) {
		process_ipv4(buf, len);
	}
}

void handle_packet(u_char *user, const struct pcap_pkthdr *hdr, const u_char *pkt)
{
	switch(g_linktype) {
		case DLT_EN10MB:				//  1
			process_eth(pkt, hdr->len);
			break;
		case DLT_PPP:					//  9
			//process_ppp(pkt_env->buf, pkt_env->size);
			printf("Linktype PPP: (not handled yet)\n");
			break;
		case DLT_RAW:					// 12
			process_ipv4(pkt, hdr->len);
			break;
		case DLT_LINUX_SLL:				//113
			process_sll(pkt, hdr->len);
			break;
		case DLT_PRISM_HEADER:			//119
			printf("Linktype PRISM: (not handled yet)\n");
			break;
		case DLT_AIRONET_HEADER:		//120
			printf("Linktype AIRONET: (not handled yet)\n");
			break;
		case DLT_IEEE802_11:			//127
			//process_80211(pkt_env->buf, pkt_env->size);
			printf("Linktype 802.11: (not handled yet)\n");
			break;
		case DLT_IEEE802_11_RADIO_AVS:	//163
			//process_80211_avs(pkt_env->buf, pkt_env->size);
			printf("Linktype 802.11 AVS: (not handled yet)\n");
			break;
		case DLT_IPV4:					//228
			process_ipv4(pkt, hdr->len);
			break;
		case DLT_IPV6:					//229
			//process_ipv6pkt_env->buf, pkt_env->size);
			printf("Linktype IPv6: (not handled yet)\n");
			break;
#ifdef DLT_SCTP
		case DLT_SCTP:					//248
			//process_sctp(pkt_env->buf, pkt_env->size);
			printf("Linktype SCTP: (not handled yet)\n");
			break;
#endif
		default:
			printf("Linktype %d: Unknown\n", g_linktype);
	}
}
