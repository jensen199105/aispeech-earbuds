from reader import reader
from matrix import matrix
from fixed import float2fix16


class dnn_trans:
    def __init__(self, fn):
        try:
            self._bias = None
            self._window = None

            io = reader(fn)

            io.expect("<bias>")
            self._bias = matrix(1, io.read_int())
            io.read_int()
            io.expect("v")
            io.read_int()
            io.load_matrix(self._bias, False)

            io.expect("<window>")
            self._window = matrix(1, io.read_int())
            io.read_int()
            io.expect("v")
            io.read_int()
            io.load_matrix(self._window, False)
        except StopIteration:
            pass


    def dump(self):
        arr = []
        for i in range(self._bias.size):
            if (i < 264):
                arr.append(str(float2fix16(self._bias[i], 10)))
            elif(i >= 264 and i < 528):
                arr.append(str(float2fix16(self._bias[i], 10) * 10))
            else:
                arr.append(str(float2fix16(self._bias[i], 10) * 100))
        print("short bias[] = {%s};" % ",".join(arr))

        arr = []
        for f in self._window:
            arr.append(str(float2fix16(f, 10)))
        print("short window[] = {%s};" % ",".join(arr))