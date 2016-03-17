# Traffic Generator
## Brief Introduction
A simple traffic generator for network experiments.

The **server** listens for incoming requests, and replies with a *flow* with the requested size (using the requested DSCP value & sending at the requested rate) for each request.

The **client** establishes *persistent TCP connections* to a list of servers and randomly generates requests over TCP connections according to the client configuration file. *If no available TCP connection, the client will establish a new one*. Currently, we provide two types of clients: **client** and **incast-client** for dynamic flow experiments. For **client**, each request only consists of one flow (fanout = 1). For **incast-client**, each request can consist of several synchronized *incast-like* flows. A request is completed only when all its flows are completed.  

In the **client configuration file**, the user can specify the list of destination servers, the Differentiated Services Code Point (DSCP) value distribution, the request fanout distribution, the request size distribution, the sending rate distribution, the desired RX throughput and the total numbers of requests. 

## Build
In the main directory, run ```make```, then you will see **client**, **incast-client**, **simple-client** (generate static flows for simple test) and **server** in ./bin.    

## Quick Start
In the main directory, do following operations:
- Start server 
```
nohup ./bin/server -p 5001 &
```

- Start client
```
./bin/client-c conf/example_config_1g.txt -l flows.txt -s 123 -r src/script/result.py
```

- Start incast-client
```
./bin/client-c conf/incast_example_config_1g.txt -l log -s 123 -r src/script/result.py
```

## Command Line Arguments
### Server
Example:
```
./bin/server -p 5001 
```
* **-p** : the TCP **port** that the server should listen on (default 5001)

* **-d** : **debug** mode (print necessary information)

* **-h** : display help information

### Client
Example:
```
./bin/client -c conf/example_config_1g.txt -l flows.txt -s 123 -r src/script/result.py
```
* **-c** : name of **configuration** file which specifies workload characteristics (required)

* **-l** : name of **log** file with flow completion times (default flows.txt)

* **-s** : seed value for random number generation (default current system time)

* **-r** : name of python script to parse **result** files

* **-d** : **debug** mode (print necessary information)

* **-h** : display help information

### Incast-Client
Example:
```
./bin/client -c conf/incast_example_config_1g.txt -l log -s 123 -r src/script/result.py
```

Same as **client** except for **-l**

* **-l** : **log** file name prefix (default log)<br>
The prefix is used for the two output files with flow and request completion times.

## Client Configuration File
The client configuration file specifies the list of servers, the Differentiated Services Code Point (DSCP) value distribution, the request fanout distribution (only for **incast-client**), the request size distribution, the sending rate distribution, the desired load and the total numbers of requests. We provide several client configuration files as examples in ./conf directory.  

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

* **fanout:** fanout value and weight. Note that only **incast-client** need this key. The fanout and weight are both 
integers.
```
fanout 1 50
fanout 2 30
fanout 8 20
```
For each request, the client chooses a fanout with a probability proportional to the weight. For example, with the above configuration, half the requests have fanout 1, and 20% have fanout 8. If the user does not specify the fanout distribution, the fanout size is always 1 for all requests.

* **dscp:** DSCP value and weight. The DSCP value and weight are both integers. Note that DSCP value should be smaller than 64.
```
dscp 0 25
dscp 1 25
dscp 2 25
dscp 3 25
```
For each request, the client chooses a DSCP value with a probability proportional to the weight. The traffic of the request and the corresponding response(s) will be marked with this DSCP value. If the user does not specify the DSCP distribution, the DSCP value is always 0 for all requests. 

* **rate:** sending rate and weight. The sending rate (in Mbps) and weight are both integers.
```
rate 0Mbps 10
rate 500Mbps 30
rate 800Mbps 60
```
For each request, the client chooses a rate with a probability proportional to the weight. To enforce the sending rate, the sender will add some delay on the application layer. Note that 0Mbps rate value indeed indicates no rate limiting. If the user does not specify the sending rate distribution, the sender will not rate-limit traffic.

* **load:** average RX throughput at the client in Mbps.
```
load 800Mbps
```
The client generates requests to roughly match the desired average throughput. In practice, the actual throughput can be slightly lower than desired (especially for **incast-client** due to connection establishment overhead). The client outputs the actual throughput upon termination.

* **num_reqs:** the total number of requests.
```
num_reqs 1500
```

##Output
A successful run of **client** creates a file with flow completion time results. A successful run of **incast-client** creates two files with flow completion time results and request completion time results, respectively. In results files, each line gives the request/flow size (in bytes), completion time (in microseconds), DSCP value and sending rate. You can directly use ./src/script/result.py to parse these files.        




