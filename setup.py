"""\
Python/JavaScript bridge module, making use of Mozilla's spidermonkey
JavaScript implementation.  Allows implementation of JavaScript classes,
objects and functions in Python, and evaluation and calling of JavaScript
scripts and functions respectively.  Borrows heavily from Claes Jacobssen's
Javascript Perl module, in turn based on Mozilla's 'PerlConnect' Perl binding.
""",

import os
import ez_setup
ez_setup.use_setuptools()
from setuptools import setup, Extension

try:
    import Pyrex.Compiler.Main as Compiler
    res = Compiler.compile(["spidermonkey/spidermonkey.pyx"], timestamps=True)
except ImportError:
    print "Pyrex not found: Skipping source generation."

def get_js_lib():
    arch = os.uname()[0].lower()
    jslib = {
        "darwin": "js",
        "linux": "mozjs"
    }.get(arch, None)
    if not jslib:
        print "Failed to guess what JavaScript lib you might be using."
    return jslib

setup(
    name = "spidermonkey",
    version = "0.1",
    license = "MIT",
    author = "Paul J. Davis",
    author_email = "paul.joseph.davis@gmail.com",
    description = "JavaScript / Python bridge.",
    long_description = __doc__,
    url = "http://github.com/davisp/python-spidermonkey",
    zip_safe = False
    
    classifiers = [
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Operating System :: OS Independent',
        'Programming Language :: Python',
        'Programming Lnaguage :: JavaScript',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],

    setup_requires = [
        'setuptools>=0.6c8'
    ],

    install_requires = [
        'couchdb', 
        'Hypy',
        'spidermonkey',
    ],
    
    ext_modules =  [
        Extension(
            "spidermonkey",
            sources=["spidermonkey/spidermonkey.c"],
            extra_compile_args=["-DXP_UNIX", "-DJS_THREADSAFE"],
            include_dirs=["/usr/include/js", "/usr/local/include/js", "/usr/include/mozjs", "/opt/local/include/js"],
            library_dirs=["/usr/lib", "/usr/local/lib", "/opt/local/lib"],
            libraries=["m", "pthread", get_js_lib()]
        )
    ]
)
