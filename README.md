Python-Spidermonkey
===================

Execute arbitrary JavaScript code from Python. Allows you to reference
arbitrary Python objects and functions in the JavaScript VM

Installation
============


    $ git clone git://github.com/davisp/python-spidermonkey.git
    $ cd python-spidermonkey
    $ python setup.py build
    $ nosetests
    $ sudo python setup.py develop

There are quite a few compiler warnings about casting pointers that I haven't gone back and fixed yet, so kindly ignore them. Or get irritated enough to fix them for me. :D

Basics
------

    >>> import spidermonkey
    >>> rt = spidermonkey.Runtime()
    >>> cx = rt.create_context()
    >>> print cx.execute("var x = 3; x *= 4; x;")
    12
    >>> class Orange(object):
    ...   def is_ripe(self,arg):
    ...       return "ripe %s" % arg
    ...
    >>> fruit = Orange()
    >>> cx.bind("apple", fruit)
    >>> print cx.execute('"Show me the " + apple.is_ripe("raisin");')
    Show me the ripe raisin

Global Namespace
----------------

    >>> import spidermonkey
    >>> class Monkey(object):
    ...     def __init__(self):
    ...         self.baz = "blammo"
    ...     def wrench(self, arg):
    ...         return "%s now wrenched" % arg
    ...
    >>> monkey = Monkey()
    >>> rt = spidermonkey.Runtime()
    >>> cx = rt.create_context(monkey)
    >>> cx.execute("baz;")
    'blammo'
    >>> cx.execute('wrench("ceiling fan");')
    'ceiling fan now wrenched'
    >>> cx.execute('doodad;')
    Traceback (most recent call last):
        ...
    JSError: JavaScript Error: Python(0): ReferenceError: doodad is not defined

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
    >>> cx = rt.create_context()
    >>> cx.install_class(Monkey)
    <spidermonkey.ClassAdapter: <class '__main__.Monkey'>>
    >>> monkey = cx.execute('var x = new Monkey(); x.baz = "blammo schmammo"; x;')
    >>> monkey.baz
    'blammo schmammo'
    >>> monkey.__class__.__name__
    'Monkey'

JavaScript Functions
--------------------

    >>> import spidermonkey
    >>> rt = spidermonkey.Runtime()
    >>> cx = rt.create_context()
    >>> func = cx.execute('function(val) {return "whoosh: " + val;}')
    >>> func("zipper!");
    'whoosh: zipper!'

Previous Authors
================

* John J. Lee
* Atul Varma
