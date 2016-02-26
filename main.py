# Wells Santo and Patrick Kingchatchaval

# SPAAAAAAAAAAAAAAAAAAAAACE (not tabs)

# import functionality from the Flask package
from flask import Flask, render_template, redirect, request, url_for, session

# import in-memory user information
from manifest import users, emails

# Create the Flask object
app = Flask(__name__)

# Splash page for the web application
@app.route('/')
def splash():

    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))

    return render_template("login.html", loginfailed = request.args.get('loginfailed'))

# Login page to handle logging in
@app.route('/login', methods=['POST', 'GET'])
def login():

    # Check if a POST request is being made
    if request.method == 'POST':

        username = request.form["username"]

        # Login is successful
        if username in users.keys() and users[username]["password"] == request.form["password"]:
            
            # Add to current session
            session['username'] = username
            # Persist the session across closed windows
            session.permanent = True

            # Redirect to home page after login
            return redirect(url_for('home'))

    # If no POST request, or login is unsuccessful, redirect to splash
    # Note that this passes 'loginfailed' through the URL...
    return redirect(url_for('splash', loginfailed = True))

# Registration page for making new accounts
@app.route('/register')
def reg():
    if 'username' in session:
        return redirect(url_for('home'))
    return render_template("register.html", regfail = False)

@app.route('/checkregistration', methods=['POST', 'GET'])
def checkreg():
    if 'username' in session:
        return redirect(url_for('home'))
    if request.method == 'POST':
        username = request.form["username"]

        # If this is a new user, add them to the user table
        if username in users.keys():
            return render_template("register.html", regfail = True)
        else:
            enteredEmail = request.form["email"]
            if enteredEmail in emails:
                    return render_template("register.html", regfail = True)
            users[username] = {
                'password': request.form["password"],
                'email': enteredEmail,
                'chirps': [],
                'friends': []
            }
            emails.add(enteredEmail)
            return render_template("regsuccess.html")

    # If someone landed here not on a POST request, send back to register page
    return render_template("register.html", regfail = False)

# Home page after logged in
@app.route('/home')
def home():

    # If there's no active session, redirect back to splash page
    if 'username' not in session:
        return redirect(url_for('splash'))

    # Retrieve chirps of user and user's friends
    allChirps = [{
                      'author': session['username'],
                      'chirps': users[session['username']]['chirps']
                 }]
    for friend in users[session['username']]['friends']:
        allChirps.append({
                              'author': friend,
                              'chirps': users[friend]['chirps']
                         })

    # Otherwise, generate the home page
    return render_template("verify.html",
        username = session['username'],
        email    = users[session['username']]['email'],
        chirps   = allChirps)

# Posting a chirp
@app.route('/postchirp', methods=['POST', 'GET'])
def postchirp():
    if request.method == 'POST':
        users[session['username']]['chirps'].insert(0, request.form["chirp"])
    return redirect(url_for('home'))
    

# Logout
@app.route('/logout')
def logout():
    # Clear the session
    session.clear()
    # Redirect back to the splash page
    return redirect(url_for('splash'))

# Set secret key for sessions
app.secret_key = '\xbby\x1b\x90\x93v\x97LGK\x8f\xeaE\x1a\xd8\xd2Q\x8e\xe0z\x8d\xdc\xf5\x8c'

# Run the Flask application
app.run("localhost", 8000, debug = True)

# Note: If you restart the 'server' while someone is accessing a session, bad things happen
