import re
import math
from fixed import float2fix32


class config:
    def __init__(self, fn):
        self._use_vad = 0
        self._use_fsmn = 0
        self._use_phone = 0
        self._use_cmn = 0
        self._cmn_start_min_frame = 0
        self._cmn_post_update_frame = 0
        self._cmn_left_seek_frame = 0
        self._cmn_min_flush_frame = 0
        self._cmn_win_size = 0
        self._parm_use_freq_direct = 0
        self._parm_window_size = 0
        self._parm_window_step = 0
        self._parm_target_kind = None
        self._parm_use_hamming = 0
        self._parm_use_hanning = 0
        self._parm_fbank_chans = 0
        self._wmax = 0
        self._wmax_eword = 0
        self._wsmooth = 0
        self._wsmooth_eword = 0
        self._space = 0
        self._win = 0;
        self._skip_frame = 0
        self._dict = None
        self._use_e2e = 0
        self._wakeup_skip_win = 0
        # char check
        self._use_char_check = 0
        self._char_score_min_win = 0
        self._char_score_max_win = 0
        self._char_check_words = None
        self._char_score_thresh_array = None
        # search max conf
        self._use_max_conf_search = 0
        self._max_conf_search_words = None
        self._max_conf_search_win_array = None
        self._max_conf_search_max_array = None
        # delay wakeup
        self._use_delay_wakeup = 0
        self._delay_wakeup_search0 = 0
        self._delay_wakeup_search1 = 0
        self._delay_wakeup_search_max = 0
        self._delay_wakeup_thresh_times = 0
        self._delay_wakeup_words = None
        # duration penalty
        self._use_duration_penalty = 0
        self._use_restart_easy_wakeup = 0
        self._score_false_reduce_duration = 0
        self._score_false_reduce_nowakeup_duration = 0
        self._score_easy_wakeup_duration = 0
        self._score_false_reduce_coef = 0
        self._score_easy_wakeup_coef = 0
        self._score_easy_wakeup_nowakeup_coef = 0

        fp = open(fn, "r")
        lines = fp.readlines()

        for line in lines:
            line = line.strip()
            if len(line) == 0 or line.find("#") == 0 or line.find("=") < 0:
                continue
            val = line[line.find("=")+1:]

            if line.find("use_vad") == 0:
                self._use_vad = int(re.sub("\D", "", val))
            elif line.find("use_duration_penalty") == 0:
                self._use_duration_penalty = int(re.sub("\D", "", line))
            elif line.find("use_restart_easy_wakeup") == 0:
                self._use_restart_easy_wakeup = int(re.sub("\D", "", line))
            elif line.find("use_fsmn") == 0:
                self._use_fsmn = int(re.sub("\D", "", val))
            elif line.find("use_phoneme") == 0:
                self._use_phone = int(re.sub("\D", "", val))
            elif line.find("use_cmn") == 0:
                self._use_cmn = int(re.sub("\D", "", val))
            elif line.find("start_min_frame") == 0:
                self._cmn_start_min_frame = int(re.sub("\D", "", val))
            elif line.find("post_update_frame") == 0:
                self._cmn_post_update_frame = int(re.sub("\D", "", val))
            elif line.find("left_seek_frame") == 0:
                self._cmn_left_seek_frame = int(re.sub("\D", "", val))
            elif line.find("min_flush_frame") == 0:
                self._cmn_min_flush_frame = int(self._get_line_val(line))
            elif line.find("win_size") == 0:
                self._cmn_win_size = int(re.sub("\D", "", val))
            elif line.find("USEFREQDATA") == 0:
                self._parm_use_freq_direct = int(re.sub("\D", "", val))
            elif line.find("WINDOWSIZE") == 0:
                self._parm_window_size = int(float(self._get_line_val(line)))
            elif line.find("TARGETRATE") == 0:
                self._parm_window_step = int(float(self._get_line_val(line)))
            elif line.find("TARGETKIND") == 0:
                self._parm_target_kind = self._get_line_val(line)
            elif line.find("USEHAMMING") == 0:
                self._parm_use_hamming = int(re.sub("\D", "", val))
            elif line.find("USEHANNING") == 0:
                self._parm_use_hanning = int(re.sub("\D", "", val))
            elif line.find("NUMCHANS") == 0:
                self._parm_fbank_chans = int(re.sub("\D", "", val))
            elif line.find("wmax_eword") == 0:
                self._wmax_eword = int(re.sub("\D", "", val))
            elif line.find("wmax") == 0:
                self._wmax = int(re.sub("\D", "", val))
            elif line.find("wsmooth_eword") == 0:
                self._wsmooth_eword = int(re.sub("\D", "", val))
            elif line.find("wsmooth") == 0:
                self._wsmooth = int(re.sub("\D", "", val))
            elif line.find("space") == 0:
                self._space = int(re.sub("\D", "", val))
            elif line.find("win") == 0:
                self._win = int(re.sub("\D", "", val))
            elif line.find("skip_frame") == 0:
                self._skip_frame = int(re.sub("\D", "", val))
            elif line.find("dict") == 0:
                self._dict = val.strip("[").strip("];\n").split(",")
            elif line.find("use_e2e") == 0:
                self._use_e2e = int(re.sub("\D", "", val))
            elif line.find("wakeup_skip_win") == 0:
                self._wakeup_skip_win = int(re.sub("\D", "", val))
            elif line.find("use_char_check") == 0:
                self._use_char_check = int(re.sub("\D", "", val))
            elif line.find("char_score_min_win") == 0:
                self._char_score_min_win = int(re.sub("\D", "", val))
            elif line.find("char_score_max_win") == 0:
                self._char_score_max_win = int(re.sub("\D", "", val))
            elif line.find("char_check_words") == 0:
                self._char_check_words = val.strip("[").strip("];\n").split(",")
                for i in range(len(self._char_check_words)):
                    self._char_check_words[i] = self._char_check_words[i].strip();
            elif line.find("char_score_thresh_array") == 0:
                self._char_score_thresh_array = val.strip("[").strip("];\n").split(",")
            elif line.find("use_max_conf_search") == 0:
                self._use_max_conf_search = int(re.sub("\D", "", val))
            elif line.find("max_conf_search_words") == 0:
                self._max_conf_search_words = val.strip("[").strip("];\n").split(",")
            elif line.find("max_conf_search_win_array") == 0:
                self._max_conf_search_win_array = val.strip("[").strip("];\n").split(",")
            elif line.find("max_conf_search_max_array") == 0:
                self._max_conf_search_max_array = val.strip("[").strip("];\n").split(",")
            elif line.find("use_delay_wakeup") == 0:
                self._use_delay_wakeup = int(re.sub("\D", "", val))
            elif line.find("delay_wakeup_search0") == 0:
                self._delay_wakeup_search0 = int(re.sub("\D", "", val))
            elif line.find("delay_wakeup_search1") == 0:
                self._delay_wakeup_search1 = int(re.sub("\D", "", val))
            elif line.find("delay_wakeup_search_max") == 0:
                self._delay_wakeup_search_max = int(re.sub("\D", "", val))
            elif line.find("delay_wakeup_thresh_times") == 0:
                self._delay_wakeup_thresh_times = int(re.sub("\D", "", val))
            elif line.find("delay_wakeup_words") == 0:
                self._delay_wakeup_words = val.strip("[").strip("];\n").split(",")
            elif line.find("score_false_reduce_duration") == 0:
                self._score_false_reduce_duration = int(re.sub("\D", "", line))
            elif line.find("score_false_reduce_nowakeup_duration") == 0:
                self._score_false_reduce_nowakeup_duration = int(re.sub("\D", "", line))
            elif line.find("score_easy_wakeup_duration") == 0:
                self._score_easy_wakeup_duration = int(re.sub("\D", "", line))
            elif line.find("score_false_reduce_coef") == 0:
                self._score_false_reduce_coef = float(self._get_line_val(line))
            elif line.find("score_easy_wakeup_coef") == 0:
                self._score_easy_wakeup_coef = float(self._get_line_val(line))
            elif line.find("score_easy_wakeup_nowakeup_coef") == 0:
                self._score_easy_wakeup_nowakeup_coef = float(self._get_line_val(line))

        if not self._use_phone:
            if not self._wmax:
                self._wmax = 20
            if not self._wsmooth:
                self._wsmooth = 10
        else:
            if not self._wmax_eword:
                self._wmax_eword = 25
            if not self._wsmooth_eword:
                self._wsmooth_eword = 1
        if not self._char_check_words:
            self._char_check_words = []
        if not self._max_conf_search_words:
            self._max_conf_search_words = []
        if not self._delay_wakeup_words:
            self._delay_wakeup_words = []

        if not self._cmn_start_min_frame:
            self._cmn_start_min_frame = 15;
        if not self._cmn_post_update_frame:
            self._cmn_post_update_frame = 15
        if not self._cmn_left_seek_frame:
            self._cmn_left_seek_frame = 15
        if not self._cmn_min_flush_frame:
            self._cmn_min_flush_frame = -1
        if not self._cmn_win_size:
            self._cmn_win_size = 31

        if not self._parm_window_size:
            self._parm_window_size = 300000
        if not self._parm_window_step:
            self._parm_window_step = 200000
        if not self._parm_fbank_chans:
            self._parm_fbank_chans = 24
        if not self._parm_target_kind:
            self._parm_target_kind = "FBANK_Z"

        if self._parm_use_freq_direct:
            if self._parm_window_step == 160000:
                self._parm_window_size = int(self._parm_window_step*257*8/512)
                self._parm_window_step = int(self._parm_window_step*257*8/512)
            else:
                raise Exception("only support 16ms frame size while frequency direct is enabled")

            self._parm_use_hamming = 0
            self._parm_use_hanning = 0


    def _get_line_val(self, line):
        val = line[line.index("=")+1:]
        val = val[0:val.index(";")]
        return val.strip()


    def use_fsmn(self):
        return self._use_fsmn


    def use_phone(self):
        return self._use_phone


    def use_e2e(self):
        return self._use_e2e


    def use_hamming(self):
        return self._parm_use_hamming


    def use_hanning(self):
        return self._parm_use_hanning


    def use_char_check(self):
        return self._use_char_check


    def use_max_conf_search(self):
        return self._use_max_conf_search


    def use_delay_wakeup(self):
        return self._use_delay_wakeup


    def use_duration_penalty(self):
        return self._use_duration_penalty


    def fank_chans(self):
        return self._parm_fbank_chans


    def win_size(self):
        return self._parm_window_size


    def dump(self):
        print("const unsigned int AISPEECH_ARRAY_CFG[] = {%d,%d,%d,%d,%d,%d,%d,%d,%d};" %
              (self._use_vad, self._use_cmn, self._wsmooth, self._wmax, len(self._dict),
               self._use_char_check, len(self._char_check_words),
               self._use_max_conf_search, len(self._max_conf_search_words)))
        print("const unsigned AISPEECH_SPACE = %d;" % self._space)
        print("const char *dict[] = {%s};" % ",".join(self._dict))
        print("const short dict_count = %d;" % len(self._dict))
        if self._wsmooth_eword != 0 and self._wsmooth_eword != 1:
            raise Exception("value of wsmooth_eword(%d) is not supported by lite wakeup currently" % self._wsmooth_eword)
        print("const char wmax_eword = %d;" % self._wmax_eword)
        print("const char use_e2e = %d;" % self._use_e2e)
        print("const char wakeup_skip_win = %d;" % self._wakeup_skip_win)

        if self._use_char_check:
            print("const int char_score_min_win = %d;" % self._char_score_min_win)
            print("const int char_score_max_win = %d;" % self._char_score_max_win)
            print("const char *char_check_words[] = {%s};" % ",".join(self._char_check_words))

            offset = 0
            char_check_idx = []
            for word in self._char_check_words:
                char_check_idx.append(str(offset))
                offset += len(word.split(" "))
            print("const short char_check_idx[] = {%s};" % ",".join(char_check_idx))

            char_score_thresh_array = []
            for thresh in self._char_score_thresh_array:
                char_score_thresh_array.append(str(float2fix32(float(thresh), 31)))
            print("const int char_score_thresh_array[] = {%s};" % ",".join(char_score_thresh_array))
        else:
            print("const int char_score_max_win = 0;")
            print("const int char_score_min_win = 0;")
            print("const char **char_check_words = 0;")
            print("const short *char_check_idx = 0;")
            print("const int char_score_thresh_array[] = {0};")

        if self._use_max_conf_search:
            print("const char *max_conf_search_words[] = {%s};" % ",".join(self._max_conf_search_words))

            offset = 0
            max_conf_search_idx = []
            for word in self._max_conf_search_words:
                max_conf_search_idx.append(str(offset))
                offset += 1
            print("const short max_conf_search_idx[] = {%s};" % ",".join(max_conf_search_idx))
            print("const char max_conf_search_win_array[] = {%s};" % ",".join(self._max_conf_search_win_array))
            print("const char max_conf_search_max_array[] = {%s};" % ",".join(self._max_conf_search_max_array))
        else:
            print("const char **max_conf_search_words = 0;")
            print("const short *max_conf_search_idx = 0;")
            print("const short *max_conf_search_win_array = 0;")
            print("const short *max_conf_search_max_array = 0;")

        if self._use_delay_wakeup:
            print("const char use_delay_wakeup = 1;")
            print("const short delay_wakeup_search0 = %d;" % self._delay_wakeup_search0)
            print("const short delay_wakeup_search1 = %d;" % self._delay_wakeup_search1)
            print("const short delay_wakeup_search_max = %d;" % self._delay_wakeup_search_max)
            print("const int delay_wakeup_thresh_times = %d;" % float2fix32(math.log(self._delay_wakeup_thresh_times, math.e), 26))
            print("const short delay_wakeup_word_count = %d;" % len(self._delay_wakeup_words))
            print("const char *delay_wakeup_words[] = {%s};" % ",".join(self._delay_wakeup_words))
        else:
            print("const char use_delay_wakeup = 0;")
            print("const short delay_wakeup_search0 = 0;")
            print("const short delay_wakeup_search1 = 0;")
            print("const short delay_wakeup_search_max = 0;")
            print("const int delay_wakeup_thresh_times = 0;")
            print("const short delay_wakeup_word_count = 0;")
            print("const char **delay_wakeup_words = 0;")

        print("")
        print("const short cmn_start_min_frame = %d;" % self._cmn_start_min_frame)
        print("const short cmn_post_update_frame = %d;" % self._cmn_post_update_frame)
        print("const short cmn_left_seek_frame = %d;" % self._cmn_left_seek_frame)
        print("const short cmn_min_flush_frame = %d;" % self._cmn_min_flush_frame)
        print("const short cmn_win_size = %d;" % self._cmn_win_size)

        print("")
        print("const int parm_window_size = %d;" % self._parm_window_size)
        print("const int parm_window_step = %d;" % self._parm_window_step)
        print("const char* parm_target_kind = \"%s\";" % self._parm_target_kind)
        print("const char parm_use_hamming = %d;" % self._parm_use_hamming)
        print("const char parm_use_hanning = %d;" % self._parm_use_hanning)
        print("const char parm_fbank_chans = %d;" % self._parm_fbank_chans)

        print("")
        if self._use_duration_penalty:
            print("const char use_duration_penalty = 1;")
            print("const char use_restart_easy_wakeup = %d;" % self._use_restart_easy_wakeup)
            print("const int score_false_reduce_duration = %d;" % self._score_false_reduce_duration)
            print("const int score_false_reduce_nowakeup_duration = %d;" % self._score_false_reduce_nowakeup_duration)
            print("const int score_easy_wakeup_duration = %d;" % self._score_easy_wakeup_duration)
            print("const int score_false_reduce_coef = %d;" % float2fix32(math.log(self._score_false_reduce_coef,math.e), 26))
            print("const int score_easy_wakeup_coef = %d;" % float2fix32(math.log(self._score_easy_wakeup_coef,math.e), 26))
            print("const int score_easy_wakeup_nowakeup_coef = %d;" % float2fix32(math.log(self._score_easy_wakeup_nowakeup_coef,math.e), 26))
        else:
            print("const char use_duration_penalty = 0;")
            print("const char use_restart_easy_wakeup = 0;")
            print("const int score_false_reduce_duration = 0;")
            print("const int score_false_reduce_nowakeup_duration = 0;")
            print("const int score_easy_wakeup_duration = 0;")
            print("const int score_false_reduce_coef = 0;")
            print("const int score_easy_wakeup_coef = 0;")
            print("const int score_easy_wakeup_nowakeup_coef = 0;")

        if self._use_fsmn:
            print("")
            print("const char fsmn_win = %d;" % self._win)
            print("const char fsmn_skip_frame = %d;" % self._skip_frame)
        else:
            print("")
            print("const char dnn_win = %d;" % self._win)
            print("const char dnn_skip_frame = %d;" % self._skip_frame)
