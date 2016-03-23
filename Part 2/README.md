First:

`make run`

Then:

`python main.py`

Do not close either server when accessing the browser platform.

----

DO NOT MODIFY `user.txt`, `email.txt`, OR ANY OF THE USER FILES! (Any change in formatting may prevent the system from working.)



1 file for list of users
1 file for each user
1 file for list of emails

------

we do a file read on *every* request, because
we do not want to store all of the user data
in memory and potentially cause a stack overflow!

------

no encryption

------

queries:

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

------

email file: csv of emails
	- emails only separated by commas, no spaces
	- need comma after last email as well!

------

user files will be in the 'users' directory
each users' file:

password\n
email\n
# of friends\n
friend1\n
friend2\n
# of chirps\n
chirp1\n
chirp2\n
