# Wells Santo and Patrick Kingchatchaval

from flask import Flask, render_template, request

app = Flask(__name__)

# Currently this is just for debugging
chirps = [
			{
				'author': 'Gary',
				'message': 'Hello'
			},
			{
				'author': 'Kanye',
				'message': 'Invest In Kanye Ideas'
			}
		 ]

@app.route('/', methods=['POST', 'GET'])
def hello():
	if request.method == 'POST':
		return render_template("verify.html",
			                   username=request.form["username"],
			                   password=request.form["password"],
			                   chirps=chirps)

	return render_template("login.html")

app.run("127.0.0.1", 8000, debug=True)
