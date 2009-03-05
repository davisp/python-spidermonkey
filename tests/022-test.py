import t
@t.echo("roundtrip Infinity")
def test(echo):
    t.eq(echo(1E500*1E500), 1E500*1E500)
