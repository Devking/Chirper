# Chirper

The not-so-original social media alternative, designed by **Wells Lucas Santo** and **Patrick Kingchatchaval**.

## Table of Contents

1. The Application Itself

	1.1. Dependencies

	1.2. Running the Application

	&nbsp;&nbsp;&nbsp;1.2.1. Running the Application Responsively

	1.3. Dummy Example Data

2. Under the Hood

	2.1 Data Files

	&nbsp;&nbsp;&nbsp;2.1.1. User Data File

	2.2 Queries

	2.3 Query API

	&nbsp;&nbsp;&nbsp;2.3.1 Query Descriptions

*The links above only work in certain versions of Markdown.*

# 1. The Application Itself

## 1.1 Dependencies

* Python 2.7.11
* Python's Flask Package
* jQuery (via [Google's Hosted Library](https://developers.google.com/speed/libraries/))
* C++11
* UNIX Socket Libraries

Because the data server uses UNIX socket libraries, it must be run from a **nix* environment.

This code was tested on both Mac OS X Yosemite and Ubuntu 14.04, using `g++ -std=c++11`.

## 1.2. Running the Application

This application requires that you run both the data server and the web server concurrently. You must first run the data server *before* running the web server.

To run the data server, run the following in the terminal:

`make run`

This command is defined in the `makefile` within this directory. It compiles the data server (written in C++11), creates an executable, and runs it. The data server will run on `localhost:9000`.

Afterwards, start the web server by running the following in terminal:

`python main.py`

This will run the web server on `localhost:8000`. To access the application, open your favorite browser and visit the following address:

`http://127.0.0.1:8000`

If you wish to stop the servers, it's safer to close the web server first, as that will close the data server as well.

### 1.2.1. Running the Application Responsively

This application also incorporates *responsive design* &ndash; if you run the *web server* on your public IP, you can access the application on a mobile device as well.

The front-end of the application has been tested on the latest versions of Chrome, Safari, Firefox, and Opera, on Ubuntu, Mac OSX, Windows, iOS, and Android.

## 1.3. Dummy Example Data

All data about user information and chirps are stored in text files, located within the `manifest` and `users` directories. Currently, we have two dummy accounts already created that you can access:

	username: admin
	password: pass

and

	username: bob
	password: dole

Because all data is stored in text files, the state of the application will be saved between executions of the server.

# Under the Hood

## Data Files

Our data server uses two primary text files, and then one text file per user. The file structure is as follows:

	Chirper
	|     (server files)
	|     manifest
	      |    user.txt
	      |    email.txt
	|     users
	      |    admin.txt
	      |    bob.txt
	      |   (other user files)

Description of Files:

- `user.txt`: Contains a list of all the users in the system, separated by commas.
- `email.txt`: Contains a list of all the emails in the system, separated by commas.
- `[username].txt`: Contains information about `[username]`. See section below.

Due to formatting specifications, **DO NOT manually modify any of these text files**, which may prevent the system from working correctly.

### User Data File

The formatting of the user data file is as follows:

	[password]
	[email]
	[# of friends]
	[friend 1]
	[friend 2]
	[...]
	[# of chirps]
	[chirp 1]
	[chirp 2]
	[...]

A single newline (`\n`) separates each piece of information in the user data file. Below is an example of what a user data file `admin.txt` may look like:

	pass
	admin@admin.com
	1
	bob
	2
	Hello there!
	This is a data file.

## Queries

In order for the Python web server to communicate with the C++11 data server, messages are passed between the two servers via TCP/IPv4 sockets.

When the Python web server begins, it establishes a *single persistent connection* to the C++11 data server. Our application will persist this connection as long as both servers are running, and send all messages (which are unencrypted at the moment) over this single connection.

All queries and responses between the two servers follow the specific format of our Chirper Query API, which is described below.

## Query API

A query is described as any message sent from the Python web server to the C++11 data server. All queries will expect at least one response message from the data server.

13 queries currently exist in our system. They are described as follows:

	Query No | Query Code | Description
	----------------------------------------
	       1 |     CHKEML | Check E-mail
	       2 |     CHKUSR | Check User
	       3 |     CHKPWD | Check Password
	       4 |     CHKFND | Check Friend
	       5 |     CRTUSR | Create User
	       6 |     DELUSR | Delete User
	       7 |     CRTCHP | Create Chirp
	       8 |     DELCHP | Delete Chirp
	       9 |     ADDFND | Add Friend
	      10 |     DELFND | Delete Friend
	      11 |     POPLAT | Populate Page
	      12 |     MOVEUP | Move Friend Up
	      13 |     MOVEDN | Move Friend Down

All queries are represented by a 6-character query code. Within `api_mapping.h`, we translate each query code to a query number, which our `server.cpp` script uses for a switch statement to decide what query to process.

All queries follow the same format, which is represented below:

	[Query Code] [Main Field]
	[Optional Data Field]
	[Optional Data Field 2]

The `[Main Field]` is a single parameter that every query must include. Some queries may pass extra data (described above as `[Optional Data Field]`, which is sent after a newline (`\n`).

### Query Descriptions

Below, each query is described in further detail.

**1 CHKEML Check E-mail**

*Checks if an e-mail already exists or not.*

Query Format:

	CHKEML [email]

Response Format:

	[YES or NO]

**2 CHKUSR Check Username**

*Checks if a user already exists or not.*

Query Format:

	CHKUSR [username]

Response Format:

	[YES or NO]

**3 CHKPWD Check Password**

*Check if a given password is valid for a given user.*

Query Format:

	CHKPWD [username]
	[password]

Response Format:

	[YES or NO]

**4 CHKFND Check Friend**

*Checks if a friend is already on this users' friend list.*

Query Format:

	CHKFND [username]
	[friendname]

Response Format:

	[YES or NO]

**5 CRTUSR Create User**

*Create a new user. Assumes existence checks have been performed separately.*

Query Format:

	CRTUSR [username]
	[password]
	[email]

**6 DELUSR Delete User**

*Delete a user.*

Query Format:

	DELUSR [username]

**7 CRTCHP Create Chirp**

*Create a chirp.*

Query Format:

	CRTCHP [username]
	[chirp text]

**8 DELCHP Delete Chirp**

*Delete a chirp.*

Query Format:

	DELCHP [username]
	[chirpid]

Note: chirpid corresponds to the index of the chirp on the page.

**9 ADDFND Add Friend**

*Add a friend to the friend's list.*

Query Format:

	ADDFND [username]
	[friendname]

**10 DELFND Delete Friend**

*Delete a friend from the friend's list.*

Query Format:

	DELFND [username]
	[friendname]

**11 POPLAT Populate Page**

*Populate a user's main page with all chirps and friends.*

Query Format:

	POPLAT [username]

Response Format:

	[email]
	[# of friends]
	[each friend name]
	[# of chirps]
	[each chirp]
	[# of chirps of friend 1]
	[friend 1's chirps]
	[# of chirps of friend 2]
	[friend 2's chirps]
	[...]

**12 MOVEUP Move Friend Up**

*Move a friend upwards on the friend's list.*

Query Format:

	MOVEUP [username]
	[friendid]

Note: friendid corresponds to the index of the friend on the page.

**13 MOVEDN Move Friend Down**

*Move a friend downwards on the friend's list.*

Query Format:

	MOVEDN [username]
	[friendid]

Note: friendid corresponds to the index of the friend on the page.
