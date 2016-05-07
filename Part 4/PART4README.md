# Chirper Part 4

The not-so-original social media alternative, designed by **Wells Lucas Santo** and **Patrick Kingchatchaval**.

# How To Run The System

1. Open 6 tabs in terminal.

2. On tabs 1 - 5, navigate to directories `Server1`, `Server2`, `Server3`, `Server4`, and `Server5`.

3. Call `make run` in each of these 5 tabs. This will start up each of the data servers. Once running, a message will be printed notifying you which port each server is running on. We are currently using ports `9000`, `9100`, `9200`, `9300`, and `9400` on localhost for our 5 data servers.

4. On tab 6, run `python webserver.py`. This will start up the web server running Flask on `127.0.0.1:8000`.

5. Navigate to your favorite web browser and visit `127.0.0.1:8000` to access Chirper.

## Explanation Of How The Files Are Set Up

Since we now account for replicated data servers, running the entire system is a bit more involved than in the previous iterations of this project. First, notice that there are **five data servers** being used. These are split up to the directories `Server1`, `Server2`, `Server3`, `Server4`, and `Server5`. All of the code and the data files in these directories is intended to be the exact same, with the exception of the `makefile` in each directory. The idea here is that in a real world environment, these five directories would be split up across five different machines, and not the same machine. Each machine, of course, would thus need its own replica of all of the necessary files.

Now notice that `dataserver.cpp` in each directory is the same. This is because the port number being used by each data server *is not* hard-coded into this file. Instead, the `dataserver` executable must take a port number parameter in order to run. That is, if you wanted to run the data server on port `9000` without using the makefile, you would call `./dataserver 9000`. What each of the makefiles in each of the directories do is run the data server executable, with different port numbers specified. This way, you are able to just call `make run` in each directory, and have each data server run on a different port.

# Things That Are (And Are Not) Accounted For

1. If you're running multiple data servers at the moment, one or more data servers can crash (or in a controlled environment, can be killed) and the system will still work correctly. This is only true if at least one data server is left running. We do not account for the situation where no data servers are alive. (And this is not accounted for in Parts 2 and 3 either.)

2. You do not have to run all five data servers when starting the web server for the first time. You can choose to run at the very least one data server (this will emulate the behavior of Part 3). Our system will handle this correctly.

3. Although we do account for servers going down (or never starting up), we do not account for servers returning from the dead. That is, if the web server notices that a data server has gone down, it will no longer attempt to make connections to that data server. We do deal with data server crashes, but not data server recoveries.

4. We assume that when you start up the web server (and the data servers) for the first time, all of the data files in each of the five directories are exactly the same. (That is, we assume that all of the systems are consistent when first initialized.)

5. As always, our system is able to support multiple clients (aka users on multiple browsers), and we support the use of Chirper on various different browsers, operating systems, and screen resolutions.

# What's New In Part 4

1. Mutexes are no longer passed around between functions; instead, they are globals defined in the `queries.cpp` file (and declared using extern in the `queries.h` header, to follow separate compilation rules).

2. Dramatically cleaned up the function signatures in `queries.cpp` and `queries.h`. Previously, we were passing around all of the necessary mutexes, the file descriptor integer, and a `char[]` buffer to fill up and pass into the socket. All of these have been removed, and use of sockets has been completely decoupled with the actual query processing code. That is, the functions in `queries.cpp` are now no longer in charge of message passing and message coordination; they now receive a query in the form of a string and return a response in the form of a string. The queries and responses are now completely dealt with by the `processQuery()` function in `dataserver.cpp`. By decoupling these two bits of functionality, the query functions can focus exclusively on processing queries and obtaining locks, and the `processQuery()` function can focus on the actual message coordination.

3. **Active replication** has been implemented. Instead of only having one data server with one copy of all of the data files, we now make use of five data servers and five copies of the data. This, of course, can be scaled up or scaled down, and our system can handle anything from only one set of replicas to many more sets of replicas.

4. To facilitate active replication, the Python web server is now multithreaded to deal with each data server. Whenever a query is received by the Python web server, it will connect to the data servers that are alive. For each of these alive data servers that it connects to, a new thread is spun off to deal with coordinating the query and the response per data server. This allows for **simultaneous multicast and multi-receive** to/from each of the data servers, instead of sequentially polling each server.

5. To facilitate the possibility of multiple clients sending queries at the same time, and different data servers receiving these queries in different orders, we have made use of message numbers in order to enforce **total ordering** at all of the data servers.

6. In order to achieve our goals with active replication, simultaneous multicast/multireceive, and total ordering, we have also established a new Chirper protocol for application communication. This involves an initial query request from the web server to the data server, a response from the data server back to the web server, and a final acknowledgement from the web server back to the data server. This three-part communication process helps guarantee that our requirements for consistency, concurrency, low latency, and replication are met.

In the following section, I elaborate on how points 3 - 6 are accomplished.

# Active Replication

Our system utilizes active replication. This is based very closely on the conception of [State Machine Replication](https://en.wikipedia.org/wiki/State_machine_replication) as originally designed by Leslie Lamport in his paper "[Using Time Instead of Timeout for Fault-Tolerant Distributed Systems](http://research.microsoft.com/en-us/um/people/lamport/pubs/using-time.pdf)." Our system also closely follows the design of active replication as described in our textbook, section 18.3.2, whose naming conventions we use below.

This process is divided into roughly five parts.

1. *Connection*: When the web server is ready to make a query, it will attempt to connect to a list of ports where data servers are expected to be running. If it makes a successful connection, it adds that connection to a list of currently open sockets. If it finds that a data server cannot be connected to (meaning that the server has died), it will remove the corresponding port off of the access list, so that it does not check that dead server in the future. This is implemented mainly in `lines 40 - 50` in `webserver.py`.

2. *Request*: Once the live data servers are all connected to, we will perform a simultaneous multicast to all of the data servers. This is accomplished by spinning off a new thread per connection, as seen in `lines 58 - 61` in `webserver.py`. The threads will then concurrently engage in a three-part messaging process, of which this is the first part: the request (with a *request number*) is initially sent from the web server to *every* data server to be processed. The request is also added to a request queue at this time, which is used in the case of timeouts (explained in the next section). Finally, a `results` list is created at this point in order to store responses from each of the data servers. (The actual insertion into this list occurs in step 5.)

3. *Execution*: Since the data servers themselves are multithreaded, they can also concurrently handle multiple queries at once. This was the functionality implemented in Part 3 of the project. In order to ensure total ordering across all of the data servers, we make use of *condition variables* and the *request number* to ensure that requests are processed in the same, correct order. If the data server receives a request number that is too high, that thread will use the condition variable and yield, which is done on `line 85` of `dataserver.cpp`. If the data server receives the expected request number, it will process the query and generate a response (with a response number matching the request number). The response will be stored in a response queue at this time (to deal with timeouts; next section). The data servers will then send the response over the connection back to the web server, which is the second step of the three-part messaging process.

4. *Agreement*: By relying on the reliability of TCP, we know that all active data servers will have received the same query, and because of the total ordering mechanism, all data servers will respond with the same response for a given request number. Since the active replication approach is one in which *all* active data servers are sent all of the queries, there do not need to be additional coordination steps between the data servers to ensure consistency. Due to the use of the condition variable, we are guaranteed consistent states for the same request number that is processed on different data servers. (Timeouts and hanging servers will be discussed in the next section.)

5. *Acknowledgement*: Once the data servers have processed the queries and returned responses, the threads of the web server will concurrently look at each response and ensure that an ACK is returned to each data server for that given request/response number. This ACK is the third step in the three-part messaging process. This allows the data server to be certain that the web server has received the response up to the ACKed number. Each thread will then obtain a lock in order to access the `results` list, to insert the response received from the particular data server it has connected to. Each thread will then finish, and afterwards, the main calling thread will inspect the `results` list to get the correct response to use for the current query.

# The Timeout Situation

In addition to what is described above, we deal with ensuring timely operation of the system with low latencies by enforcing timeouts with the connections that each thread of the web server makes with the data server. This is to ensure that the web server will never hang, and will continue to proceed even if one or more data servers are hanging.

To begin, one large advantage of our system is the use of the multithreaded simultaneous multicast/multireceive to deal with each data server. Our system will not wait for each data server to respond sequentially; rather, we send and receive concurrently. Even better, we enforce a timeout such that there is a maximum duration of time in which we will wait for any data server to respond.

In the case of data servers timing out, we have a contingency plan that deals with data servers who may not be up-to-date, or may be hanging. First, we make use of the request numbers in order to check what queries have been recived by which data servers. We will make use of the request queue (as described in part two above) in order to keep track of all the queries (just strings) that have not been ACKed/responded to by all data servers. When we receive a new response from a data server, we will check its response number and update the latest message ACKed by that data server to reflect that new response. In this way, we can update the request queue to only store messages that some data servers may not have seen yet.

When a connection is opened, and before the web server sends the most recent request to a data server, the web server checks to see if there are still requests that it still does not think the data server has seen. In this way, if a timeout did occur, then the data server will be able to process any requests that it is missing (since it must process requests in order).

But what if the data server already processed a request, but the connection timed out before the web server could receive the response? In this situation, this is why we also make use of a response queue on the data server side, as described in step 3 above. This step is powerful, since if the web server's connection times out, but the data server is still processing the current query, we do not lose the work that the data server has performed. Rather, the next time the web server makes a connection to the data server, the data server will already have the response queued up and ready to reply to the web server. In the event that the data server receives a request it has already processed, it will *not* reprocess the request (since not all requests are idempotent). Instead, since we already know what response to return, we will merely return the response and not act on the duplicate request.

Finally, this is where the purpose of the final ACK from the web server back to the data server is used for. In order for the data server to not have to keep track of every possible response it has made so far, this ACK is used in order to check if the web server actually received the response or not. If the web server receives this ACK, it is guaranteed that the data server has processed that response, and the web server no longer needs to remember it. If this ACK is not received, the data server merely holds on to the response, in the case that the web server did not hear the response due to a timeout.

As such, we deal with timeouts by using a request queue on the web server side and a response queue on the data server side. Each are maintained by making use of the three-step communication process and request/response numbers. By allowing for timeouts, we ensure that even if one or more data servers hang, the threads of the web server will all eventually complete, and the web server itself will not hang. (In fact, so long as at least one data server response, the web server will always have a response to issue back to the client, thus allowing for very low latency responses even in a replicated system.)
