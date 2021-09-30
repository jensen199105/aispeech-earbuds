from reader import reader
from matrix import matrix
from fixed import float2fix16


class fsmn_trans:
    def __init__(self, fn):
        self._bias = None
        self._window = None

        try:
            io = reader(fn)
            io.expect("<Nnet>")

            s = io.expects(["<Splice>", "<AddShift>"])
            if s == "<Splice>":
                io.read_int()
                io.read_int()
                io.expect("[")
                while True:
                    try:
                        io.read_int()
                    except Exception:
                        break
                io.expect("<AddShift>")
            self._bias = matrix(1, io.read_int())
            io.read_int()
            io.expect("<LearnRateCoef>")
            io.read_int()
            io.load_matrix(self._bias)

            io.expect("<Rescale>")
            self._window = matrix(1, io.read_int())
            io.read_int()
            io.expect("<LearnRateCoef>")
            io.read_int()
            io.load_matrix(self._window)

            io.expect("</Nnet>")
        except StopIteration:
            pass


    def dump(self):
        arr = []
        for f in self._bias:
            arr.append(str(float2fix16(f, 10)))
        print("const short trans_bias[] = {%s};" % ",".join(arr))

        arr = []
        for f in self._window:
            arr.append(str(float2fix16(f, 10)))
        print("const short trans_window[] = {%s};" % ",".join(arr))
