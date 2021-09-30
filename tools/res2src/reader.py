class reader:
    def __init__(self, fn):
        self._fp = open(fn, "rb")
        self._iter = None


    def _get_str(self):
        for line in self._fp.readlines():
            fields = line.decode().split()
            for field in fields:
                yield field


    def read_str(self):
        if self._iter is None:
            self._iter = self._get_str()
        return next(self._iter)


    def read_int(self):
        if self._iter is None:
            self._iter = self._get_str()
        iter = next(self._iter)
        if iter is not None:
            return int(iter)
        else:
            raise EOFError()


    def read_float(self):
        if self._iter is None:
            self._iter = self._get_str()
        iter = next(self._iter)
        if iter is not None:
            return float(iter)
        else:
            raise EOFError()


    def expect(self, token):
        str = self.read_str()
        if str != token:
            raise SyntaxError("[{}] not support.".format(str))


    def expects(self, tokens):
        str = self.read_str()
        try:
            index = tokens.index(str)
            return tokens[index]
        except:
            raise SyntaxError("[{}] not support.".format(str))


    def load_matrix(self, m, expect=True):
        if expect:
            self.expect("[")
        for i in range(0, m.size):
            m[i] = self.read_float()
        if expect:
            self.expect("]")
