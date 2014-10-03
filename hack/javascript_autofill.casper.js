var casper = require('casper').create(
    /*{
        verbose: true,
        logLevel: 'debug'
    }*/
);

casper.start(
    'http://localhost:3000/javascript_autofill.php',
    function () {
        // fetch token even if we don't have to worry about it
        var token = this.evaluate(
            function () {
                var captcha = document.getElementById('captcha-wrapper');
                captcha.style.display = 'block';
                var s = '';
                var l = document.querySelectorAll('#captcha > span');
                for (var i = 0; i < l.length; i++) {
                    s += getComputedStyle(l[i], ':after').content.replace(/[^a-zA-Z0-9]/g, '');
                }
                captcha.style.display = 'none';

                return s;
            }
        );
        this.echo('Token is : ' + token);

        // submit the form (just ignore captcha field which is already filled in)
        this.fill(
            'form',
            {
                comment: 'owned owned owned',
                from: 'foo@bar.local',
            },
            true
        );
    }
);

casper.thenEvaluate(
    function () {
        if (this.exists('p.alert-success')) {
            this.echo('ok');
        } else {
            this.echo('ko');
        }
    }
);

casper.run();
