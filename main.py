# Wells Santo and Patrick Kingchatchaval

from flask import Flask, render_template, request

app = Flask(__name__)

# Currently this is just for debugging
# This information should go in a text file or database
chirps = [
			{
				'author': 'Gary',
				'message': 'Hello'
			},
			{
				'author': 'Kanye',
				'message': 'Invest In Kanye West Ideas'
			}
		 ]

@app.route('/', methods=['POST', 'GET'])
def hello():

	loginfail = False

	if request.method == 'POST':

		if request.form["username"] == 'admin' and request.form["password"] == 'pass':
			return render_template("verify.html",
			                   	   username = request.form["username"],
			                       password = request.form["password"],
			                       chirps   = chirps)

		loginfail = True

	return render_template("login.html", loginfail = loginfail)

app.run("127.0.0.1", 8000, debug=True)
