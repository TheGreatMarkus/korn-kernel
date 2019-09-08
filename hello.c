/**
 * @file    hello.c
 * @author  Cristian Aldea, Dan Seremet
 * @date    2 September 2019
 * @version 1.0
 * @brief  A simple LKM that sends a UDP packet
 * @see 
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fcntl.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <linux/ip.h>
#include <linux/udp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cristian Aldea, Dan Seremet");
MODULE_DESCRIPTION("A simple kernel module");
MODULE_VERSION("1.0");

unsigned int inet_addr(char *str);
int send_packet(struct net_device *dev, uint8_t dest_addr[ETH_ALEN], uint16_t proto);

static int __init LKM_init(void)
{  
   // Ethernet addresses
   static char addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
   uint8_t dest_addr[ETH_ALEN];
   memcpy(dest_addr, addr, ETH_ALEN);

   // Network Device
   struct net_device *enp0s3;
   enp0s3 = dev_get_by_name(&init_net, "enp0s3");

   // Protocol
   uint16_t proto;
   proto = ETH_P_IP;

   send_packet(enp0s3, dest_addr, proto);

   printk(KERN_INFO "Hello from KornKernel!\n");
   return 0;
}

static void __exit LKM_exit(void)
{
   printk(KERN_INFO "Goodbye from KornKernel!\n");
}

unsigned int inet_addr(char *str)
{
   int a, b, c, d;
   char arr[4];
   sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
   arr[0] = a;
   arr[1] = b;
   arr[2] = c;
   arr[3] = d;
   return *(unsigned int *)arr;
}

int send_packet(struct net_device *dev, uint8_t dest_addr[ETH_ALEN], uint16_t proto)
{
   int ret;
   unsigned char *data;

   // TODO: make these parameters
   char *srcIP = "127.0.0.1";
   char *dstIP = "192.168.0.200";
   char *data_string = "Test Message :)";
   int data_len = strlen(data_string);

   int udp_header_len = 8;
   int udp_total_len = udp_header_len + data_len;

   int ip_header_len = 20;
   int ip_payload_len = udp_total_len;
   int ip_total_len = ip_header_len + ip_payload_len;

   /* skb */
   struct sk_buff *skb = alloc_skb(ETH_HLEN + ip_total_len, GFP_ATOMIC); //allocate a network buffer
   skb->dev = dev;
   skb->pkt_type = PACKET_OUTGOING;
   skb_reserve(skb, ETH_HLEN + ip_header_len + udp_header_len); //adjust headroom
                                                                /* allocate space to data and write it */
   data = skb_put(skb, data_len);
   memcpy(data, data_string, data_len);

   /* UDP header */
   struct udphdr *uh = (struct udphdr *)skb_push(skb, udp_header_len);
   uh->len = htons(udp_total_len);
   uh->source = htons(15934);
   uh->dest = htons(15904);

   /* IP header */
   struct iphdr *iph = (struct iphdr *)skb_push(skb, ip_header_len);
   iph->ihl = ip_header_len / 4; //4*5=20 ip_header_len
   iph->version = 4;             // IPv4
   iph->tos = 0;
   iph->tot_len = htons(ip_total_len);
   iph->frag_off = 0;
   iph->ttl = 64;               // Set a TTL.
   iph->protocol = IPPROTO_UDP; //  protocol.
   iph->check = 0;
   iph->saddr = inet_addr(srcIP);
   iph->daddr = inet_addr(dstIP);

   /*changing Mac address */
   struct ethhdr *eth = (struct ethhdr *)skb_push(skb, sizeof(struct ethhdr)); //add data to the start of a buffer
   skb->protocol = eth->h_proto = htons(proto);
   skb->no_fcs = 1;
   memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
   memcpy(eth->h_dest, dest_addr, ETH_ALEN);

   skb->pkt_type = PACKET_OUTGOING;
   ret = dev_queue_xmit(skb);
   return 1;
}

/** @brief Using the "module_init()" and "module_exit()" macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function.
 */
module_init(LKM_init);
module_exit(LKM_exit);