# CS3254 Parallel and Distributed Systems
# Code Written By: Wells Santo and Patrick Kingchatchaval

from flask import Flask, render_template, redirect, request, url_for, session

# Import in-memory user information, aka some default dummy created users and dummy messages
from manifest import users, emails

# Create the Flask object
app = Flask(__name__)

# Splash/Login page
@app.route('/')
def splash():
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))

    return render_template("login.html",
                            loginfailed = request.args.get('loginfailed'),
                            deletedaccount = request.args.get('deletedaccount'))

# Redirect for login POST logic
@app.route('/login', methods=['POST', 'GET'])
def login():
    if request.method == 'POST':
        username = request.form["username"]

        # Check if login is successful
        if username in users.keys() and users[username]["password"] == request.form["password"]:
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
    return render_template("register.html")

# Check registration POST validity
@app.route('/checkregistration', methods=['POST', 'GET'])
def checkreg():
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))

    if request.method == 'POST':
        username = request.form["username"]
        enteredemail = request.form["email"]
        # Check for duplicate user/email
        if username in users.keys() or enteredemail in emails:
            return render_template("register.html", regfail = True)
        # Check for spaces and tabs in user/email (we disallow these)
        if ' ' in username or ' ' in enteredemail or '\t' in username or '\t' in enteredemail:
            return render_template("register.html", spacereg = True)
        # Check for empty username/password/email
        if username != '' and request.form["password"] != '' and enteredemail != '':
            users[username] = {
                'password': request.form["password"],
                'email': enteredemail,
                'chirps': [],
                'friends': []
            }
            emails.add(enteredemail)
            return render_template("regsuccess.html")
        else:
            return render_template("register.html", emptyreg = True)

    # If someone landed here not on a POST request, send them back to register page
    return render_template("register.html")

# Home page after being logged in
@app.route('/home')
def home():
    # If there's no active session, redirect back to splash/login page
    if 'username' not in session:
        return redirect(url_for('splash'))

    # Retrieve chirps of this user
    allchirps = [{
                      'author': session['username'],
                      'chirps': users[session['username']]['chirps']
                 }]

    # Retrieve chirps of user's friends
    for friend in users[session['username']]['friends']:
        # Check first if the user exists; if not, remove the user from the friend's list
        if friend in users.keys():
            allchirps.append({
                                  'author': friend,
                                  'chirps': users[friend]['chirps']
                             })
        else:
            users[session['username']]['friends'].remove(friend)

    # Otherwise, generate the home page
    return render_template("home.html",
                            username       = session['username'],
                            email          = users[session['username']]['email'],
                            friends        = users[session['username']]['friends'],
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
        if request.form["chirp"].strip() != '':
            users[session['username']]['chirps'].insert(0, request.form["chirp"])
        else:
            return redirect(url_for('home', emptychirp = True))
    return redirect(url_for('home'))

# Delete a chirp
@app.route('/deletechirp/<chirp_id>')
def deletechirp(chirp_id):
    # Make sure that if the user types in the URL, the chirp_id is a valid integer
    if 'username' in session and chirp_id.isdigit():
        chirp_id = int(chirp_id)
        if chirp_id < len(users[session['username']]['chirps']):
            users[session['username']]['chirps'].pop(chirp_id)
    return redirect(url_for('home'))

# Adding a friend
@app.route('/addfriend', methods=['POST', 'GET'])
def addfriend():
    if request.method == 'POST':
        if request.form["friend"] == '':
            return redirect(url_for('home', emptyfriend = True))
        if request.form["friend"] == session['username']:
            return redirect(url_for('home', addyourself = True))
        if request.form["friend"] not in users.keys():
            return redirect(url_for('home', friendnotfound = True))
        if request.form["friend"] in users[session['username']]['friends']:
            return redirect(url_for('home', alreadyfriends = True))
        users[session['username']]['friends'].append(request.form["friend"])
    return redirect(url_for('home'))

# Unfollow a friend
@app.route('/unfollow/<user>')
def unfollow(user):
    if 'username' in session:
        if user in users[session['username']]['friends']:
            users[session['username']]['friends'].remove(user)
    return redirect(url_for('home'))

# Move a friend up the list
@app.route('/moveup/<user_id>')
def moveup(user_id):
    if 'username' in session and user_id.isdigit():
        user_id = int(user_id)
        if user_id > 0 and user_id < len(users[session['username']]['friends']):
            users[session['username']]['friends'][user_id], users[session['username']]['friends'][user_id - 1] = \
                users[session['username']]['friends'][user_id - 1], users[session['username']]['friends'][user_id]
    return redirect(url_for('home'))

# Move a friend down the list
@app.route('/movedown/<user_id>')
def movedown(user_id):
    if 'username' in session and user_id.isdigit():
        user_id = int(user_id)
        if user_id > -1 and user_id < len(users[session['username']]['friends']) - 1:
            users[session['username']]['friends'][user_id], users[session['username']]['friends'][user_id + 1] = \
                users[session['username']]['friends'][user_id + 1], users[session['username']]['friends'][user_id]
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
        emails.remove(users[session['username']]['email'])
        del users[session['username']]
        session.clear()
        deletesuccess = True
    return redirect(url_for('splash', deletedaccount = deletesuccess))

# Set secret key for sessions
app.secret_key = '\xbby\x1b\x90\x93v\x97LGK\x8f\xeaE\x1c\xd8\xd2Q\x8e\xe0z\x8d\xdc\xf5\x8c'

# Run the Flask application
app.run("localhost", 8000, debug = False)
