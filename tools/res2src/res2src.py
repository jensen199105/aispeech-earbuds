#!/usr/bin/python3

from os import chdir
from argparse import ArgumentParser
from fsmn_nnet import fsmn_nnet
from fsmn_trans import fsmn_trans
from dnn_nnet import dnn_nnet
from dnn_trans import dnn_trans
from phone import phone
from config import config


__VERSION__ = "002"


def dump_build_config(cfg, fn):
    fp = open(fn, "w+")

    ## CONFIG_LITE-FESPD_FREQOUT_DIRECT=y
    ## CONFIG_WAKEUP_DUMP_SCORE=y

    ## currently enable it always
    print("CONFIG_WAKEUP_NCHANNEL=y", file=fp)
    ## currently disable it always
    print("CONFIG_WAKEUP_USE_VAD=n", file=fp)
    ## currently no reset config in wakeup.cfg.r, enable it always
    print("CONFIG_WAKEUP_RESET=y", file=fp)

    if cfg.use_phone():
        print("CONFIG_WAKEUP_DICT_PHONEME=y", file=fp)
        print("CONFIG_WAKEUP_DICT_WORD=n", file=fp)
        print("CONFIG_WAKEUP_DICT_CHAR=n", file=fp)
    elif cfg.use_e2e():
        print("CONFIG_WAKEUP_DICT_PHONEME=n", file=fp)
        print("CONFIG_WAKEUP_DICT_WORD=y", file=fp)
        print("CONFIG_WAKEUP_DICT_CHAR=n", file=fp)
    else:
        print("CONFIG_WAKEUP_DICT_PHONEME=n", file=fp)
        print("CONFIG_WAKEUP_DICT_WORD=n", file=fp)
        print("CONFIG_WAKEUP_DICT_CHAR=y", file=fp)

    if cfg.use_fsmn():
        print("CONFIG_WAKEUP_NN_FSMN=y", file=fp)
        print("CONFIG_WAKEUP_NN_DNN=n", file=fp)
    else:
        print("CONFIG_WAKEUP_NN_FSMN=n", file=fp)
        print("CONFIG_WAKEUP_NN_DNN=y", file=fp)

    if cfg.fank_chans() == 24:
        print("CONFIG_WAKEUP_FBANK_CHANS24=y", file=fp)
        print("CONFIG_WAKEUP_FBANK_CHANS40=n", file=fp)
    elif cfg.fank_chans() == 40:
        print("CONFIG_WAKEUP_FBANK_CHANS24=n", file=fp)
        print("CONFIG_WAKEUP_FBANK_CHANS40=y", file=fp)

    if cfg.use_hamming():
        if cfg.win_size() == 250000:
            print("CONFIG_WAKEUP_HAMMING400=y", file=fp)
            print("CONFIG_WAKEUP_HAMMING480=n", file=fp)
            print("CONFIG_WAKEUP_HANNING512=n", file=fp)
        elif cfg.win_size() == 300000:
            print("CONFIG_WAKEUP_HAMMING400=n", file=fp)
            print("CONFIG_WAKEUP_HAMMING480=y", file=fp)
            print("CONFIG_WAKEUP_HANNING512=n", file=fp)
    elif cfg.use_hanning():
        if cfg.win_size() == 320000:
            print("CONFIG_WAKEUP_HAMMING400=n", file=fp)
            print("CONFIG_WAKEUP_HAMMING480=n", file=fp)
            print("CONFIG_WAKEUP_HANNING512=y", file=fp)

    if cfg.use_char_check():
        print("CONFIG_WAKEUP_XCHECK=y", file=fp)
    else:
        print("CONFIG_WAKEUP_XCHECK=n", file=fp)

    if cfg.use_max_conf_search():
        print("CONFIG_WAKEUP_XTERM=y", file=fp)
    else:
        print("CONFIG_WAKEUP_XTERM=n", file=fp)

    if cfg.use_delay_wakeup():
        print("CONFIG_WAKEUP_DELAY=y", file=fp)
    else:
        print("CONFIG_WAKEUP_DELAY=n", file=fp)

    if cfg.use_duration_penalty():
        print("CONFIG_WAKEUP_DUR_PENALTY=y", file=fp)
    else:
        print("CONFIG_WAKEUP_DUR_PENALTY=n", file=fp)

    fp.close()


if __name__ == "__main__":
    parse = ArgumentParser()
    parse.add_argument("--version", action="version", version=__VERSION__)
    parse.add_argument("--directory", metavar="DIR", default=".", help="set current working directory to DIR (default: %(default)s)")
    parse.add_argument("--wakeup", metavar="WAKEUP", default="wakeup.cfg.r", help="use WAKEUP as wakeup config file (default: %(default)s)")
    parse.add_argument("--transform", metavar="TRANS", default="final.feature_transform", help="use TRNAS as feature transform parameter file (default: %(default)s)")
    parse.add_argument("--nnet", metavar="NNET", default="final.nnet.txt", help="use NNET as neural net parameter file (default: %(default)s)")
    parse.add_argument("--dict", metavar="DICT", default="dict", help="use dict as char-phoneme mapping file (default: %(default)s)")
    parse.add_argument("--config", metavar="CONFIG", help="generate make config and save to file CONFIG (default: don't gernerate)")

    args = parse.parse_args()
    chdir(args.directory)

    print("const char *res_fmt_ver = \"%s\";" % __VERSION__)
    print("")

    cfg = config(args.wakeup)
    cfg.dump()
    print("")

    if cfg.use_phone():
        print("const char use_phoneme = 1;")
        ph = phone(args.dict)
        ph.dump()
    else:
        print("const char use_phoneme = 0;")
        print("const short char_count = 0;")
        print("const char **char_array = 0;")
        print("const char **phones_array = 0;")
    print("")

    if cfg.use_fsmn():
        trans = fsmn_trans(args.transform)
    else:
        trans = dnn_trans(args.transform)
    trans.dump()
    print("")

    if cfg.use_fsmn():
        nn = fsmn_nnet(args.nnet)
    else:
        nn = dnn_nnet(args.nnet)
    nn.dump()
    print("")

    if args.config:
        dump_build_config(cfg, args.config)
