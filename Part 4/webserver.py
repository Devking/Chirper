# CS3254 Parallel and Distributed Systems
# Code Written By: Wells Santo and Patrick Kingchatchaval

# Python Web server runs on localhost:8000
# C++ Data servers run on localhost, ports 9000, 9100, 9200, 9300, 9400

from flask import Flask, render_template, redirect, request, url_for, session
import socket
import select
import threading

# Define the address and ports of our data servers
host = 'localhost'
ports = [9000, 9100, 9200, 9300, 9400]

# Keep track of the latest message ACKed by each data server
portacks = [0, 0, 0, 0, 0]
messqueue = []
msgnum = 0

# A lock for accessing the 'results' list
resultslock = threading.Lock()

# Send a message through a socket and receive a response
def socketsendrecv(sendmsg):
    # Increment current message number
    global msgnum
    msgnum = msgnum + 1
    # Append message number to the front of the message to send
    newmsg = str(msgnum) + '\n' + sendmsg
    # Add current message to list of un-acked messages, in case we need to resend
    messqueue.append((msgnum, newmsg))
    # Update list of un-acked messages to remove messages that everyone has acked
    minack = min(portacks)
    while len(messqueue) > 0 and messqueue[0][0] <= minack:
        messqueue.pop(0)
    # Connect to sockets based on 'ports' list & remove dead servers from ports list
    sockets = []
    i = 0
    for i in xrange(len(ports) - 1, -1, -1):
        try:
            s = socket.socket()
            # Enforce timeouts for the socket!
            s.settimeout(1)
            s.connect((host, ports[i]))
            sockets.append(s)
        except IOError:
            print 'Could not connect to web server at port', ports[i]
            del ports[i]
            del portacks[i]
    # Do the simultaneous, multithreaded multicast/multireceive
    return multicast(sockets, newmsg, msgnum)

# Spin off threads to send the query to each data server
def multicast(sockets, newmsg, msgnum):
    threads = []
    results = []
    for i in range(len(sockets)):
        t = threading.Thread(target = singlesendrecv, args = (sockets[i], i, newmsg, msgnum, results))
        threads.append(t)
        t.start()
    # Since we're working with timeouts, all of the threads will run and do work concurrently
    # We're guaranteed not to get stuck here due to enforced timeouts
    for i in range(len(threads)):
        threads[i].join()
    # Assumes that at least one server returned the right answer
    return results[0]

# Deal with one socket/server in the multicast/multireceive
def singlesendrecv(thesocket, i, newmsg, msgnum, results):
    # If this server did not ACK the latest queries, we need to resend them
    if portacks[i] + 1 < msgnum:
        messageDic = dict(messqueue)
        for j in range(portacks[i]+1, msgnum):
            queryj = messageDic.get(j)
            response = threewayshake(thesocket, queryj, i)
            # If we get a timeout in here, we shouldn't continue
            if response == '':
                return
    # If the ACKing is up-to-date, then send the new query
    response = threewayshake(thesocket, newmsg, i)
    if response == '':
        return
    # Finally, append received message to the 'results' list
    resultslock.acquire()
    results.append(response)
    resultslock.release()

# Deal with the actual send-recv-send message passing
def threewayshake(thesocket, msg, i):
    # Send the query to the data server
    thesocket.sendall(msg)
    returnstr = ''
    # Wait for a response from the data server; time-out is enabled
    try:
        nextrecvstr = thesocket.recv(4096)
        while nextrecvstr != '':
            returnstr += nextrecvstr
            # Check if the terminating string was received
            # This string will tell us to stop expecting messages, and
            # process the query immediately, without waiting for the timeout any more
            checkterminal = returnstr.split('\n\n\nR')
            if len(checkterminal) < 2 or checkterminal[1] != 'END':
                nextrecvstr = thesocket.recv(4096)
            else:
                returnstr = checkterminal[0]
                break
    # If time out occurs, then we know the data server was stalled
    # We will simply close the socket and query the data server again in the next connection
    except socket.timeout:
        print 'Got a timeout!'
        thesocket.close()
        return ''
    # Get the message number out of the data server's response
    messagenumber = returnstr.split('\n')[0]
    # Based on the received message number, update the lowest message number seen
    # Notice that the total ordering on the data server side means this will never skip numbers
    returnstr = '\n'.join(returnstr.split('\n')[1:])
    portacks[i] = max(portacks[i], int(messagenumber))
    # Return an ACK for the received message and then close the socket
    thesocket.sendall(messagenumber + '\n')
    thesocket.close()
    return returnstr

# Create the Flask object
app = Flask(__name__)

# Splash/Login page
@app.route('/')
def splash():
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))

    return render_template('login.html',
                            loginfailed = request.args.get('loginfailed'),
                            deletedaccount = request.args.get('deletedaccount'))

# Redirect for login POST logic
@app.route('/login', methods=['POST', 'GET'])
def login():
    if request.method == 'POST':
        username = request.form['username']

        # Check if login is successful
        chkpwd = socketsendrecv('CHKPWD ' + username + '\n' + request.form['password'] + '\n')
        if chkpwd == 'YES':
            # Add to current session
            session['username'] = username
            # Persist the session across closed windows (on the same browser)
            session.permanent = True
            # Redirect to home page after login
            return redirect(url_for('home'))

    # If no POST request, or login is unsuccessful, redirect to splash
    # Note that this passes 'loginfailed' through the URL
    return redirect(url_for('splash', loginfailed = True))

# Registration page for making new accounts
@app.route('/register')
def reg():
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))
    return render_template('register.html')

# Check registration POST validity
@app.route('/checkregistration', methods=['POST', 'GET'])
def checkreg():
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))

    if request.method == 'POST':
        username = request.form['username']
        enteredemail = request.form['email']
        # Check for spaces and tabs in user/email (we disallow these)
        if ' ' in username or ' ' in enteredemail or '\t' in username or '\t' in enteredemail:
            return render_template('register.html', spacereg = True)
        # Check for empty username/password/email
        if username == '' or request.form['password'] == '' or enteredemail == '':
            return render_template('register.html', emptyreg = True)
        # Check for duplicate user/email
        chkusr = socketsendrecv('CHKUSR ' + username + '\n')
        chkeml = socketsendrecv('CHKEML ' + enteredemail + '\n')
        if chkusr == 'YES' or chkeml == 'YES':
            return render_template('register.html', regfail = True)
        socketsendrecv('CRTUSR ' + username + '\n' + request.form['password'] + '\n'
                  + enteredemail + '\n')
        return render_template('regsuccess.html')

    # If someone landed here not on a POST request, send them back to register page
    return render_template('register.html')

# Home page after being logged in
@app.route('/home')
def home():
    # If there's no active session, redirect back to splash/login page
    if 'username' not in session:
        return redirect(url_for('splash'))

    # Retrieve email and friend list
    alldata = socketsendrecv('POPLAT ' + session['username'] + '\n')
    splitdata = alldata.split('\n')
    index = 0
    retrievedemail = splitdata[index]
    index += 1
    retrievedfriends = []
    for i in xrange(int(splitdata[index])):
        index += 1
        retrievedfriends.append(splitdata[index])

    # Retrieve chirps of this user
    allchirps = [{
                      'author': session['username'],
                      'chirps': []
                 }]
    index += 1
    for j in xrange(int(splitdata[index])):
        index += 1
        allchirps[0]['chirps'].append(splitdata[index])

    # Retrieve chirps of user's friends
    for k in xrange(len(retrievedfriends)):
        allchirps.append({
                              'author': retrievedfriends[k],
                              'chirps': []
                         })
        index += 1
        for l in xrange(int(splitdata[index])):
            index += 1
            allchirps[k + 1]['chirps'].append(splitdata[index])

    # Otherwise, generate the home page
    return render_template('home.html',
                            username       = session['username'],
                            email          = retrievedemail,
                            friends        = retrievedfriends,
                            chirps         = allchirps,
                            emptychirp     = request.args.get('emptychirp'),
                            emptyfriend    = request.args.get('emptyfriend'),
                            addyourself    = request.args.get('addyourself'),
                            friendnotfound = request.args.get('friendnotfound'),
                            alreadyfriends = request.args.get('alreadyfriends'))

# Posting a chirp
@app.route('/postchirp', methods=['POST', 'GET'])
def postchirp():
    if request.method == 'POST':
        # Check for empty chirp before posting
        if request.form['chirp'].strip() != '':
            socketsendrecv('CRTCHP ' + session['username'] + '\n' + request.form['chirp'] + '\n')
        else:
            return redirect(url_for('home', emptychirp = True))
    return redirect(url_for('home'))

# Delete a chirp
@app.route('/deletechirp/<chirp_id>')
def deletechirp(chirp_id):
    # Make sure that if the user types in the URL, the chirp_id is a valid integer
    if 'username' in session and chirp_id.isdigit():
        socketsendrecv('DELCHP ' + session['username'] + '\n' + chirp_id + '\n')
    return redirect(url_for('home'))

# Adding a friend
@app.route('/addfriend', methods=['POST', 'GET'])
def addfriend():
    if request.method == 'POST':
        # Check for empty friend name
        if request.form['friend'] == '':
            return redirect(url_for('home', emptyfriend = True))
        # Check for friend name being the user themselves
        if request.form['friend'] == session['username']:
            return redirect(url_for('home', addyourself = True))
        # Check for friend name being a valid user
        chkusr = socketsendrecv('CHKUSR ' + request.form['friend'] + '\n')
        if chkusr == 'NO':
            return redirect(url_for('home', friendnotfound = True))
        # Check for friend already added
        chkfnd = socketsendrecv('CHKFND ' + session['username'] + '\n' + request.form['friend'] + '\n')
        if chkfnd == 'YES':
            return redirect(url_for('home', alreadyfriends = True))
        # Add friend
        socketsendrecv('ADDFND ' + session['username'] + '\n' + request.form['friend'] + '\n')
    return redirect(url_for('home'))

# Unfollow a friend
@app.route('/unfollow/<user>')
def unfollow(user):
    if 'username' in session:
        chkfnd = socketsendrecv('CHKFND ' + session['username'] + '\n' + user + '\n')
        if chkfnd == 'YES':
            socketsendrecv('DELFND ' + session['username'] + '\n' + user + '\n')
    return redirect(url_for('home'))

# Move a friend up the list
@app.route('/moveup/<user_id>')
def moveup(user_id):
    if 'username' in session and user_id.isdigit():
        socketsendrecv('MOVEUP ' + session['username'] + '\n' + user_id + '\n')
    return redirect(url_for('home'))

# Move a friend down the list
@app.route('/movedown/<user_id>')
def movedown(user_id):
    if 'username' in session and user_id.isdigit():
        socketsendrecv('MOVEDN ' + session['username'] + '\n' + user_id + '\n')
    return redirect(url_for('home'))

# Logout
@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('splash'))

# Delete account
@app.route('/deleteaccount')
def deleteaccount():
    deletesuccess = False
    if 'username' in session:
        socketsendrecv('DELUSR ' + session['username'] + '\n')
        session.clear()
        deletesuccess = True
    return redirect(url_for('splash', deletedaccount = deletesuccess))

# Set secret key for sessions
app.secret_key = '\xbby\x1b\x90\x93v\x97LGK\x8f\xeaE\x1c\xd8\xd2Q\x8e\xe0z\x8d\xdc\xf5\x8c'

# Run the Flask application
app.run('localhost', 8000, debug = False)
