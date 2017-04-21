# Traffic Generator
## Brief Introduction
A simple traffic generator for network experiments.

The **server** listens for incoming requests, and replies with a *flow* with the requested size (using the requested DSCP value & sending at the requested rate) for each request.

The **client** establishes *persistent TCP connections* to a list of servers and randomly generates requests over TCP connections according to the client configuration file. *If no available TCP connection, the client will establish a new one*. Currently, we provide two types of clients: **client** and **incast-client** for dynamic flow experiments. For **client**, each request only consists of one flow (fanout = 1). For **incast-client**, each request can consist of several synchronized *incast-like* flows. A request is completed only when all its flows are completed.  

In the **client configuration file**, the user can specify the list of destination servers, the request size distribution, the Differentiated Services Code Point (DSCP) value distribution, the sending rate distribution and the request fanout distribution, . 

## Build
In the main directory, run ```make```, then you will see **client**, **incast-client**, **simple-client** (generate static flows for simple test), **server** and some python scripts in ./bin.    

## Quick Start
In the main directory, do following operations:
- Start server 
```
./bin/server -p 5001 -d
```

- Start client
```
./bin/client -b 900 -c conf/client_config.txt -n 5000 -l flows.txt -s 123 -r bin/result.py
```

- Start incast-client
```
./bin/incast-client -b 900 -c conf/incast_client_config.txt -n 5000 -l log -s 123 -r bin/result.py
```

## Command Line Arguments
### Server
Example:
```
./bin/server -p 5001 -d  
```
* **-p** : the TCP **port** that the server listens on (default 5001)

* **-v** : give more detailed output (**verbose**)

* **-d** : run the server as a **daemon**

* **-h** : display help information

### Client
Example:
```
./bin/client -b 900 -c conf/client_config.txt -n 5000 -l flows.txt -s 123 -r bin/result.py
```
* **-b** : desired average RX **bandwidth** in Mbits/sec
 
* **-c** : **configuration** file which specifies workload characteristics (required)

* **-n** : **number** of requests (instead of -t)

* **-t** : **time** in seconds to generate requests (instead of -n)
 
* **-l** : **log** file with flow completion times (default flows.txt)

* **-s** : **seed** to generate random numbers (default current system time)

* **-r** : python script to parse **result** files

* **-v** : give more detailed output (**verbose**)

* **-h** : display **help** information

Note that you need to specify either the number of requests (-n) or the time to generate requests (-t). But you cannot specify both of them.

### Incast-Client
Example:
```
./bin/incast-client -b 900 -c conf/incast_client_config.txt -l log -s 123 -r bin/result.py
```

Same as **client** except for **-l**

* **-l** : **log** file name prefix (default log)<br>
The prefix is used for the two output files with flow and request completion times.

## Client Configuration File
The client configuration file specifies the list of servers, the request size distribution, the Differentiated Services Code Point (DSCP) value distribution, the sending rate distribution and the request fanout distribution (only for **incast-client**). We provide several client configuration files as examples in ./conf directory.  

The format is a sequence of key and value(s), one key per line. The permitted keys are:

* **server:** IP address and TCP port of a server.
```
server 192.168.1.51 5001
```

* **req_size_dist:** request size distribution file path and name.
```
req_size_dist conf/DCTCP_CDF.txt
```
There must be one request size distribution file, present at the given path, 
which specifies the CDF of the request size distribution. See "DCTCP_CDF.txt" in ./conf directory 
for an example with proper formatting.

* **dscp:** DSCP value and weight. The DSCP value and weight are both integers. Note that DSCP value should be smaller than 64.
```
dscp 0 25
dscp 1 25
dscp 2 25
dscp 3 25
```
For each request, the client chooses a DSCP value with a probability proportional to the weight. The traffic of the request and the corresponding response(s) will be marked with this DSCP value. If the user does not specify the DSCP distribution, the DSCP value is always 0 for all requests. **This feature can be used to create multiple classes of traffic (e.g., section 5.1.2 of [MQ-ECN](https://www.usenix.org/system/files/conference/nsdi16/nsdi16-paper-bai.pdf) paper).**  

* **rate:** sending rate and weight. The sending rate (in Mbps) and weight are both integers.
```
rate 0Mbps 10
rate 500Mbps 30
rate 800Mbps 60
```
For each request, the client chooses a rate with a probability proportional to the weight. To enforce the sending rate, the sender will add some delay at the application layer. *Note that 0Mbps indicates no rate limiting.* If the user specifies very low sending rates, the client may achieve a much lower average RX throughput in practice, which is undesirable. If the user does not specify the sending rate distribution, the sender will not rate-limit the traffic. **We suggest the user simply disabling this feature except for some special scenarios.**   

* **fanout:** fanout value and weight. Note that only **incast-client** need this key. The fanout and weight are both 
integers.
```
fanout 1 50
fanout 2 30
fanout 8 20
```
For each request, the client chooses a fanout with a probability proportional to the weight. For example, with the above configuration, half the requests have fanout 1, and 20% have fanout 8. If the user does not specify the fanout distribution, the fanout size is always 1 for all requests.

##Output
A successful run of **client** creates a file with flow completion time results. A successful run of **incast-client** creates two files with flow completion time results and request completion time results, respectively. You can directly use ./bin/result.py to parse these files. 

In files with flow completion times, each line gives flow size (in bytes), flow completion time (in microseconds), DSCP value, desired sending rate (in Mbps) and actual per-flow goodput (in Mbps). 

In files with request completion times, each line gives request size (in bytes), request completion time (in microseconds), DSCP value, desired sending rate (in Mbps), actual per-request goodput (in Mbps) and request fanout size.

##Contact
For questions, please contact Wei Bai (http://sing.cse.ust.hk/~wei/).

##Publications
- Paper on Traffic Generator: please use the citation below as the reference to Traffic Generator
```
Enabling ECN in Multi-Service Multi-Queue Data Centers
Wei Bai, Li Chen, Kai Chen, Haitao Wu
USENIX NSDI 2016

@inproceedings {194968,
    author = {Wei Bai and Li Chen and Kai Chen and Haitao Wu},
    title = {Enabling ECN in Multi-Service Multi-Queue Data Centers},
    booktitle = {13th USENIX Symposium on Networked Systems Design and Implementation (NSDI 16)},
    year = {2016},
    month = Mar,
    isbn = {978-1-931971-29-4},
    address = {Santa Clara, CA},
    pages = {537--549},
    url = {https://www.usenix.org/conference/nsdi16/technical-sessions/presentation/bai},
    publisher = {USENIX Association},
}
```

- Papers that use Traffic Generator:
```
ClickNP: Highly Flexible and High-performance Network Processing with Reconfigurable Hardware
Bojie Li, Kun Tan, Larry Luo, Renqian Luo, Yanqing Peng, Ningyi Xu, Yongqiang Xiong, Peng Cheng, Enhong Chen
ACM SIGCOMM 2016

Fast and Cautious: Leveraging Multi-path Diversity for Transport Loss Recovery in Data Centers
Guo Chen, Yuanwei Lu, Yuan Meng, Bojie Li, Kun Tan, Dan Pei, Peng Cheng, Larry Luo, Yongqiang Xiong, Xiaoliang Wang, Youjian Zhao
USENIX ATC 2016
```






