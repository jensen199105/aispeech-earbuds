class matrix:
    def __init__(self, rows, cols):
        self.rows = rows
        self.cols = cols
        self.size = rows * cols
        self._data = [0.0] * self.size

    
    def __getitem__(self, index):
        return self._data[index]
    

    def __setitem__(self, index, value):
        self._data[index] = value