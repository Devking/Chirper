$(function() {

    $('.fn-delete').click(function () {
        if (confirm('Do you really want to delete your account? There\'s no going back!'))
            window.location.href = '/deleteaccount'
    })

});
