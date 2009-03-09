# Copyright 2009 Paul J. Davis <paul.joseph.davis@gmail.com>
#
# This file is part of the python-spidermonkey package released
# under the MIT license.
import t

@t.echo()
def test_roundtrip_int(echo):
    t.eq(echo(42), 42)

@t.echo()
def test_roundtrip_float(echo):
    t.eq(echo(42.5), 42.5)

@t.echo()
def test_roundtrip_str(echo):
    t.eq(echo("spam"), "spam")

@t.echo()
def test_round_trip_None(echo):
    t.eq(echo(None), None)

@t.echo()
def test_roundtrip_True(echo):
    t.eq(echo(True), True)

@t.echo()
def test_roundtrip_False(echo):
    t.eq(echo(False), False)

@t.echo()
def test_roundtrip_inf(echo):
    t.eq(echo(1E500*1E500), 1E500*1E500)

@t.echo()
def test_roundtrip_nan(echo):
    t.eq(type(echo(1E500/1E500)), float)
    t.ne(echo(1E500/1E500), 1E500/1E500)
