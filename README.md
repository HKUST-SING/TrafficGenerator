# Traffic Generator
## Brief Introduction
A simple traffic generator for network experiments.

The server listens for incoming requests, and reples with a *flow* with the requested size (using the requested DSCP value / sending at the requested rate) for each request.

The client establishes persistent TCP connections to a list of servers and randomly generates requsts acccording to the configureation file. Users can specify destination servers, DSCP distributions, fanout distributions, request size dsitributions, sending rate distributions, desired loads and numbers of requests. Currently, we provide two types of clients: *client* and *incast-client*. For *client*, each requess only consists of one flow (fanout = 1). For *incast-client*, each request can consist of several synchronized *incast-like* flows. A request is completed only when all its flows are completed.   



