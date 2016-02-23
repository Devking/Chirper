# Wells Santo and Patrick Kingchatchaval

# import functionality from the Flask package
from flask import Flask, render_template, redirect, request, session

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
@app.route('/login', methods=['POST'])
def login():
	if request.method == 'POST':

		username = request.form["username"]
		if username in users.keys() and users[username]["password"] == request.form["password"]:
			return render_template("verify.html",
			                   	   username = username,
			                       chirps   = users[username]["chirps"])
		else:
			return redirect(url_for('hello'))

# Registration page for making new accounts
@app.route('/register')
def reg():
	return render_template("register.html")

@app.route('/home')
def home():
	return render_template("verify.html",
			               username = username,
			               chirps   = users[username]["chirps"])

# Run the Flask application
app.run("127.0.0.1", 8000, debug = True)
