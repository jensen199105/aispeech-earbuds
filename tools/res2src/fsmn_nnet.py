from reader import reader
from matrix import matrix
from fixed import float2fix8, float2fix16


class fsmn_layer:
    dense1 = None
    bias = None
    dense2 = None
    filter = None
    in_dim = None
    out_dim = None
    l_order = None
    r_order = None
    skip = None


class fsmn_nnet:
    def __init__(self, fn):
        self._in_cols = 0
        self._out_cols = 0
        self._dense = None
        self._bias = None
        self._linear = None
        self._layers = []

        io = reader(fn)

        try:
            io.expect("<Nnet>")

            key = io.read_str()
            if key == "<LinearTransform>":
                self.load_linear(io)
                key = io.read_str()

            while True:
                if key != "<Dfsmn>":
                    break
                layer = self.load_layer(io)
                self._layers.append(layer)
                key = io.read_str()

            if key != "<AffineTransform>":
                raise SyntaxError("[{}] not support.".format(key))

            self._in_cols = self._layers[0].in_dim
            self._out_cols = io.read_int()
            in_dim = io.read_int()

            io.expect("<LearnRateCoef>")
            io.read_int()
            io.expect("<BiasLearnRateCoef>")
            io.read_int()
            io.expect("<MaxNorm>")
            io.read_int()

            self._dense = matrix(self._out_cols, in_dim)
            io.load_matrix(self._dense)

            self._bias = matrix(1, self._out_cols)
            io.load_matrix(self._bias)

            io.expect("<Softmax>")
            io.read_int()
            io.read_int()
            io.expect("</Nnet>")
        except StopIteration as e:
            pass


    def load_linear(self, io):
        rows = io.read_int()
        cols = io.read_int()
        io.expect("<LearnRateCoef>")
        io.read_int()
        io.expect("<ClipGradient>")
        io.read_int()
        self._linear = matrix(rows, cols)
        io.load_matrix(self._linear)


    def load_layer(self, io):
        layer = fsmn_layer()

        layer.out_dim = io.read_int()
        layer.in_dim = io.read_int()

        io.expect("<LOrder>")
        layer.l_order = io.read_int()
        io.expect("<ROrder>")
        layer.r_order = io.read_int()
        io.expect("<LStride>")
        io.read_int()
        io.expect("<RStride>")
        io.read_int()
        io.expect("<HidSize>")
        hidsize = io.read_int()
        io.expect("<LearnRateCoef>")
        io.read_int()
        io.expect("<BiasLearnRateCoef>")
        io.read_int()
        io.expect("<MaxNorm>")
        io.read_int()
        io.expect("<ClipGradient>")
        io.read_int()

        s = io.expects(["<ClipWeight>", "<Skip>"])
        if s == "<ClipWeight>":
            io.read_int()
            io.expect("<Skip>")
        layer.skip = io.read_int()

        # filter
        layer.filter = matrix(layer.out_dim, layer.l_order+layer.r_order+1)
        io.load_matrix(layer.filter)
        # dense1
        layer.dense1 = matrix(hidsize, layer.in_dim)
        io.load_matrix(layer.dense1)
        # bias
        layer.bias = matrix(1, hidsize)
        io.load_matrix(layer.bias)
        # dense2
        layer.dense2 = matrix(layer.out_dim, hidsize)
        io.load_matrix(layer.dense2)

        return layer


    def dump(self):
        max_col = 0
        max_history_frame = 0
        max_history_col = 0

        for i in range(len(self._layers)):
            l = self._layers[i]
            max_col = max(max_col, l.dense1.cols, l.dense2.cols)
            max_history_frame = max(max_history_frame, l.l_order+l.r_order)
            max_history_col = max(max_history_col, l.dense2.rows)

        print("const char fsmn_layer_num = %d;" % len(self._layers))
        print("const short fsmn_in_col = %d;" % self._in_cols)
        print("const short fsmn_out_col = %d;" % self._out_cols)
        print("const short fsmn_max_col = %d;" % max_col)
        print("const short fsmn_max_history_frame = %d;" % max_history_frame)
        print("const short fsmn_max_history_col = %d;" % max_history_col)
        print("")

        arr = []
        for f in self._bias:
            arr.append(str(float2fix16(f, 12)))
        print("const short fsmn_bias2[] = {%s};" % ",".join(arr))

        arr = []
        for f in self._dense:
            arr.append(str(float2fix16(f, 12)))
        print("const short fsmn_dense2_shape[] = {%d,%d};" % (self._dense.rows, self._dense.cols))
        print("const short fsmn_dense2[] = {%s};" % ",".join(arr))
        print("")

        if self._linear:
            arr = []
            for f in self._linear:
                arr.append(str(float2fix16(f, 12)))
            print("const short fsmn_linear_shape[] = {%d,%d};" % (self._linear.rows, self._linear.cols))
            print("const short fsmn_linear[] = {%s};" % ",".join(arr))
        else:
            print("const short fsmn_linear_shape[] = {0,0};")
            print("const short *fsmn_linear = 0;")
        print("")

        fsmn_layerX_in_dims = []
        fsmn_layerX_out_dims = []
        fsmn_layerX_history_frames = []
        fsmn_layerX_dense1_shapes = []
        fsmn_layerX_dense2_shapes = []
        fsmn_layerX_filter_shapes = []
        fsmn_layerX_biases = []
        fsmn_layerX_dense1s = []
        fsmn_layerX_dense2s = []
        fsmn_layerX_filters = []

        for i in range(len(self._layers)):
            l = self._layers[i]

            fsmn_layerX_in_dims.append(str(l.in_dim))
            fsmn_layerX_out_dims.append(str(l.out_dim))
            fsmn_layerX_history_frames.append(str(l.l_order+l.r_order))
            fsmn_layerX_dense1_shapes.append(str(l.dense1.rows))
            fsmn_layerX_dense1_shapes.append(str(l.dense1.cols))
            fsmn_layerX_dense2_shapes.append(str(l.dense2.rows))
            fsmn_layerX_dense2_shapes.append(str(l.dense2.cols))
            fsmn_layerX_filter_shapes.append(str(l.filter.rows))
            fsmn_layerX_filter_shapes.append(str(l.filter.cols))
            
            arr = []
            for f in l.bias:
                arr.append(str(float2fix16(f, 12)))
            fsmn_layerX_biases.append(arr)
            
            arr = []
            for f in l.dense1:
                arr.append(str(float2fix8(f, 7)))
            fsmn_layerX_dense1s.append(arr)
            
            arr = []
            for f in l.dense2:
                arr.append(str(float2fix8(f, 7)))
            fsmn_layerX_dense2s.append(arr)
            
            arr = []
            for f in l.filter:
                arr.append(str(float2fix8(f, 7)))
            fsmn_layerX_filters.append(arr)

        print("const short fsmn_layerX_in_dims[] = {%s};" % ",".join(fsmn_layerX_in_dims))
        print("const short fsmn_layerX_out_dims[] = {%s};" % ",".join(fsmn_layerX_out_dims))
        print("const short fsmn_layerX_history_frames[] = {%s};" % ",".join(fsmn_layerX_history_frames))
        print("const short fsmn_layerX_dense1_shapes[] = {%s};" % ",".join(fsmn_layerX_dense1_shapes))
        print("const short fsmn_layerX_dense2_shapes[] = {%s};" % ",".join(fsmn_layerX_dense2_shapes))
        print("const short fsmn_layerX_filter_shapes[] = {%s};" % ",".join(fsmn_layerX_filter_shapes))

        print("")
        biases = []
        for i in range(len(fsmn_layerX_biases)):
            bias = fsmn_layerX_biases[i]
            biases.append("fsmn_layer%d_bias" % i)
            print("const short fsmn_layer%d_bias[] = {%s};" % (i, ",".join(bias)))
        print("const short* fsmn_layerX_biases[] = {%s};" % ",".join(biases))

        print("")
        dense1s = []
        for i in range(len(fsmn_layerX_dense1s)):
            dense1 = fsmn_layerX_dense1s[i]
            dense1s.append("fsmn_layer%d_dense1" % i)
            print("const char fsmn_layer%d_dense1[] = {%s};" % (i, ",".join(dense1)))
        print("const char* fsmn_layerX_dense1s[] = {%s};" % ",".join(dense1s))

        print("")
        dense2s = []
        for i in range(len(fsmn_layerX_dense2s)):
            dense2 = fsmn_layerX_dense2s[i]
            dense2s.append("fsmn_layer%d_dense2" % i)
            print("const char fsmn_layer%d_dense2[] = {%s};" % (i, ",".join(dense2)))
        print("const char* fsmn_layerX_dense2s[] = {%s};" % ",".join(dense2s))

        print("")
        filters = []
        for i in range(len(fsmn_layerX_filters)):
            filter = fsmn_layerX_filters[i]
            filters.append("fsmn_layer%d_filter" % i)
            print("const char fsmn_layer%d_filter[] = {%s};" % (i, ",".join(filter)))
        print("const char* fsmn_layerX_filters[] = {%s};" % ",".join(filters))
