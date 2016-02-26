# Chirper

The not-very-original social media alternative, designed by **Wells Lucas Santo** and **Patrick Kingchatchaval**.

## Dependencies

This application runs on **Python 2.7.11** and requires the **Flask** package.

This application also has a minor feature that uses **jQuery**.

We are using jQuery from Google's hosted library service, so this feature will only work if you are connected to the internet.

This application has been tested on the latest versions of Chrome, Safari, and Firefox, on Ubuntu, Mac OSX, and Windows.

## Running the Application

On this directory, run the following in the terminal:

`python main.py`

This will start a Flask-based server, running on localhost.

You can access the web application at the following address:

`http://127.0.0.1:8000`

This server will persist the session (aka retain all newly created users and messages) until you stop the server.

## Dummy Example Data

Right now, since we are storing user data in memory, we already have two users in the system as dummy example data.

These two users are as follows:

> **Username:** admin
>
> **Password:** pass

and

> **Username:** bob
>
> **Password:** dole

admin is following bob, but bob is not following admin.
