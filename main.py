# Wells Santo and Patrick Kingchatchaval

from flask import Flask, render_template, request

app = Flask(__name__)

@app.route('/', methods=['POST', 'GET'])
def hello():
	if request.method == 'POST':
		# print request.form["form_data"]
		# return 'got a post'
		# Generate the template page making use of the form data!
		return render_template("verify.html", responsetext=request.form["form_data"])
	# Instead of having inline HTML, we can just load a html page
	# Our HTML form should have a name -- so we get that variable back in the POST
	return render_template("login.html")

app.run("127.0.0.1", 8000, debug=True)
