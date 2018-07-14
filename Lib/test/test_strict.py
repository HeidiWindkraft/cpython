# Test various flavors of legal and illegal future statements

from functools import partial
import unittest
from test import support
from textwrap import dedent
import os
import re
import sys

class StrictTest(unittest.TestCase):

    def check_syntax_error(self, err, basename, lineno):
        self.assertIn('%s.py, line %d' % (basename, lineno), str(err))
        self.assertEqual(os.path.basename(err.filename), basename + '.py')
        self.assertEqual(err.lineno, lineno)

    def test_implname(self):
        self.assertEqual(sys.implementation.name, "strict_cpython")

    def test_goodstrict(self):
        with support.CleanImport('goodsyntax_strict'):
            from test import goodsyntax_strict
            self.assertEqual(goodsyntax_strict.result, True)

    def test_badstrict(self):
        with self.assertRaises(SyntaxError) as cm:
            from test import badsyntax_strict_no_unused_vars
        self.check_syntax_error(cm.exception, "badsyntax_strict_no_unused_vars", 13)

if __name__ == "__main__":
    unittest.main()

