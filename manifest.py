# Currently this is just for debugging
# This information should go in a text file or database

# Flask's templates automatically escape the HTML -- no need to worry about injection
users = {
            'admin': {
                'password': 'pass',
                'email': 'admin@admin.com',
                'chirps': [
                            {
                                'author': 'Gary',
                                'message': '<b>Hello</b>'
                            },
                            {
                                'author': 'Kanye',
                                'message': 'Invest In Kanye West Ideas'
                            }
                ]
            },

            'bob': {
                'password': 'dole',
                'email': 'bob@dole.com',
                'chirps': [
                            {
                                'author': 'bob',
                                'message': 'Hi'
                            },
                            {
                                'author': 'Someone',
                                'message': 'something'
                            }
                ]
            }
        }
