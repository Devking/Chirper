# Chirper

The not-so-original social media alternative, designed by **Wells Lucas Santo** and **Patrick Kingchatchaval**.

## Table of Contents

1. <a href="#one">The Application Itself</a><br />
	1.1. <a href="#oneone">Dependencies</a><br />
	1.2. <a href="#onetwo">Running the Application</a><br />
	&nbsp;&nbsp;&nbsp;1.2.1. <a href="#onetwoone">Running the Application Responsively</a><br />
	1.3. <a href="#onethree">Dummy Example Data</a><br />
2. <a href="#two">Under the Hood</a><br />
	2.1 <a href="#twoone">Data Files</a><br />
	&nbsp;&nbsp;&nbsp;2.1.1. <a href="#twooneone">User Data File</a><br />
	2.2 <a href="#twotwo">Queries</a><br />
	2.3 <a href="#twothree">Query API</a><br />

*These links only work if you are viewing the Markdown page in a browser.*

# 1. <span id="one">The Application Itself</span>

## 1.1 <span id="oneone">Dependencies</span>

* Python 2.7.11
* Python's Flask Package
* jQuery (via [Google's Hosted Library](https://developers.google.com/speed/libraries/))
* C++11
* UNIX Socket Libraries

Because the data server uses UNIX socket libraries, it must be run from a **nix* environment.

This code was tested on both Mac OS X Yosemite and Ubuntu 14.04, using `g++ -std=c++11`.

## 1.2. <span id="onetwo">Running the Application</span>

This application requires that you run both the data server and the web server concurrently. You must first run the data server *before* running the web server.

To run the data server, run the following in the terminal:

`make run`

This command is defined in the `makefile` within this directory. It compiles the data server (written in C++11), creates an executable, and runs it. The data server will run on `localhost:9000`.

Afterwards, start the web server by running the following in terminal:

`python main.py`

This will run the web server on `localhost:8000`. To access the application, open your favorite browser and visit the following address:

`http://127.0.0.1:8000`

If you wish to stop the servers, it's safer to close the web server first, as that will close the data server as well.

### 1.2.1. <span id="onetwoone">Running the Application Responsively</span>

This application also incorporates *responsive design* &ndash; if you run the *web server* on your public IP, you can access the application on a mobile device as well.

The front-end of the application has been tested on the latest versions of Chrome, Safari, Firefox, and Opera, on Ubuntu, Mac OSX, Windows, iOS, and Android.

## 1.3. <span id="onethree">Dummy Example Data</span>

All data about user information and chirps are stored in text files, located within the `manifest` and `users` directories. Currently, we have two dummy accounts already created that you can access:

	username: admin
	password: pass

and

	username: bob
	password: dole

Because all data is stored in text files, the state of the application will be saved between executions of the server.**

# <span class="#two">Under the Hood</span>

## <span class="#twoone">Data Files</span>

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

```
pass
admin@admin.com
1
bob
2
Hello there!
This is a data file.
```

## Queries

In order for the Python web server to communicate with the C++11 data server, messages are passed between the two servers via TCP/IPv4 sockets.

When the Python web server begins, it establishes a *single persistent connection* to the C++11 data server. Our application will persist this connection as long as both servers are running, and send all messages (which are unencrypted at the moment) over this single connection.

All queries and responses between the two servers follow the specific format of our Chirper API, which is described below.

## Query API

13 queries currently exist in our system. They are numbered as follows:


Query No | Query Code | Description
------------------------------------
       1 |     CHKEML | Check E-mail
       2 |     CHKUSR | Check User
       3 | CHKPWD | Check Password
4 | CHKFND | Check Friend
 5 | CRTUSR | Create User
 6 | DELUSR | Delete User
 7 | CRTCHP | Create Chirp
 8 | DELCHP |  Delete Chirp
 9 | ADDFND |  Add Friend
10 | DELFND |  Delete Friend
11 | POPLAT |  Populate Page
12 | MOVEUP |  Move Friend Up
13 | MOVEDN | Move Friend Down


-------

- check email                           CHKEML  email                           1 - done
- check username                        CHKUSR  username                        2 - done
- check password                        CHKPWD  username\npassword              3 - done
- check friend                          CHKFND  username\nfriend                4 - done
- create user                           CRTUSR  username\npassword\nemail       5 - done
- delete user                           DELUSR  username                        6 - done
- create chirp                          CRTCHP  username\nchirp                 7 - done
- delete chirp                          DELCHP  username\nchirpid               8 - done
- add friend                            ADDFND  username\nfriend                9 - done
- delete friend                         DELFND  username\nfriend                10 - done
- populate                              POPLAT  username                        11 - done
- reorder friends                       MOVEUP  username\nfriendid              12 - done
                                        MOVEDN  username\nfriendid              13 - done
first field: action
space
second field: username/email
newline
optional data field: password, email, friend name, chirp id, friend id
