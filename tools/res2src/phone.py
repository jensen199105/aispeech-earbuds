class phone:
    def __init__(self, fn):
        self._char_array = []
        self._phones_array = []

        fp = open(fn, "r")
        lines = fp.readlines()
        fp.close()

        for line in lines:
            line = line.strip()
            if len(line) == 0:
                continue

            arr = line.split(" ")
            self._char_array.append("\"%s\"" % arr[0])
            self._phones_array.append("\"%s\"" % " ".join(arr[1:]))


    def dump(self):
        print("const short char_count = %d;" % len(self._char_array))
        print("const char* char_array[] = {%s};" % ",".join(self._char_array))
        print("const char* phones_array[] = {%s};" % ",".join(self._phones_array))
