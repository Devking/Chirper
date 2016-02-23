# Wells Santo and Patrick Kingchatchaval

# import functionality from the Flask package
from flask import Flask, render_template, redirect, request, url_for, session

# import in-memory user information
from manifest import users

# Create the Flask object
app = Flask(__name__)

# Splash page for the web application
@app.route('/')
def hello(loginstatus = False):

	# Check if someone is already logged in
	if 'username' in session:
		return redirect(url_for('home'))

	return render_template("login.html", loginstatus = loginstatus)

# Login page to handle logging in
@app.route('/login', methods=['POST', 'GET'])
def login():
	if request.method == 'POST':

		username = request.form["username"]

		# Login is successful
		if username in users.keys() and users[username]["password"] == request.form["password"]:
			
			# Add to current session
			session['username'] = username
			session['chirps'] = users[username]['chirps']

			# Redirect to home page after login
			return redirect(url_for('home'))

	return redirect(url_for('hello'))

# Registration page for making new accounts
@app.route('/register')
def reg():
	return render_template("register.html")

# Home page after logged in
@app.route('/home')
def home():

	# If there's no active session, redirect back to splash page
	if 'username' not in session:
		return redirect(url_for('hello'))

	# Otherwise, generate the home page
	return render_template("verify.html",
			               username = session['username'],
			               chirps   = session['chirps'])

# Logout
@app.route('/logout')
def logout():
	# Clear the session
	session.clear()
	# Redirect back to the splash page
	return redirect(url_for('hello'))

# Set secret key for sessions
app.secret_key = '\xbby\x1b\x90\x93v\x97LGK\x8f\xeaE\x1a\xd8\xd2Q\x8e\xe0z\x8d\xdc\xf5\x8c'

# Run the Flask application
app.run("127.0.0.1", 8000, debug = True)
