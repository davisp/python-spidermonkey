import t
@t.echo("roundtrip float")
def test(echo):
    t.eq(echo(42.5), 42.5)
