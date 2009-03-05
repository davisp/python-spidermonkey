import t
@t.echo("roundtrip true")
def test(echo):
    t.eq(echo(True), True)
