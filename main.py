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
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))
    
    return render_template("register.html", regfail = False)

@app.route('/checkregistration', methods=['POST', 'GET'])
def checkreg():
    # Check if someone is already logged in
    if 'username' in session:
        return redirect(url_for('home'))
    
    if request.method == 'POST':
        username = request.form["username"]

        # If this is a new user, add them to the user table
        if username not in users.keys():
            enteredemail = request.form["email"]
            if enteredemail not in emails:
                if username != '' and request.form["password"] != '' and request.form["email"] != '':
                    users[username] = {
                        'password': request.form["password"],
                        'email': enteredemail,
                        'chirps': [],
                        'friends': []
                    }
                    emails.add(enteredemail)
                    return render_template("regsuccess.html")
                else:
                    return render_template("register.html", regfail = False, emptyreg = True)
            else:
                return render_template("register.html", regfail = True, emptyreg = False)
        else:
            return render_template("register.html", regfail = True, emptyreg = False)

    # If someone landed here not on a POST request, send back to register page
    return render_template("register.html", regfail = False)

# Home page after logged in
@app.route('/home')
def home():

    # If there's no active session, redirect back to splash page
    if 'username' not in session:
        return redirect(url_for('splash'))

    # Retrieve chirps of user and user's friends
    allchirps = [{
                      'author': session['username'],
                      'chirps': users[session['username']]['chirps']
                 }]
    for friend in users[session['username']]['friends']:
        allchirps.append({
                              'author': friend,
                              'chirps': users[friend]['chirps']
                         })

    # Otherwise, generate the home page
    return render_template("verify.html", 
        username = session['username'],
        email    = users[session['username']]['email'],
        chirps   = allchirps,
        emptychirp = request.args.get('emptychirp'))

# Posting a chirp
@app.route('/postchirp', methods=['POST', 'GET'])
def postchirp():
    if request.method == 'POST':
        if request.form["chirp"] != '':
            users[session['username']]['chirps'].insert(0, request.form["chirp"])
        else:
            return redirect(url_for('home', emptychirp = True))
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
