# Statserve Project
Statserve is a simple webserver written in C that echos back statistics on processes and can be extended to use custom protocols. 

## Setup: 
Statserve requires one dependency which can be found at the following url: 

`https://github.com/spencedalley/liblcthw`

With the dependency installed, you can just clone the project, run `$ make` and statserve should be ready to use. 

## Usage:

####Running the server:

To start the server, just run: 
```
$ bin/statserve <host> <port>
```
Below shows an example connecting to localhost on port 8000. 

```
$ bin/statserve 127.0.0.1 8000
```

#### Connecting to the server:
Below is an example method of connecting to the server which is running on localhost port 8000.
```
$ nc 127.0.0.1 8000
```

#### Server protocols
This version of statserve has create, sample, update, and delete protocols implemented

Create protocol:
----------------------------------------
Create a new statistic instance on the server with the following command: 

```
$ create /<url_ending> <num> 
```
Sample protocol:
----------------------------------------
Fetch sample statistics from the server with the following command: 

```
$ sample /<url_ending> 
```

Update protocol:
----------------------------------------
Update sample statistics from the server with the following command: 

```
$ update /<url_ending> <num>
```

Delete protocol:
----------------------------------------
Delete sample statistics from the server with the following command: 

```
$ delete /<url_ending> 
```
## Future Development: 

Future development will focus on: 
* Creating a more robust parser for the protocol

## Running Tests: 
Tests are run with the following command: 
```
$ make

sh ./tests/runtests.sh
Running unit tests
-----
RUNNING: ./tests/statserve_tests
ALL TESTS PASSED
Tests run: 5
tests/statserve_tests PASS
```
