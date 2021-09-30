#include "wtk_parm_cfg.h"
#include "AISP_TSL_str.h"

extern unsigned AISPEECH_ARRAY_CFG[];
extern const int parm_window_size;
extern const int parm_window_step;
extern const char *parm_target_kind;
extern const char parm_use_hamming;
extern const char parm_use_hanning;
extern const char parm_fbank_chans;

#ifdef WAKEUP_FBANK24
static const aisp_s8_t loChan_fixed24[] =
{
    -1,  0,  0,  1,  1,  1,  2,  2,  3,  3,  3,  3,  4,  4,  4,  5,
    5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  7,  8,  8,  8,  8,
    8,  8,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 11, 11,
    11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,
    19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

/* values are in Q15 */
static const aisp_s16_t loWt_fixed24[] =
{
    0, 18570,  4966, 24677, 12124,    38, 21153,  9904,
    31799, 21277, 11086,  1204, 24382, 15067,  6011, 29969,
    21391, 13034,  4886, 29706, 21947, 14369,  6963, 32491,
    25409, 18477, 11690,  5042, 31295, 24908, 18644, 12499,
    6468,    546, 27499, 21786, 16171, 10652,  5224, 32654,
    27403, 22234, 17147, 12138,  7205,  2345, 30326, 25608,
    20957, 16372, 11850,  7391,  2992, 31420, 27137, 22909,
    18736, 14616, 10547,  6529,  2560, 31406, 27532, 23703,
    19918, 16177, 12479,  8822,  5206,  1630, 30861, 27361,
    23899, 20474, 17084, 13729, 10409,  7122,  3868,   646,
    30224, 27064, 23936, 20837, 17767, 14726, 11713,  8728,
    5770,  2839,  32702, 29822, 26968, 24139, 21334, 18553,
    15796, 13062, 10351,  7662,  4995,  2350, 32494, 29891,
    27309, 24748, 22206, 19684, 17181, 14698, 12233,  9787,
    7360,  4950,   2558,   183, 30594, 28253, 25930, 23622,
    21332, 19057, 16798, 14554, 12326, 10113,  7915,  5732,
    3563,  1409,  32037, 29911, 27798, 25700, 23615, 21543,
    19484, 17438, 15405, 13385, 11377,  9381,  7398,  5426,
    3467,  1519,  32351, 30426, 28513, 26611, 24720, 22840,
    20970, 19111, 17263, 15426, 13598, 11781,  9974,  8177,
    6390,  4613,   2845,  1087, 32106, 30367, 28637, 26916,
    25204, 23501, 21807, 20122, 18445, 16777, 15118, 13467,
    11824, 10190,  8564,  6946,  5336,  3733,  2139,   553,
    31742, 30171, 28607, 27051, 25502, 23961, 22427, 20900,
    19380, 17867, 16362, 14863, 13371, 11886, 10407,  8936,
    7471,  6012,   4561,  3115,  1676,   243, 31585, 30165,
    28751, 27343, 25941, 24545, 23155, 21772, 20394, 19021,
    17655, 16294, 14939, 13590, 12246, 10907,  9575,  8247,
    6925,  5608,   4297,  2991,  1690,   395, 31872, 30587,
    29306, 28031, 26761, 25495, 24235, 22979, 21728, 20482,
    19241, 18005, 16773, 15546, 14323, 13105, 11892, 10683,
    9478,  8278,  7083,  5891,  4705,  3522,  2344,  1170
};
#endif

#ifdef WAKEUP_FBANK40
static const aisp_s8_t loChan_fixed40[] =
{
    -1,  0,  1,  2,  2,  3,  3,  4,  4,  5, 6,  6,   6,  7,  7,  8,
    8,  9,  9,  9, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14,
    14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18,
    18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 21, 21, 21, 21,
    21, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24,
    24, 24, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26, 26,
    27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34,
    34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35,
    35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
    37, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40
};

static const aisp_s16_t loWt_fixed40[] =
{
    0,  9484, 19942, 31296, 10709, 23656,  4545, 18864,
    1032, 16545, 32599, 16393,   665, 18156,  3305, 21624,
    7556, 26619, 13256,   221, 20264,  7836, 28460, 16585,
    4970, 26370, 15240,  4337, 26420, 15946,  5673, 28363,
    18472,  8760, 31991, 22621, 13413,  4361, 28229, 19474,
    10861,  2385, 26810, 18595, 10505,  2536, 27452, 19714,
    12087,  4567, 29920, 22607, 15393,  8275,  1251, 27086,
    20242, 13485,  6812,   222, 26480, 20049, 13695,  7415,
    1209, 27842, 21777, 15779,  9849,  3984, 30951, 25212,
    19534, 13916,  8357,  2855, 30177, 24787, 19450, 14166,
    8934,  3753, 31390, 26308, 21274, 16286, 11345,  6450,
    1599, 29559, 24795, 20073, 15392, 10752,  6152,  1591,
    29838, 25354, 20907, 16497, 12124,  7786,  3483, 31982,
    27748, 23547, 19378, 15242, 11138,  7065,  3023, 31780,
    27798, 23846, 19923, 16029, 12163,  8325,  4514,   730,
    29741, 26010, 22305, 18626, 14972, 11343,  7738,  4157,
    601, 29836, 26326, 22839, 19375, 15933, 12514,  9116,
    5739,  2384, 31818, 28504, 25212, 21939, 18686, 15453,
    12239,  9045,  5870,  2713, 32343, 29224, 26122, 23039,
    19973, 16925, 13894, 10880,  7884,  4903,  1940, 31761,
    28830, 25915, 23016, 20133, 17265, 14412, 11575,  8753,
    5945,  3153,   374, 30379, 27629, 24894, 22172, 19465,
    16771, 14090, 11423,  8770,  6129,  3501,   887, 31053,
    28464, 25887, 23322, 20770, 18230, 15703, 13187, 10682,
    8190,  5709,  3240,   782, 31103, 28668, 26243, 23830,
    21427, 19035, 16654, 14284, 11924,  9574,  7235,  4906,
    2587,   278, 30747, 28458, 26178, 23909, 21649, 19398,
    17157, 14926, 12703, 10490,  8286,  6092,  3906,  1729,
    32329, 30169, 28019, 25877, 23743, 21619, 19502, 17394,
    15294, 13203, 11119,  9044,  6977,  4918,  2867,   823,
    31556, 29528, 27507, 25495, 23490, 21492, 19502, 17520,
    15544, 13576, 11616,  9662,  7716,  5776,  3844,  1918
};
#endif

#ifdef WAKEUP_HAMMING400
static const aisp_s16_t hamwin_fixed400[] =
{
    2621,  2623,  2629,  2638,  2651,  2668,  2689,  2713,  2741,  2773,  2808,  2847,  2890,  2936,  2986,  3040,
    3097,  3158,  3223,  3291,  3363,  3438,  3517,  3599,  3685,  3775,  3867,  3963,  4063,  4166,  4272,  4382,
    4495,  4611,  4731,  4853,  4979,  5108,  5241,  5376,  5514,  5655,  5800,  5947,  6097,  6250,  6406,  6565,
    6726,  6890,  7057,  7227,  7399,  7573,  7751,  7930,  8112,  8297,  8483,  8672,  8864,  9057,  9253,  9450,
    9650,  9852, 10056, 10261, 10468, 10678, 10889, 11101, 11316, 11531, 11749, 11968, 12188, 12409, 12632, 12857,
    13082, 13309, 13536, 13765, 13994, 14225, 14456, 14689, 14922, 15155, 15390, 15624, 15860, 16096, 16332, 16568,
    16805, 17042, 17279, 17517, 17754, 17991, 18229, 18466, 18703, 18939, 19176, 19412, 19647, 19883, 20117, 20351,
    20584, 20817, 21049, 21280, 21510, 21739, 21967, 22194, 22420, 22645, 22869, 23091, 23312, 23531, 23750, 23966,
    24181, 24395, 24606, 24817, 25025, 25231, 25436, 25639, 25839, 26038, 26235, 26429, 26622, 26812, 27000, 27185,
    27368, 27549, 27728, 27904, 28077, 28248, 28416, 28581, 28744, 28904, 29062, 29216, 29368, 29516, 29662, 29805,
    29945, 30082, 30215, 30346, 30473, 30598, 30719, 30837, 30951, 31063, 31171, 31275, 31377, 31474, 31569, 31660,
    31748, 31832, 31912, 31989, 32063, 32133, 32199, 32262, 32321, 32377, 32429, 32477, 32522, 32562, 32600, 32633,
    32663, 32689, 32712, 32730, 32745, 32756, 32764, 32767, 32767, 32764, 32756, 32745, 32730, 32712, 32689, 32663,
    32633, 32600, 32562, 32522, 32477, 32429, 32377, 32321, 32262, 32199, 32133, 32063, 31989, 31912, 31832, 31748,
    31660, 31569, 31474, 31377, 31275, 31171, 31063, 30951, 30837, 30719, 30598, 30473, 30346, 30215, 30082, 29945,
    29805, 29662, 29516, 29368, 29216, 29062, 28904, 28744, 28581, 28416, 28248, 28077, 27904, 27728, 27549, 27368,
    27185, 27000, 26812, 26622, 26429, 26235, 26038, 25839, 25639, 25436, 25231, 25025, 24817, 24606, 24395, 24181,
    23966, 23750, 23531, 23312, 23091, 22869, 22645, 22420, 22194, 21967, 21739, 21510, 21280, 21049, 20817, 20584,
    20351, 20117, 19883, 19647, 19412, 19176, 18939, 18703, 18466, 18229, 17991, 17754, 17517, 17279, 17042, 16805,
    16568, 16332, 16096, 15860, 15624, 15390, 15155, 14922, 14689, 14456, 14225, 13994, 13765, 13536, 13309, 13082,
    12857, 12632, 12409, 12188, 11968, 11749, 11531, 11316, 11101, 10889, 10678, 10469, 10261, 10056,  9852,  9650,
    9450,  9253,  9057,  8864,  8672,  8483,  8297,  8112,  7930,  7751,  7574,  7399,  7227,  7057,  6890,  6726,
    6565,  6406,  6250,  6097,  5947,  5800,  5655,  5514,  5376,  5241,  5108,  4979,  4854,  4731,  4611,  4495,
    4382,  4272,  4166,  4063,  3963,  3867,  3775,  3685,  3599,  3517,  3438,  3363,  3291,  3223,  3158,  3097,
    3040,  2986,  2936,  2890,  2847,  2808,  2773,  2741,  2713,  2689,  2668,  2651,  2638,  2629,  2623,  2621
};
#endif

#ifdef WAKEUP_HAMMING480
static const aisp_s16_t hamwin_fixed480[] =
{
    2621,  2623,  2627,  2633,  2642,  2654,  2668,  2685,  2704,  2726,  2751,  2778,  2808,  2840,  2875,  2912,
    2952,  2995,  3040,  3087,  3137,  3190,  3245,  3302,  3362,  3425,  3489,  3557,  3627,  3699,  3773,  3850,
    3930,  4012,  4096,  4182,  4271,  4362,  4455,  4551,  4649,  4749,  4852,  4956,  5063,  5172,  5283,  5396,
    5512,  5629,  5749,  5870,  5994,  6120,  6247,  6377,  6508,  6642,  6777,  6914,  7054,  7195,  7337,  7482,
    7628,  7776,  7926,  8077,  8230,  8385,  8541,  8699,  8859,  9020,  9182,  9346,  9511,  9678,  9846, 10016,
    10186, 10358, 10532, 10706, 10882, 11059, 11237, 11416, 11597, 11778, 11960, 12144, 12328, 12513, 12699, 12886,
    13074, 13263, 13452, 13642, 13833, 14024, 14216, 14409, 14602, 14796, 14990, 15185, 15380, 15576, 15772, 15968,
    16165, 16361, 16558, 16756, 16953, 17151, 17348, 17546, 17744, 17941, 18139, 18337, 18534, 18731, 18929, 19125,
    19322, 19519, 19715, 19910, 20106, 20301, 20495, 20689, 20883, 21076, 21268, 21460, 21651, 21841, 22031, 22220,
    22408, 22596, 22782, 22968, 23153, 23336, 23519, 23701, 23882, 24062, 24240, 24418, 24594, 24769, 24943, 25116,
    25288, 25458, 25627, 25794, 25960, 26125, 26288, 26449, 26610, 26768, 26925, 27081, 27235, 27387, 27538, 27686,
    27834, 27979, 28123, 28265, 28405, 28543, 28679, 28814, 28946, 29077, 29205, 29332, 29457, 29579, 29700, 29818,
    29935, 30049, 30161, 30271, 30379, 30485, 30588, 30690, 30789, 30885, 30980, 31072, 31162, 31250, 31335, 31418,
    31499, 31577, 31653, 31726, 31797, 31865, 31932, 31995, 32057, 32115, 32172, 32225, 32277, 32325, 32372, 32415,
    32457, 32495, 32531, 32565, 32596, 32624, 32650, 32673, 32694, 32712, 32728, 32741, 32751, 32759, 32764, 32767,
    32767, 32764, 32759, 32751, 32741, 32728, 32712, 32694, 32673, 32650, 32624, 32596, 32565, 32531, 32495, 32457,
    32415, 32372, 32325, 32277, 32225, 32172, 32115, 32057, 31995, 31932, 31865, 31797, 31726, 31653, 31577, 31499,
    31418, 31335, 31250, 31162, 31072, 30980, 30885, 30789, 30690, 30588, 30485, 30379, 30271, 30161, 30049, 29935,
    29818, 29700, 29579, 29457, 29332, 29205, 29077, 28946, 28814, 28679, 28543, 28405, 28265, 28123, 27979, 27834,
    27686, 27538, 27387, 27235, 27081, 26925, 26768, 26610, 26449, 26288, 26125, 25960, 25794, 25627, 25458, 25288,
    25116, 24943, 24769, 24594, 24418, 24240, 24062, 23882, 23701, 23519, 23336, 23153, 22968, 22782, 22596, 22408,
    22220, 22031, 21841, 21651, 21460, 21268, 21076, 20883, 20689, 20495, 20301, 20106, 19910, 19715, 19519, 19322,
    19125, 18929, 18731, 18534, 18337, 18139, 17941, 17744, 17546, 17348, 17151, 16953, 16756, 16558, 16361, 16165,
    15968, 15772, 15576, 15380, 15185, 14990, 14796, 14602, 14409, 14216, 14024, 13833, 13642, 13452, 13263, 13074,
    12886, 12699, 12513, 12328, 12144, 11960, 11778, 11597, 11416, 11237, 11059, 10882, 10706, 10532, 10358, 10186,
    10016,  9846,  9678,  9511,  9346,  9182,  9020,  8859,  8699,  8541,  8385,  8230,  8077,  7926,  7776,  7628,
    7482,  7337,  7195,  7054,  6914,  6777,  6642,  6508,  6377,  6247,  6120,  5994,  5870,  5749,  5629,  5512,
    5396,  5283,  5172,  5063,  4956,  4852,  4749,  4649,  4551,  4455,  4362,  4271,  4182,  4096,  4012,  3930,
    3850,  3773,  3699,  3627,  3557,  3489,  3425,  3362,  3302,  3245,  3190,  3137,  3087,  3040,  2995,  2952,
    2912,  2875,  2840,  2808,  2778,  2751,  2726,  2704,  2685,  2668,  2654,  2642,  2633,  2627,  2623,  2621
};
#endif

#ifdef WAKEUP_HANNING512
static const aisp_s16_t hanwin_fixed512[] =
{
    1,     5,    11,    20,    31,    44,    60,    79,    99,   123,   148,   177,   207,   240,   276,   314,
    354,   397,   442,   489,   539,   591,   646,   703,   762,   824,   888,   954,  1023,  1094,  1167,  1242,
    1320,  1400,  1482,  1567,  1654,  1743,  1834,  1927,  2023,  2120,  2220,  2322,  2426,  2532,  2640,  2751,
    2863,  2977,  3094,  3212,  3332,  3455,  3579,  3705,  3833,  3963,  4095,  4228,  4364,  4501,  4640,  4781,
    4924,  5068,  5214,  5361,  5511,  5662,  5814,  5968,  6124,  6281,  6440,  6600,  6762,  6925,  7090,  7255,
    7423,  7591,  7761,  7933,  8105,  8279,  8454,  8630,  8808,  8986,  9166,  9346,  9528,  9711,  9895, 10079,
    10265, 10452, 10639, 10828, 11017, 11207, 11397, 11589, 11781, 11974, 12168, 12362, 12557, 12752, 12948, 13145,
    13342, 13539, 13737, 13935, 14134, 14333, 14532, 14731, 14931, 15131, 15331, 15532, 15732, 15933, 16133, 16334,
    16535, 16735, 16936, 17136, 17337, 17537, 17737, 17937, 18137, 18336, 18535, 18734, 18932, 19130, 19328, 19525,
    19722, 19918, 20114, 20309, 20503, 20697, 20890, 21083, 21275, 21466, 21656, 21846, 22035, 22223, 22410, 22596,
    22781, 22965, 23149, 23331, 23512, 23692, 23871, 24049, 24226, 24402, 24576, 24749, 24921, 25092, 25261, 25429,
    25596, 25761, 25925, 26087, 26248, 26408, 26566, 26722, 26877, 27030, 27182, 27332, 27481, 27627, 27772, 27916,
    28058, 28198, 28336, 28472, 28607, 28739, 28870, 28999, 29126, 29251, 29375, 29496, 29615, 29733, 29848, 29961,
    30073, 30182, 30289, 30394, 30497, 30598, 30697, 30793, 30888, 30980, 31070, 31158, 31244, 31327, 31408, 31487,
    31564, 31638, 31710, 31780, 31847, 31913, 31975, 32036, 32094, 32150, 32203, 32254, 32303, 32349, 32393, 32435,
    32474, 32510, 32545, 32576, 32606, 32633, 32657, 32679, 32699, 32716, 32731, 32743, 32753, 32760, 32765, 32767,
    32767, 32765, 32760, 32753, 32743, 32731, 32716, 32699, 32679, 32657, 32633, 32606, 32576, 32545, 32510, 32474,
    32435, 32393, 32349, 32303, 32254, 32203, 32150, 32094, 32036, 31975, 31913, 31847, 31780, 31710, 31638, 31564,
    31487, 31408, 31327, 31244, 31158, 31070, 30980, 30888, 30793, 30697, 30598, 30497, 30394, 30289, 30182, 30073,
    29961, 29848, 29733, 29615, 29496, 29375, 29251, 29126, 28999, 28870, 28739, 28607, 28472, 28336, 28198, 28058,
    27916, 27772, 27627, 27481, 27332, 27182, 27030, 26877, 26722, 26566, 26408, 26248, 26087, 25925, 25761, 25596,
    25429, 25261, 25092, 24921, 24749, 24576, 24402, 24226, 24049, 23871, 23692, 23512, 23331, 23149, 22965, 22781,
    22596, 22410, 22223, 22035, 21846, 21656, 21466, 21275, 21083, 20890, 20697, 20503, 20309, 20114, 19918, 19722,
    19525, 19328, 19130, 18932, 18734, 18535, 18336, 18137, 17937, 17737, 17537, 17337, 17136, 16936, 16735, 16535,
    16334, 16133, 15933, 15732, 15532, 15331, 15131, 14931, 14731, 14532, 14333, 14134, 13935, 13737, 13539, 13342,
    13145, 12948, 12752, 12557, 12362, 12168, 11974, 11781, 11589, 11397, 11207, 11017, 10828, 10639, 10452, 10265,
    10079,  9895,  9711,  9528,  9346,  9166,  8986,  8808,  8630,  8454,  8279,  8105,  7933,  7761,  7591,  7423,
    7255,  7090,  6925,  6762,  6600,  6440,  6281,  6124,  5968,  5814,  5662,  5511,  5361,  5214,  5068,  4924,
    4781,  4640,  4501,  4364,  4228,  4095,  3963,  3833,  3705,  3579,  3455,  3332,  3212,  3094,  2977,  2863,
    2751,  2640,  2532,  2426,  2322,  2220,  2120,  2023,  1927,  1834,  1743,  1654,  1567,  1482,  1400,  1320,
    1242,  1167,  1094,  1023,   954,   888,   824,   762,   703,   646,   591,   539,   489,   442,   397,   354,
    314,   276,   240,   207,   177,   148,   123,    99,    79,    60,    44,    31,    20,    11,     5,     1,
};
#endif

static const aisp_s8_t *wtk_get_loChan(int fbank_chans)
{
    if (fbank_chans == 24)
    {
#ifdef WAKEUP_FBANK24
        return loChan_fixed24;
#endif
    }
    else
        if (fbank_chans == 40)
        {
#ifdef WAKEUP_FBANK40
            return loChan_fixed40;
#endif
        }

    return NULL;
}

static const aisp_s16_t *wtk_get_loWt(int fbank_chans)
{
    if (fbank_chans == 24)
    {
#ifdef WAKEUP_FBANK24
        return loWt_fixed24;
#endif
    }
    else
        if (fbank_chans == 40)
        {
#ifdef WAKEUP_FBANK40
            return loWt_fixed40;
#endif
        }

    return NULL;
}

static const aisp_s16_t *wtk_get_hwin(int use_hanming, int use_hanning, int window_size)
{
    if (use_hanming)
    {
        if (window_size == 250000)
        {
#ifdef WAKEUP_HAMMING400
            return hamwin_fixed400;
#endif
        }
        else
            if (window_size == 300000)
            {
#ifdef WAKEUP_HAMMING480
                return hamwin_fixed480;
#endif
            }
    }
    else
        if (use_hanning)
        {
            if (window_size == 320000)
            {
#ifdef WAKEUP_HANNING512
                return hanwin_fixed512;
#endif
            }
        }

    return NULL;
}

int wtk_parm_cfg_init(wtk_parm_cfg_t *cfg)
{
    AISP_TSL_memset(cfg, 0, sizeof(*cfg));
    wtk_zmean_cfg_init(&cfg->zmean);
    cfg->use_z = 0;
    cfg->use_cmn = AISPEECH_ARRAY_CFG[1];
    cfg->window_size = parm_window_size;
    cfg->window_step = parm_window_step;
    cfg->use_hamming = parm_use_hamming;
    cfg->use_hanning = parm_use_hanning;
    cfg->fbank_num_chans = parm_fbank_chans;
    cfg->hwin = wtk_get_hwin(parm_use_hamming, parm_use_hanning, parm_window_size);

    if (NULL == cfg->hwin && (cfg->use_hamming || cfg->use_hanning))
    {
        return -1;;
    }

    cfg->loChan = wtk_get_loChan(parm_fbank_chans);
    cfg->loWt = wtk_get_loWt(parm_fbank_chans);

    if (NULL == cfg->loChan || NULL == cfg->loWt)
    {
        return -1;
    }

    cfg->src_sample_rate = 625;
    cfg->del_win = cfg->acc_win = cfg->third_win = 2;
    cfg->numCepCoef = 12;
    cfg->cepLifter = 22;
    cfg->lpc_order = 12;
    // cfg->preEmph = 0.97f;
    // cfg->cepScale = 1.0f; // 10
    // cfg->loFBankFreq = -1.0;
    // cfg->hiFBankFreq = -1.0;
    // cfg->warpFreq = 1.0;
    // cfg->warpLowerCutOff = 0.0;
    // cfg->warpUpperCutOff = 0.0;
    // cfg->compressFact = 0.33f;
    cfg->zMeanSrc = 1;
    cfg->rawEnergy = 1;
    cfg->use_power = 1; // 1
    cfg->fbankFixed = 2;
    cfg->double_fft = 0;
//    cfg->SILFLOOR = 50;
//    cfg->addDither = 0.0;
    cfg->SIMPLEDIFFS = 0;
    cfg->cache_size = 1;
    cfg->feature_basic_cols = 12;
    cfg->align = 0;
    cfg->feature_cols = 0;
    cfg->static_feature_cols = 0;
//    cfg->ESCALE = 1.0f;
    return 0;
}

int wtk_parm_cfg_clean(wtk_parm_cfg_t *cfg)
{
    if (cfg->use_cmn)
    {
        wtk_zmean_cfg_clean(&cfg->zmean);
    }

#if defined(WAKEUP_NN_DNN)
    wtk_dnn_cfg_clean(&cfg->dnn);
#elif defined(WAKEUP_NN_FSMN)
    wtk_fsmn_cfg_clean(&cfg->fsmn);
#endif
    return 0;
}

static short wtk_win2sigma(int win)
{
    int i;
    int sigma;
    sigma = 0;

    for (i = 1; i <= win; ++i)
    {
        sigma += i * i;
    }

    sigma *= 2;
    return (short)sigma;
}

static void wtk_parm_cfg_update_inform(wtk_parm_cfg_t *cfg)
{
    cfg->feature_basic_cols = cfg->fbank_num_chans;
    cfg->static_feature_cols = cfg->feature_basic_cols + cfg->ZERO + cfg->ENERGY;
    cfg->feature_cols = (cfg->static_feature_cols) * (1 + cfg->DELTA + cfg->ACCS + cfg->THIRD);
}

int wtk_parm_cfg_update2(wtk_parm_cfg_t *cfg)
{
    int ret;
    cfg->frame_size = cfg->window_size / cfg->src_sample_rate;
    cfg->frame_step = cfg->window_step / cfg->src_sample_rate;
    cfg->ZERO = 0;
    cfg->DELTA = 0;
    cfg->ACCS = 0;
    cfg->THIRD = 0;
    cfg->ENERGY = 0;
    cfg->ZMEAN = 0;

    if (!AISP_TSL_strcmp(parm_target_kind, "FBANK_Z"))
    {
        cfg->ZMEAN = 1;
    }
    else
        if (!AISP_TSL_strcmp(parm_target_kind, "FBANK_D_A_Z"))
        {
            cfg->DELTA = 1;
            cfg->ACCS = 1;
            cfg->ZMEAN = 1;
        }
        else
        {
            ret = -1;
            goto end;
        }

    cfg->sigma[0] = wtk_win2sigma(cfg->del_win);
    cfg->sigma[1] = wtk_win2sigma(cfg->acc_win);
    cfg->sigma[2] = wtk_win2sigma(cfg->third_win);
#if defined(WAKEUP_NN_DNN)
    ret = wtk_dnn_cfg_update2(&cfg->dnn);
#elif defined(WAKEUP_NN_FSMN)
    ret = wtk_fsmn_cfg_update2(&cfg->fsmn);
#endif

    if (ret != 0)
    {
        goto end;
    }

    wtk_parm_cfg_update_inform(cfg);
end:
    return ret;
}

int wtk_parm_cfg_update_local(wtk_parm_cfg_t *cfg)
{
    int ret = 0;
#if defined(WAKEUP_NN_DNN)
    ret = wtk_dnn_cfg_init(&cfg->dnn);
#elif defined(WAKEUP_NN_FSMN)
    ret = wtk_fsmn_cfg_init(&cfg->fsmn);
#endif
    return ret;
}
