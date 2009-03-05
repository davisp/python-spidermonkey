import t
@t.echo("roundtrip null")
def test(echo):
    t.eq(echo(None), None)
