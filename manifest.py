# For our project, the users and chirps are stored in-memory, following the project specs.
# For a larger production project, this would all go into a database.

# Flask's templates automatically escape the HTML -- no need to worry about injection

users = {
            'admin': {
                'password': 'pass',
                'email': 'admin@admin.com',
                'chirps': ['Hello', 'Invest In Kanye West Ideas'],
                'friends': ['bob']
            },

            'bob': {
                'password': 'dole',
                'email': 'bob@dole.com',
                'chirps': ['Hi', 'something'],
                'friends': []
            }
        }

# Keep track of existing emails in a separate set
# This way, we can check for duplicate e-mails during registration in O(1) time
# Again, this would be cleaned up if a database were used
emails = {'admin@admin.com', 'bob@dole.com'}
