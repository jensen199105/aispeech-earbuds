def _float2fixed(f, q, bits):
    v = int(round(f * (1 << q)))
    if v > (1 << (bits-1)) - 1:
        v = (1 << (bits-1)) - 1
    elif v < -(1 << (bits-1)):
        v = -(1 << (bits-1))
    return v


def float2fix8(f, q):
    return _float2fixed(f, q, 8)


def float2fix16(f, q):
    return _float2fixed(f, q, 16)


def float2fix32(f, q):
    return _float2fixed(f, q, 32)