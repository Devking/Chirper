# Wells Santo and Patrick Kingchatchaval

# import functionality from the Flask package
from flask import Flask, render_template, request

# import in-memory user information
from manifest import users

# Create the Flask object
app = Flask(__name__)

# Main page for the web application
@app.route('/', methods=['POST', 'GET'])
def hello():

	loginfail = False

	if request.method == 'POST':

		username = request.form["username"]
		if username in users.keys() and users[username]["password"] == request.form["password"]:
			return render_template("verify.html",
			                   	   username = username,
			                       chirps   = users[username]["chirps"])

		loginfail = True

	return render_template("login.html", loginfail = loginfail)

# Registration page for making new accounts
@app.route('/register')
def reg():
	return render_template("register.html")

# Run the Flask application
app.run("127.0.0.1", 8000, debug = True)
