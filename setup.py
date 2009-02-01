"""\
Python/JavaScript bridge module, making use of Mozilla's spidermonkey
JavaScript implementation.  Allows implementation of JavaScript classes,
objects and functions in Python, and evaluation and calling of JavaScript
scripts and functions respectively.  Borrows heavily from Claes Jacobssen's
Javascript Perl module, in turn based on Mozilla's 'PerlConnect' Perl binding.
""",

import os
import sys
import ez_setup
ez_setup.use_setuptools()
from setuptools import setup, Extension

try:
    import Pyrex.Compiler.Main as Compiler
    res = Compiler.compile(["spidermonkey/spidermonkey.pyx"], timestamps=True)
except ImportError:
    print >>sys.stderr, "Pyrex not found. Skipping source re-generation."

def get_platform_config():
    """Retrieve platform specific locatiosn for headers and libraries."""
    platforms = {
        "darwin": {
            "include_dirs": ["/usr/include", "/usr/local/include", "/opt/local/include/js"],
            "library_dirs": ["/usr/lib", "/usr/local/lib", "/opt/local/lib"],
            "libraries": ["js"]
        },
        "freebsd": {
            "include_dirs": ["/usr/include", "/usr/local/include", "/usr/local/include/js"],
            "library_dirs": ["/usr/lib", "/usr/local/lib"],
            "libraries": ["js"]
        },  
        "linux": {
            "include_dirs": ["/usr/include", "/usr/include/mozjs", "/usr/local/include"],
            "library_dirs": ["/usr/lib", "/usr/local/lib"],
            "libraries": ["mozjs"]
        },
        "openbsd": {
            "include_dirs": ["/usr/include", "/usr/local/include", "/usr/local/include/js"],
            "library_dirs": ["/usr/lib", "/usr/local/lib"],
            "libraries": ["js"]
        }
    }
    arch = os.uname()[0].lower()
    if arch not in platforms:
        print "Failed to find a platform configuration."
        exit(-1)
    return platforms[arch]

setup(
    name = "python-spidermonkey",
    version = "0.0.2",
    license = "MIT",
    author = "Paul J. Davis",
    author_email = "paul.joseph.davis@gmail.com",
    description = "JavaScript / Python bridge.",
    long_description = __doc__,
    url = "http://github.com/davisp/python-spidermonkey",
    download_url = "http://github.com/davisp/python-spidermonkey.git",
    zip_safe = False,
    
    classifiers = [
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Natural Language :: English',
        'Operating System :: OS Independent',
        'Programming Language :: C',
        'Programming Language :: JavaScript',
        'Programming Language :: Other',
        'Programming Language :: Python',
        'Topic :: Internet :: WWW/HTTP :: Browsers',
        'Topic :: Internet :: WWW/HTTP :: Dynamic Content',
        'Topic :: Software Development :: Libraries :: Python Modules',
    ],
    
    setup_requires = [
        'setuptools>=0.6c8',
        'pyrex>=0.9.8.5',
    ],

    ext_modules =  [
        Extension(
            "spidermonkey",
            sources=["spidermonkey/spidermonkey.c"],
            extra_compile_args=["-DXP_UNIX", "-DJS_THREADSAFE"],
            **get_platform_config()
        )
    ],

    test_suite = 'nose.collector',

)
