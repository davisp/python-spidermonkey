#
# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
#

"""\
Python/JavaScript bridge module, making use of Mozilla's spidermonkey
JavaScript implementation.  Allows implementation of JavaScript classes,
objects and functions in Python, and evaluation and calling of JavaScript
scripts and functions respectively.  Borrows heavily from Claes Jacobssen's
Javascript Perl module, in turn based on Mozilla's 'PerlConnect' Perl binding.
""",

# I haven't the sligthest, but this appears to fix
# all those EINTR errors. Pulled and adapted for OS X
# from twisted bug #733
# 
# Definitely forgot to comment this out before distribution.
#
# import ctypes
# import signal
# libc = ctypes.CDLL("libc.dylib")
# libc.siginterrupt(signal.SIGCHLD, 0)

import glob
import os
import subprocess as sp
import sys
from distutils.dist import Distribution
import ez_setup
ez_setup.use_setuptools()
from setuptools import setup, Extension

DEBUG = "--debug" in sys.argv
USE_SYSTEM_LIB = "--system-library" in sys.argv

def find_sources(extensions=(".c", ".cpp")):
    if USE_SYSTEM_LIB:
        return [
            fname
            for ext in extensions
            for fname in glob.glob("spidermonkey/*" + ext)
        ]
    else:
        return [
            os.path.join(dpath, fname)
            for (dpath, dnames, fnames) in os.walk("./spidermonkey")
            for fname in fnames
            if fname.endswith(extensions)
        ]

def pkg_config(pkg_name, config=None):
    pipe = sp.Popen("pkg-config --cflags --libs %s" % pkg_name,
                        shell=True, stdout=sp.PIPE, stderr=sp.PIPE)
    (stdout, stderr) = pipe.communicate()
    if pipe.wait() != 0:
        raise RuntimeError("No package configuration found for: %s" % pkg_name)
    if config is None:
        config = {
            "include_dirs": [],
            "library_dirs": [],
            "libraries": [],
            "extra_compile_args": [],
            "extra_link_args": []
        }
    prefixes = {
        "-I": ("include_dirs", 2),
        "-L": ("library_dirs", 2),
        "-l": ("libraries", 2),
        "-D": ("extra_compile_args", 0),
        "-Wl": ("extra_link_args", 0)
    }
    for flag in stdout.split():
        for prefix in prefixes:
            if not flag.startswith(prefix):
                continue
            # Hack for xulrunner
            if flag.endswith("/stable"):
                flag = flag[:-6] + "unstable"
            name, trim = prefixes[prefix]
            config[name].append(flag[trim:])
    return config

def nspr_config(config=None):
    return pkg_config("nspr", config)

def js_config(config=None):
    config = pkg_config("mozilla-js", config)
    if "-DJS_THREADSAFE" not in config["extra_compile_args"]:
        raise SystemError("Unable to link against a library that was "
            "compiled without -DJS_THREADSAFE");
    return config

def platform_config():
    sysname = os.uname()[0]
    machine = os.uname()[-1]

    # If we're linking against a system library it should give
    # us all the information we need.
    if USE_SYSTEM_LIB:
        return js_config()
    
    # Build our configuration
    config = {
        "extra_compile_args": [
            "-DJS_THREADSAFE",
            "-DPOSIX_SOURCE",
            "-D_BSD_SOURCE",
            "-Wno-strict-prototypes" # Disable copius JS warnings
        ],
        "include_dirs": [
            "spidermonkey/libjs",
            "spidermonkey/%s-%s" % (sysname, machine)
        ],
        "library_dirs": [],
        "libraries": [],
        "extra_link_args": []
    }

    # Debug builds are useful for finding errors in
    # the request counting semantics for Spidermonkey
    if DEBUG:
        config["extra_compile_args"].extend([
            "-UNDEBG",
            "-DDEBUG",
            "-DJS_PARANOID_REQUEST"
        ])

    if sysname in ["Linux", "FreeBSD"]:
        config["extra_compile_args"].extend([
            "-DHAVE_VA_COPY",
            "-DVA_COPY=va_copy"
        ])

    # Currently no suppot for Win32, patches welcome.
    if sysname in ["Darwin", "Linux", "FreeBSD"]:
        config["extra_compile_args"].append("-DXP_UNIX")
    else:
        raise RuntimeError("Unknown system name: %s" % sysname)

    return nspr_config(config=config)

Distribution.global_options.append(("debug", None,
                    "Build a DEBUG version of spidermonkey."))
Distribution.global_options.append(("system-library", None,
                    "Link against an installed system library."))

setup(
    name = "python-spidermonkey",
    version = "0.0.6",
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
        'nose>=0.10.0',
    ],

    ext_modules =  [
        Extension(
            "spidermonkey",
            sources=find_sources(),
            **platform_config()
        )
    ],

    test_suite = 'nose.collector',

)
