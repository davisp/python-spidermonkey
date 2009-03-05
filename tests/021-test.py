import t
@t.echo("roundtrip false")
def test(echo):
    t.eq(echo(False), False)
