$(function() {

    $('.fn-chirp-field').focus()

    $(document).keyup(function(e) {
        var tag = e.target.tagName.toLowerCase();
        if (e.which == 78 && tag != 'input' && tag != 'textarea') {
            console.log('test')
            $('.fn-chirp-field').focus()
        }
    })

    $('.fn-delete').click(function () {
        if (confirm('Do you really want to delete your account? There\'s no going back!'))
            window.location.href = '/deleteaccount'
    })

    $('.fn-chirp-field').keydown(updateCharCount)
    $('.fn-chirp-field').keyup(updateCharCount)

    function updateCharCount() {
        var count = $(this).val().length
        $('.fn-chirp-count').text(100 - count)
    }

});
