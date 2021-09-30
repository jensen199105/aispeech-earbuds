from reader import reader
from matrix import matrix


class dnn_layer:
    window = None
    bias = None
    in_cols = 0
    out_cols = 0
    type = ""


class dnn_nnet:
    def __init__(self, fn):
        self._layers = []

        io = reader(fn)

        try:
            while True:
                l = self.load_layer(io)
                self._layers.append(l)
        except StopIteration:
            pass

        self.in_cols = self._layers[0].in_cols
        self.out_cols = self._layers[-1].out_cols


    def load_layer(self, io):
        io.expect("<biasedlinearity>")

        l = dnn_layer()
        l.out_cols = io.read_int()
        l.in_cols = io.read_int()

        io.expect("m")
        io.read_int()
        io.read_int()

        l.window = matrix(l.out_cols, l.in_cols)
        io.load_matrix(l.window, False)

        io.expect("v")
        io.read_int()

        l.bias = matrix(1, l.out_cols)
        io.load_matrix(l.bias, False)

        l.type = io.read_str()
        if l.type != "<sigmoid>" and l.type != "<linear>" and l.type != "<softmax>" and l.type !="<relu>":
            raise SyntaxError("[{}] not support.".format(l.type))

        io.read_int()
        io.read_int()

        return l


    def dump(self):
        def fixed8(f, max):  # Q10, 1+5, 10 => ±2^5 = (-31, 32)
            q = 64.0
            v = int(f/max * q + (f >= 0 and 0.5 or -0.5))
            if v > 63:
                v = 63
            elif v < -64:
                v = -64
            return v


        def fixed32(f):  # Q10, 1+5, 10 => ±2^5 = (-31, 32)
            q = 1<<24
            v = int(f * q + (f >= 0 and 0.5 or -0.5))
            return v


        def max_row(m, maxs):
            for i in range(0, m.rows):
                maxs[i] = 0.0
                for j in range(0, m.cols):
                    if(abs(m[i*m.cols+j]) > maxs[i]):
                        maxs[i] = abs(m[i*m.cols+j])

        max_col = 0
        info = [len(self._layers), self.in_cols, self.out_cols]
        
        for i in range(len(self._layers)):
            l = self._layers[i]

            max_col = max(max_col, l.window.cols)
            info.append(l.window.rows)
            info.append(l.window.cols)

            maxs = matrix(1, l.window.rows)
            max_row(l.window, maxs)

            arr = []
            for f in maxs:
                arr.append(str(fixed32(f)))
            print("int AISPEECH_WKP_MARRAY_%d[] = {%s};" % (i, ",".join(arr)))

            arr = []
            for m in range(0,l.window.rows):
                for n in range(l.window.cols):
                    arr.append(str(fixed8(l.window[m*l.window.cols+n], maxs[m])))
            print("signed char AISPEECH_WKP_WARRAY_%d[] = {%s};" % (i, ",".join(arr)))

            arr = []
            for f in l.bias:
                arr.append(str(fixed32(f)))
            print("int AISPEECH_BARRAY_%d[] = {%s};" % (i, ",".join(arr)))

        info.insert(3, max_col)

        arr = []
        for i in info :
            arr.append(str(i))
        print("short wkp_info[] = {%s};" % ",".join(arr))

        arr = []
        for i in range(len(self._layers)):
            arr.append("AISPEECH_WKP_MARRAY_%d" % i)
        print("int *AISPEECH_PWM[] = {%s};" % ",".join(arr))

        arr = []
        for i in range(len(self._layers)):
            arr.append("AISPEECH_WKP_WARRAY_%d" % i)
        print("signed char *AISPEECH_PW[] = {%s};" % ",".join(arr))

        arr = []
        for i in range(len(self._layers)):
            arr.append("AISPEECH_BARRAY_%d" % i)
        print("int *AISPEECH_PB[] = {%s};" % ",".join(arr))