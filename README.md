
Execute arbitrary JavaScript code from Python. Allows you to reference
arbitrary Python objects and functions in the JavaScript VM

Requirements
============

Python Development Files
------------------------

Mac OS X:

If you installed Python via port then the headers should already be installed.
I have not heard reports of problems from people using the bundled
interpreters.

Debian/Ubuntu:

    $ sudo apt-get install pythonX.X-dev

Where X.X is the version of Python you are using. I have not tested
python-spidermonkey on Py3K so it may be horribly broken there.

Netscape Portable Runtime (nspr)
--------------------------------

The nspr library is required for building the Spidermonkey sources. You should
be able to grab it from your package manager of choice with something like the
following:

Mac OS X:

    $ sudo port install nspr

Debian/Ubuntu:

    $ sudo apt-get install libnspr4-dev

Alternatively you can build from [source][nspr]. If you choose this route make
sure that the nspr-config command is on your $PATH when running the install
commands below.

Installation
============

    $ git clone git://github.com/davisp/python-spidermonkey.git
    $ cd python-spidermonkey
    $ python setup.py build
    $ python setup.py test

    $ sudo python setup.py install

    *OR*
    
    $ sudo python setup.py develop

Having Issues?
==============

Add issues to the Lighthouse project [here][lh].


Examples
========

Basics
------

    >>> import spidermonkey
    >>> rt = spidermonkey.Runtime()
    >>> cx = rt.new_context()
    >>> cx.execute("var x = 3; x *= 4; x;")
    12
    >>> class Orange(object):
    ...   def is_ripe(self,arg):
    ...       return "ripe %s" % arg
    ...
    >>> fruit = Orange()
    >>> cx.add_global("apple", fruit)
    >>> cx.execute('"Show me the " + apple.is_ripe("raisin");')
    Show me the ripe raisin

Playing with Classes
--------------------

    >>> import spidermonkey
    >>> class Monkey(object):
    ...     def __init__(self):
    ...         self.baz = "blammo"
    ...     def wrench(self, arg):
    ...         return "%s now wrenched" % arg
    ...
    >>> rt = spidermonkey.Runtime()
    >>> cx = rt.new_context()
    >>> cx.add_global(Monkey)
    >>> monkey = cx.execute('var x = new Monkey(); x.baz = "schmammo"; x;')
    >>> monkey.baz
    'schmammo'
    >>> monkey.__class__.__name__
    'Monkey'

JavaScript Functions
--------------------

    >>> import spidermonkey
    >>> rt = spidermonkey.Runtime()
    >>> cx = rt.new_context()
    >>> func = cx.execute('function(val) {return "whoosh: " + val;}')
    >>> func("zipper!");
    'whoosh: zipper!'

Previous Authors
================

* John J. Lee
* Atul Varma

[lh]: http://davisp.lighthouseapp.com/projects/26898-python-spidermonkey/overview
[nspr]: ftp://ftp.mozilla.org/pub/mozilla.org/nspr/releases
