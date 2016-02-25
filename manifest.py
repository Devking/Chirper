# Currently this is just for debugging
# This information should go in a text file or database

# Flask's templates automatically escape the HTML -- no need to worry about injection
users = {
            'admin': {
                'password': 'pass',
                'email': 'admin@admin.com',
                'chirps': ['<b>Hello</b>', 'Invest In Kanye West Ideas'],
                'friends': ['bob']
            },

            'bob': {
                'password': 'dole',
                'email': 'bob@dole.com',
                'chirps': ['Hi', 'something'],
                'friends': []
            }
        }

emails = {'admin@admin.com', 'bob@dole.com'}
