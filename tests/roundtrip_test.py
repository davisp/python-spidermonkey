import unittest
import spidermonkey

class RoundtripTest(unittest.TestCase):
    def setUp(self):
        rt = spidermonkey.Runtime()
        self.cx = rt.create_context()
        self.x = []
        def echo(arg):
            self.x.append(arg)
            return arg
        self.cx.bind(u"echo", echo)

    def check(self, script, arg):
        ret = self.cx.execute(script)
        self.assertEqual(self.cx.execute(script), arg)
        self.assertEqual(self.x.pop(), arg)

    def test_primitive_types(self):
        self.check(u"echo(42);", 42)
        self.check(u"echo(42.5);", 42.5)
        self.check(u'echo("spam");', u"spam")
        self.check(u"echo(undefined);", None)
        self.check(u"echo(null);", None)
        self.check(u"echo(true);", True)
        self.check(u"echo(false);", False)
        #self.check("echo(Infinity);", XXX)
        #self.check("echo(NaN);", XXX)

if __name__ == '__main__':
    unittest.main()
