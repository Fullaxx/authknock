# AuthKnock [![Actions Status](https://github.com/Fullaxx/authknock/workflows/CI/badge.svg)](https://github.com/Fullaxx/authknock/actions)
An authenticated encrypted port/protocol knock daemon built from:
* [libnet](https://github.com/libnet/libnet)
* [libpcap](https://www.tcpdump.org/)
* [libsodium](https://github.com/jedisct1/libsodium)

AuthKnock will allow you to send/receive a single encrypted message that can be authenticated. \
This framework provides the ability to do one-way Command & Control.

## Requirements
First we need to make sure we have all the appropriate libraries. \
Please consult this chart for help with installing the required packages. \
If your OS is not listed, please help us fill out the table, or submit a request via github.

| OS     | Commands (as root)                                                                 |
| ------ | ---------------------------------------------------------------------------------- |
| Debian | `apt update; apt install -y build-essential libsodium-dev libpcap-dev libnet1-dev` |
| Fedora | `yum install -y gcc libsodium-devel libpcap-devel libnet-devel`                    |
| Ubuntu | `apt update; apt install -y build-essential libsodium-dev libpcap-dev libnet1-dev` |

## Compiling
```
$ cd src
$ ./compile.sh
```

## Creating keypairs
gen_keypair will create a single keypair. \
You will need 2 keypairs to send an authenticated message.
```
$ ./gen_keypair.exe server
Saved public key: server.pub
Saved secret key: server.key
$ ./gen_keypair.exe client
Saved public key: client.pub
Saved secret key: client.key
```

## IP knocking
ip_knock will send an encrypted message, wrapped in IP, with a protocol value of your choice. \
ip_receptor will listen for an encrypted message with a specific IP protocol value and authenticate it with keys provided. \
Upon authenticating the message, ip_receptor will by default run a system command on the message. \
You can change the default behavior by adjusting handle_payload() in payload.c \
root privileges are required to use libnet/libpcap.
```
# ./ip_receptor.exe           -p 255 --public client.pub --secret server.key
# ./ip_knock.exe -d 127.0.0.1 -p 255 --public server.pub --secret client.key -m ls
```
