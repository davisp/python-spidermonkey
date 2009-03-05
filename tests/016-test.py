import t
@t.echo("roundtrip integer")
def test(echo):
    t.eq(echo(42), 42)
