# Private Information Retrieval

In most applications servers store information of requests made by clients. While often these logs are only used for analysis of traffic, they can be used to track all data retrieved by a certain client. Private Information Retrieval is a system which enables client to retrieve data from servers without the server knowing which data the client is interested in.

## How does this work?

One simple way of achieving private information retrieval is to have the server simply send all messages to the client. This however is not desired as the amount of network traffic needed is the number of bytes of data the server stores (aka way too much). In an effort to reduce the number of bytes sent the client request the product of the xor operation of a random number of messages the server stores. By requesting the same xor product excluding the desired message from a different server the two results, after performing the xor operation again, return the desired message.

An in depth description can be found [here](https://dl.acm.org/doi/pdf/10.1145/293347.293350) or (in video form) [here](https://media.ccc.de/v/36c3-10565-what_s_left_for_private_messaging#t=2402).

## Installation

Follow the these steps:

```
git clone https://github.com/idahoenigmann/private_information_retrieval.git
cd private_information_retrieval/
mkdir build && cd build
cmake ..
make
```

To test communication try:

```
./server -p 1234 -f ../src/server/data.txt &
./server -p 1235 -f ../src/server/data.txt &
./client -i 0
```

To get more information on the command line parameters type:

```
./client --help
./server --help
```
