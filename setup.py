import os
import sys
import glob
from distutils.core import setup, Extension
from distutils.command.clean import clean
from distutils.command.install import install
from distutils.file_util import copy_file

try:
    import Pyrex.Compiler.Main as Compiler
    res = Compiler.compile(["spidermonkey.pyx"], timestamps=True)
except ImportError:
    if not os.path.exists("spidermonkey.c"):
        sys.stderr.write("Pyrex is required for compiliation.")

arch = os.uname()[0].lower()
jslib = {"darin": "js", "linux": "mozjs"}.get(arch)
if not jslib:
    sys.stderr.write("Failed to guess what JavaScript lib you might be using.")

setup(name = "spidermonkey",
    version = "0.0.1a",
    license = "GPL",
    author = "John J. Lee",
    author_email = "jjl@pobox.com",
    description = "JavaScript / Python bridge.",
    url = "http://code.google.com/p/python-spidermonkey/",
    long_description = """\
Python/JavaScript bridge module, making use of Mozilla's spidermonkey
JavaScript implementation.  Allows implementation of JavaScript classes,
objects and functions in Python, and evaluation and calling of JavaScript
scripts and functions respectively.  Borrows heavily from Claes Jacobssen's
Javascript Perl module, in turn based on Mozilla's 'PerlConnect' Perl binding.
""",
    ext_modules =  [
        Extension("spidermonkey",
            sources=["spidermonkey.c"],
            extra_compile_args=["-DXP_UNIX", "-DJS_THREADSAFE"],
            include_dirs=["/usr/include/js", "/usr/local/include/js", "/usr/include/mozjs", "/opt/local/include/js"],
            runtime_libraries=["/usr/lib", "/usr/local/lib", "/opt/local/lib"],
            libraries=[jslib, "pthread"]
        )
    ]
)
