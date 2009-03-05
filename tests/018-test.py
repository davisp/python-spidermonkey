import t
@t.echo("roundtrip string")
def test(echo):
    t.eq(echo("spam"), "spam")
