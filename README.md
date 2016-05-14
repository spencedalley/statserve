Statserve
========================================
A simple web server that can echo back statistics on data. 

Installation: 
========================================
To install liblcthw, download it, cd to the directory, and run the following commands: 

    $ ./configure
    $ make install


Usage: 
========================================

Statserve supports 2 simple protocols at the moment

Running the server:
----------------------------------------
To start the server, just run: 

	$ bin/statserve <host> <port>

Where <host> and <port> are the hostname and port that you want to connect to. An example is below: 

	$ bin/statserve 127.0.0.1 8000

This will set the server up locally and have it listen on port 8000. The server can be connected to with the nc command. 

Connecting to the server:
----------------------------------------
Below is an example method of connecting to the server, assuming it is running locally on port 8000: 

	$ nc 127.0.0.1 8000

Create protocol:
----------------------------------------
Create a new statistic instance on the server with the following: 

	$ create /<url_ending> <num> 

Sample protocol:
----------------------------------------
Fetch sample statistics from the server with the following: 

	$ sample /<url_ending> 

Future Development: 
========================================
The focus for future development is to make statserve support the following protocols: 

* delete <url>: Delete data from server
* update <url> <val>: Update url with val

Creating a more robust parser for the protocol

Running Tests: 
========================================
Tests are run with the following command: 

    $ make

