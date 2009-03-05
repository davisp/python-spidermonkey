import t
@t.echo("roundtrip NaN")
def test(echo):
    t.eq(type(echo(1E500/1E500)), float)
    t.ne(echo(1E500/1E500), 1E500/1E500)
