use_dnn_wakeup = 1;
use_vad=0;
use_decoder=0;
decoder={
     ctc_fn = ${pwd}/pinyin.id.durat.loop.out.prior;
     dict_fn = ${pwd}/lexicon.txt;
     osym_fn = ${pwd}/final.outsyms;
     filler_fn = ${pwd}/filler.list;
     beam = 6.0;
     weight = -0.5;
     ac_scale = 0.4;
     is_bin = 0;
     rec_filler = 0;
};
dnnwakeup={
	vad={
dnnvad={
    parm={
        TARGERRORMAT = HTK;
        SOURCEFORMAT = WAV;
        SOURCERATE   = 625.0;
        SOURCEKIND   = WAVEFORM;
        TARGETRATE   = 200000.0;
        TARGETKIND   = FBANK_D;
        SAVECOMPRESSED = 0;
        SAVEWITHCRC = 0;
        ZMEANSOURCE = 1;
        WINDOWSIZE = 300000.0;

        USEHAMMING = 1;
        PREEMCOEF = 0.97;
        NUMCHANS = 24;
        ESCALE = 1.0;
        SILFLOOR = 50.0;
        ENORMALISE = 0;
        USEPOWER = 1;

        use_dnn=1;
        dnn={
            win=5;
            use_cblas=1;
            use_linear_output=0;
            data_type=2;
            net_fn=${pwd}/vad.nnet.8bit.bin;
            trans_fn=${pwd}/vad.transf.8bit.bin;
            is_bin=1;
            skip_frame=0;
            cache_size=1;
        };
    };
    sil_thresh=-0.3;
    siltrap=10;
    speechtrap=13;
};
use_dnn=1;
left_margin=15;
right_margin=1;
min_speech=0;
version=wakeup_aifar_vad.v0.6;
	};
    parm={
        SOURCEKIND = WAVEFORM;
        SOURCEFORMAT = WAV;
        SOURCERATE = 625.0;
        TARGETFORMAT = HTK;
        TARGETKIND   = FBANK_Z;
        TARGETRATE = 200000.0;
        ZMEANSOURCE = 1;
        WINDOWSIZE = 300000.0;
        SAVECOMPRESSED = 0;
        SAVEWITHCRC = 0;
        USEHAMMING = 1;
        PREEMCOEF = 0.97;
        NUMCHANS = 24;
        ESCALE = 1.0;
        ENORMALISE = 0;
        USEPOWER = 1;

        use_z=0;
        use_cmn=1;
        zmean={
            #start_min_frame=12;
            post_update_frame=15;
#            smooth=1;
            left_seek_frame=15;
#            min_flush_frame=-1;
        };
        use_fsmn=1;
        fsmn={
			win=2;
			skip_frame=0;
                        inner_skip_frame=0;
			cache_size=1;
			nnet_fn=${pwd}/nnet.8bit.bin;
			trans_fn=${pwd}/transf.8bit.bin;
			is_bin=1;
        };
    };
	use_e2e = 1;
	wakeup_skip_win = 20;
#    use_sil_restart=1;
#    sil_min_thresh=0.9;
    use_progressive_smooth = 1;
    wsmooth=7;
    wmax=17;
#    smooth_array=[10,10,10,7,7,10];  #dict中每个字的平滑窗长
#     max_array=[30,30,30,30,30,30];  #每个字的最大窗长,例如 你好小乐  你：15*3； 好：30*2； 小：45*1； 乐：0
#     max_win=30;   #上述两项的最大值
#     use_diff_win=1; #使能这种机制
    #dict=["sil","wanmoerji","bofangyinyue","zantingbofang","tingzhibofang","shangyishou","xiayishou","zengdayinliang","jianxiaoyinliang","jietingdianhua","guaduandianhua"];   #测试报段错误
    dict=["sil","wan mo er ji","bo fang yin yue","zhan ting bo fang","ting zhi bo fang","shang yi shou","xia yi shou","zeng da yin liang","jian xiao yin liang","jie ting dian hua","gua duan dian hua"];
    #    dict=["sil", "a", "ai", "an", "ang", "ao", "ba", "bai", "ban", "bang", "bao", "bei", "ben", "beng", "bi", "bian", "biao", "bie", "bin", "bing", "bo", "bu", "ca", "cai", "can", "cang", "cao", "ce", "cen", "ceng", "cha", "chai", "chan", "chang", "chao", "che", "chen", "cheng", "chi", "chong", "chou", "chu", "chuai", "chuan", "chuang", "chui", "chun", "chuo", "ci", "cong", "cou", "cu", "cuan", "cui", "cun", "cuo", "da", "dai", "dan", "dang", "dao", "de", "dei", "den", "deng", "di", "dia", "dian", "diao", "die", "ding", "diu", "dong", "dou", "du", "duan", "dui", "dun", "duo", "e", "ei", "en", "er", "fa", "fan", "fang", "fei", "fen", "feng", "fo", "fou", "fu", "ga", "gai", "gan", "gang", "gao", "ge", "gei", "gen", "geng", "gong", "gou", "gu", "gua", "guai", "guan", "guang", "gui", "gun", "guo", "ha", "hai", "han", "hang", "hao", "he", "hei", "hen", "heng", "hng", "hong", "hou", "hu", "hua", "huai", "huan", "huang", "hui", "hun", "huo", "ji", "jia", "jian", "jiang", "jiao", "jie", "jin", "jing", "jiong", "jiu", "juan", "jue", "jun", "jv", "ka", "kai", "kan", "kang", "kao", "ke", "ken", "keng", "kong", "kou", "ku", "kua", "kuai", "kuan", "kuang", "kui", "kun", "kuo", "la", "lai", "lan", "lang", "lao", "le", "lei", "leng", "li", "lia", "lian", "liang", "liao", "lie", "lin", "ling", "liu", "lo", "long", "lou", "lu", "luan", "lue", "lun", "luo", "lv", "ma", "mai", "man", "mang", "mao", "me", "mei", "men", "meng", "mi", "mian", "miao", "mie", "min", "ming", "miu", "mo", "mou", "mu", "na", "nai", "nan", "nang", "nao", "ne", "nei", "nen", "neng", "ng", "ni", "nian", "niang", "niao", "nie", "nin", "ning", "niu", "nong", "nou", "nu", "nuan", "nue", "nuo", "nv", "o", "ou", "pa", "pai", "pan", "pang", "pao", "pei", "pen", "peng", "pi", "pian", "piao", "pie", "pin", "ping", "po", "pou", "pu", "qi", "qia", "qian", "qiang", "qiao", "qie", "qin", "qing", "qiong", "qiu", "quan", "que", "qun", "qv", "ran", "rang", "rao", "re", "ren", "reng", "ri", "rong", "rou", "ru", "ruan", "rui", "run", "ruo", "sa", "sai", "san", "sang", "sao", "se", "sen", "seng", "sha", "shai", "shan", "shang", "shao", "she", "shei", "shen", "sheng", "shi", "shou", "shu", "shua", "shuai", "shuan", "shuang", "shui", "shun", "shuo", "si", "song", "sou", "su", "suan", "sui", "sun", "suo", "ta", "tai", "tan", "tang", "tao", "te", "tei", "teng", "ti", "tian", "tiao", "tie", "ting", "tong", "tou", "tu", "tuan", "tui", "tun", "tuo", "wa", "wai", "wan", "wang", "wei", "wen", "weng", "wo", "wu", "xi", "xia", "xian", "xiang", "xiao", "xie", "xin", "xing", "xiong", "xiu", "xuan", "xue", "xun", "xv", "ya", "yan", "yang", "yao", "ye", "yi", "yin", "ying", "yo", "yong", "you", "yu", "yuan", "yue", "yun", "za", "zai", "zan", "zang", "zao", "ze", "zei", "zen", "zeng", "zha", "zhai", "zhan", "zhang", "zhao", "zhe", "zhei", "zhen", "zheng", "zhi", "zhong", "zhou", "zhu", "zhua", "zhuai", "zhuan", "zhuang", "zhui", "zhun", "zhuo", "zi", "zong", "zou", "zu", "zuan", "zui", "zun", "zuo"];
    expwords=[""];
    thresh_array=[1.25];
    thresh = 1.5;   # env no thresh_array
#output_boundary=1;
use_penalty=0;
score_high_index=65;
score_low_index=85;
score_high_weight=1.0;
score_low_weight=0.7;
version=wakeup_aifar_comm_fsmn.bin;
#---------------------------------------------
    use_duration_penalty=0;

    score_false_reduce_duration=21000;
    score_false_reduce_nowakeup_duration=6000;
    score_false_reduce_coef=0.7;

    score_easy_wakeup_nowakeup_coef=1.5;
    score_easy_wakeup_coef=1.5;
    score_easy_wakeup_duration=500;
#----------------------------------------------
};

