
[![Build Status](https://travis-ci.com/Fullaxx/authknock.svg?branch=master)](https://travis-ci.com/Fullaxx/authknock)

# AuthKnock
An authenticated encrypted port/protocol knock daemon

## Software
[libnet](https://github.com/libnet/libnet) \
[libpcap](https://www.tcpdump.org/) \
[libsodium](https://github.com/jedisct1/libsodium)

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
./compile.sh
```
