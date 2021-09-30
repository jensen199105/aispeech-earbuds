#include "gsc_core_tables.h"

/*
 * THe tables list in this file is including:
 *     1. g_aHanningWindow512
 *     2. g_aHanning_15
 */

/* Hanning_15 window, length is 31 */
const GSC_S32_T g_aHanning_15Window31[31] =
{
#ifdef GSC_FIXED
    /* Q31 */
    1289478,         5108358,        11309883,        19655731,
    29825177,        41427414,        54016574,        67108864,
    80201154,        92790314,       104392551,       114561997,
    122907845,       129109370,       132928250,       134217728,
    132928250,       129109370,       122907845,       114561997,
    104392551,        92790314,        80201154,        67108864,
    54016574,        41427414,        29825177,        19655731,
    11309883,         5108358,         1289478,
#else
    0.000600459987399, 0.002378764609022, 0.005266574615545, 0.009152913087920,
    0.013888430218137, 0.019291142738591, 0.025153427436996, 0.031250000000000,
    0.037346572563004, 0.043208857261409, 0.048611569781863, 0.053347086912080,
    0.057233425384455, 0.060121235390978, 0.061899540012601, 0.062500000000000,
    0.061899540012601, 0.060121235390978, 0.057233425384455, 0.053347086912080,
    0.048611569781863, 0.043208857261409, 0.037346572563004, 0.031250000000000,
    0.025153427436996, 0.019291142738591, 0.013888430218137, 0.009152913087920,
    0.005266574615545, 0.002378764609022, 0.000600459987399,
#endif
};

/* Hanning 512 window is W32Q31 */
const GSC_S32_T g_aHanningWindow512[GSC_FRMLEN] =
{
#ifdef GSC_FIXED
#ifndef LUDA_SUPPORT_FREQOUT_DIRECT //sqrt
    13151027,        26301560,        39451107,        52599175,        65745270,        78888899,        92029570,       105166789,
    118300065,       131428903,       144552813,       157671302,       170783877,       183890048,       196989322,       210081209,
    223165217,       236240856,       249307635,       262365064,       275412654,       288449915,       301476358,       314491496,
    327494838,       340485899,       353464191,       366429227,       379380521,       392317587,       405239940,       418147095,
    431038568,       443913877,       456772537,       469614067,       482437986,       495243811,       508031064,       520799264,
    533547932,       546276591,       558984763,       571671972,       584337741,       596981596,       609603063,       622201668,
    634776938,       647328402,       659855590,       672358032,       684835258,       697286801,       709712193,       722110970,
    734482665,       746826815,       759142957,       771430629,       783689371,       795918721,       808118223,       820287418,
    832425849,       844533063,       856608604,       868652020,       880662859,       892640671,       904585006,       916495417,
    928371456,       940212679,       952018642,       963788901,       975523015,       987220544,       998881050,      1010504095,
    1022089244,      1033636061,      1045144114,      1056612971,      1068042202,      1079431379,      1090780074,      1102087861,
    1113354317,      1124579020,      1135761547,      1146901480,      1157998402,      1169051895,      1180061545,      1191026940,
    1201947667,      1212823319,      1223653486,      1234437763,      1245175745,      1255867029,      1266511215,      1277107903,
    1287656696,      1298157198,      1308609016,      1319011757,      1329365031,      1339668451,      1349921629,      1360124182,
    1370275726,      1380375881,      1390424268,      1400420510,      1410364232,      1420255062,      1430092629,      1439876562,
    1449606497,      1459282067,      1468902910,      1478468665,      1487978973,      1497433478,      1506831825,      1516173662,
    1525458638,      1534686404,      1543856616,      1552968929,      1562023001,      1571018493,      1579955068,      1588832389,
    1597650125,      1606407945,      1615105520,      1623742524,      1632318633,      1640833525,      1649286882,      1657678385,
    1666007722,      1674274578,      1682478644,      1690619613,      1698697178,      1706711038,      1714660891,      1722546440,
    1730367389,      1738123443,      1745814314,      1753439711,      1760999350,      1768492946,      1775920218,      1783280889,
    1790574682,      1797801323,      1804960541,      1812052069,      1819075639,      1826030989,      1832917858,      1839735987,
    1846485121,      1853165007,      1859775393,      1866316033,      1872786681,      1879187094,      1885517033,      1891776259,
    1897964538,      1904081639,      1910127330,      1916101387,      1922003585,      1927833702,      1933591520,      1939276823,
    1944889398,      1950429033,      1955895523,      1961288660,      1966608244,      1971854075,      1977025955,      1982123692,
    1987147093,      1992095971,      1996970140,      2001769416,      2006493621,      2011142577,      2015716109,      2020214046,
    2024636219,      2028982463,      2033252615,      2037446513,      2041564002,      2045604927,      2049569135,      2053456479,
    2057266813,      2060999993,      2064655880,      2068234337,      2071735229,      2075158425,      2078503797,      2081771219,
    2084960570,      2088071728,      2091104577,      2094059005,      2096934899,      2099732153,      2102450660,      2105090320,
    2107651033,      2110132703,      2112535238,      2114858546,      2117102541,      2119267139,      2121352259,      2123357822,
    2125283753,      2127129980,      2128896433,      2130583047,      2132189759,      2133716507,      2135163235,      2136529888,
    2137816415,      2139022769,      2140148903,      2141194775,      2142160347,      2143045581,      2143850446,      2144574910,
    2145218946,      2145782531,      2146265643,      2146668264,      2146990378,      2147231975,      2147393045,      2147473581,
    2147473581,      2147393045,      2147231975,      2146990378,      2146668264,      2146265643,      2145782531,      2145218946,
    2144574910,      2143850446,      2143045581,      2142160347,      2141194775,      2140148903,      2139022769,      2137816415,
    2136529888,      2135163235,      2133716507,      2132189759,      2130583047,      2128896433,      2127129980,      2125283753,
    2123357822,      2121352259,      2119267139,      2117102541,      2114858546,      2112535238,      2110132703,      2107651033,
    2105090320,      2102450660,      2099732153,      2096934899,      2094059005,      2091104577,      2088071728,      2084960570,
    2081771219,      2078503797,      2075158425,      2071735229,      2068234337,      2064655880,      2060999993,      2057266813,
    2053456479,      2049569135,      2045604927,      2041564002,      2037446513,      2033252615,      2028982463,      2024636219,
    2020214046,      2015716109,      2011142577,      2006493621,      2001769416,      1996970140,      1992095971,      1987147093,
    1982123692,      1977025955,      1971854075,      1966608244,      1961288660,      1955895523,      1950429033,      1944889398,
    1939276823,      1933591520,      1927833702,      1922003585,      1916101387,      1910127330,      1904081639,      1897964538,
    1891776259,      1885517033,      1879187094,      1872786681,      1866316033,      1859775393,      1853165007,      1846485121,
    1839735987,      1832917858,      1826030989,      1819075639,      1812052069,      1804960541,      1797801323,      1790574682,
    1783280889,      1775920218,      1768492946,      1760999350,      1753439711,      1745814314,      1738123443,      1730367389,
    1722546440,      1714660891,      1706711038,      1698697178,      1690619613,      1682478644,      1674274578,      1666007722,
    1657678385,      1649286882,      1640833525,      1632318633,      1623742524,      1615105520,      1606407945,      1597650125,
    1588832389,      1579955068,      1571018493,      1562023001,      1552968929,      1543856616,      1534686404,      1525458638,
    1516173662,      1506831825,      1497433478,      1487978973,      1478468665,      1468902910,      1459282067,      1449606497,
    1439876562,      1430092629,      1420255062,      1410364232,      1400420510,      1390424268,      1380375881,      1370275726,
    1360124182,      1349921629,      1339668451,      1329365031,      1319011757,      1308609016,      1298157198,      1287656696,
    1277107903,      1266511215,      1255867029,      1245175745,      1234437763,      1223653486,      1212823319,      1201947667,
    1191026940,      1180061545,      1169051895,      1157998402,      1146901480,      1135761547,      1124579020,      1113354317,
    1102087861,      1090780074,      1079431379,      1068042202,      1056612971,      1045144114,      1033636061,      1022089244,
    1010504095,       998881050,       987220544,       975523015,       963788901,       952018642,       940212679,       928371456,
    916495417,       904585006,       892640671,       880662859,       868652020,       856608604,       844533063,       832425849,
    820287418,       808118223,       795918721,       783689371,       771430629,       759142957,       746826815,       734482665,
    722110970,       709712193,       697286801,       684835258,       672358032,       659855590,       647328402,       634776938,
    622201668,       609603063,       596981596,       584337741,       571671972,       558984763,       546276591,       533547932,
    520799264,       508031064,       495243811,       482437986,       469614067,       456772537,       443913877,       431038568,
    418147095,       405239940,       392317587,       379380521,       366429227,       353464191,       340485899,       327494838,
    314491496,       301476358,       288449915,       275412654,       262365064,       249307635,       236240856,       223165217,
    210081209,       196989322,       183890048,       170783877,       157671302,       144552813,       131428903,       118300065,
    105166789,        92029570,        78888899,        65745270,        52599175,        39451107,        26301560,        13151027,
#else
    80536,      322131,      724751,     1288333,     2012793,     2898024,     3943891,     5150239,
    6516886,     8043627,     9730233,    11576451,    13582005,    15746592,    18069890,    20551549,
    23191196,    25988436,    28942850,    32053993,    35321400,    38744581,    42323021,    46056183,
    49943509,    53984415,    58178294,    62524517,    67022433,    71671367,    76470621,    81419476,
    86517188,    91762994,    97156107,   102695717,   108380993,   114211083,   120185112,   126302183,
    132561380,   138961763,   145502373,   152182227,   159000324,   165955641,   173047136,   180273743,
    187634379,   195127940,   202753302,   210509320,   218394832,   226408654,   234549584,   242816402,
    251207866,   259722719,   268359682,   277117461,   285994741,   294990191,   304102461,   313330184,
    322671977,   332126438,   341692148,   351367673,   361151561,   371042344,   381038540,   391138647,
    401341152,   411644523,   422047216,   432547669,   443144307,   453835541,   464619767,   475495368,
    486460711,   497514152,   508654033,   519878683,   531186417,   542575541,   554044344,   565591107,
    577214098,   588911572,   600681776,   612522944,   624433299,   636411054,   648454414,   660561570,
    672730708,   684960001,   697247616,   709591708,   721990427,   734441911,   746944294,   759495700,
    772094245,   784738041,   797425190,   810153789,   822921929,   835727695,   848569165,   861444413,
    874351508,   887288513,   900253488,   913244488,   926259564,   939296764,   952354132,   965429710,
    978521535,   991627644,  1004746071,  1017874849,  1031012006,  1044155574,  1057303580,  1070454052,
    1083605017,  1096754502,  1109900536,  1123041145,  1136174359,  1149298207,  1162410721,  1175509934,
    1188593881,  1201660598,  1214708127,  1227734509,  1240737791,  1253716022,  1266667255,  1279589548,
    1292480961,  1305339561,  1318163419,  1330950612,  1343699220,  1356407333,  1369073043,  1381694450,
    1394269662,  1406796791,  1419273958,  1431699293,  1444070930,  1456387014,  1468645698,  1480845143,
    1492983517,  1505059002,  1517069785,  1529014064,  1540890048,  1552695955,  1564430014,  1576090465,
    1587675559,  1599183558,  1610612736,  1621961378,  1633227781,  1644410256,  1655507125,  1666516724,
    1677437401,  1688267517,  1699005449,  1709649584,  1720198328,  1730650097,  1741003323,  1751256453,
    1761407949,  1771456289,  1781399965,  1791237485,  1800967374,  1810588172,  1820098436,  1829496739,
    1838781671,  1847951840,  1857005869,  1865942402,  1874760097,  1883457631,  1892033699,  1900487016,
    1908816314,  1917020341,  1925097869,  1933047685,  1940868597,  1948559431,  1956119034,  1963546272,
    1970840031,  1977999216,  1985022753,  1991909589,  1998658692,  2005269048,  2011739665,  2018069574,
    2024257824,  2030303488,  2036205658,  2041963449,  2047575997,  2053042461,  2058362020,  2063533876,
    2068557254,  2073431399,  2078155582,  2082729093,  2087151245,  2091421377,  2095538846,  2099503036,
    2103313352,  2106969222,  2110470098,  2113815454,  2117004789,  2120037624,  2122913505,  2125632000,
    2128192701,  2130595224,  2132839209,  2134924319,  2136850241,  2138616686,  2140223390,  2141670111,
    2142956632,  2144082761,  2145048328,  2145853189,  2146497222,  2146980332,  2147302445,  2147463514,
    2147463514,  2147302445,  2146980332,  2146497222,  2145853189,  2145048328,  2144082761,  2142956632,
    2141670111,  2140223390,  2138616686,  2136850241,  2134924319,  2132839209,  2130595224,  2128192701,
    2125632000,  2122913505,  2120037624,  2117004789,  2113815454,  2110470098,  2106969222,  2103313352,
    2099503036,  2095538846,  2091421377,  2087151245,  2082729093,  2078155582,  2073431399,  2068557254,
    2063533876,  2058362020,  2053042461,  2047575997,  2041963449,  2036205658,  2030303488,  2024257824,
    2018069574,  2011739665,  2005269048,  1998658692,  1991909589,  1985022753,  1977999216,  1970840031,
    1963546272,  1956119034,  1948559431,  1940868597,  1933047685,  1925097869,  1917020341,  1908816314,
    1900487016,  1892033699,  1883457631,  1874760097,  1865942402,  1857005869,  1847951840,  1838781671,
    1829496739,  1820098436,  1810588172,  1800967374,  1791237485,  1781399965,  1771456289,  1761407949,
    1751256453,  1741003323,  1730650097,  1720198328,  1709649584,  1699005449,  1688267517,  1677437401,
    1666516724,  1655507125,  1644410256,  1633227781,  1621961378,  1610612736,  1599183558,  1587675559,
    1576090465,  1564430014,  1552695955,  1540890048,  1529014064,  1517069785,  1505059002,  1492983517,
    1480845143,  1468645698,  1456387014,  1444070930,  1431699293,  1419273958,  1406796791,  1394269662,
    1381694450,  1369073043,  1356407333,  1343699220,  1330950612,  1318163419,  1305339561,  1292480961,
    1279589548,  1266667255,  1253716022,  1240737791,  1227734509,  1214708127,  1201660598,  1188593881,
    1175509934,  1162410721,  1149298207,  1136174359,  1123041145,  1109900536,  1096754502,  1083605017,
    1070454052,  1057303580,  1044155574,  1031012006,  1017874849,  1004746071,   991627644,   978521535,
    965429710,   952354132,   939296764,   926259564,   913244488,   900253488,   887288513,   874351508,
    861444413,   848569165,   835727695,   822921929,   810153789,   797425190,   784738041,   772094245,
    759495700,   746944294,   734441911,   721990427,   709591708,   697247616,   684960001,   672730708,
    660561570,   648454414,   636411054,   624433299,   612522944,   600681776,   588911572,   577214098,
    565591107,   554044344,   542575541,   531186417,   519878683,   508654033,   497514152,   486460711,
    475495368,   464619767,   453835541,   443144307,   432547669,   422047216,   411644523,   401341152,
    391138647,   381038540,   371042344,   361151561,   351367673,   341692148,   332126438,   322671977,
    313330184,   304102461,   294990191,   285994741,   277117461,   268359682,   259722719,   251207866,
    242816402,   234549584,   226408654,   218394832,   210509320,   202753302,   195127940,   187634379,
    180273743,   173047136,   165955641,   159000324,   152182227,   145502373,   138961763,   132561380,
    126302183,   120185112,   114211083,   108380993,   102695717,    97156107,    91762994,    86517188,
    81419476,    76470621,    71671367,    67022433,    62524517,    58178294,    53984415,    49943509,
    46056183,    42323021,    38744581,    35321400,    32053993,    28942850,    25988436,    23191196,
    20551549,    18069890,    15746592,    13582005,    11576451,     9730233,     8043627,     6516886,
    5150239,     3943891,     2898024,     2012793,     1288333,      724751,      322131,       80536,
#endif/* #ifndef LUDA_SUPPORT_FREQOUT_DIRECT //sqrt */

#else
    0.006123924010034, 0.012247618355787, 0.018370853381597, 0.024493399449025, 0.030615026945467, 0.036735506292773, 0.042854607955846, 0.048972102451262,
    0.055087760355865, 0.061201352315381, 0.067312649053011, 0.073421421378035, 0.079527440194406, 0.085630476509339, 0.091730301441904, 0.097826686231604,
    0.103919402246959, 0.110008220994079, 0.116092914125230, 0.122173253447402, 0.128249010930863, 0.134319958717716, 0.140385869130437, 0.146446514680422,
    0.152501668076510, 0.158551102233514, 0.164594590280734, 0.170631905570464, 0.176662821686496, 0.182687112452607, 0.188704551941044, 0.194714914480997,
    0.200717974667059, 0.206713507367684, 0.212701287733626, 0.218681091206376, 0.224652693526577, 0.230615870742440, 0.236570399218142, 0.242516055642211,
    0.248452617035901, 0.254379860761556, 0.260297564530959, 0.266205506413667, 0.272103464845335, 0.277991218636026, 0.283868546978507, 0.289735229456525,
    0.295591046053079, 0.301435777158668, 0.307269203579530, 0.313091106545858, 0.318901267720007, 0.324699469204684, 0.330485493551114, 0.336259123767203,
    0.342020143325669, 0.347768336172165, 0.353503486733384, 0.359225379925137, 0.364933801160428, 0.370628536357493, 0.376309371947835, 0.381976094884232,
    0.387628492648723, 0.393266353260583, 0.398889465284271, 0.404497617837359, 0.410090600598440, 0.415668203815018, 0.421230218311373, 0.426776435496404,
    0.432306647371454, 0.437820646538112, 0.443318226205986, 0.448799180200462, 0.454263302970436, 0.459710389596022, 0.465140235796237, 0.470552637936660,
    0.475947393037074, 0.481324298779073, 0.486683153513655, 0.492023756268778, 0.497345906756902, 0.502649405382498, 0.507934053249534, 0.513199652168935,
    0.518446004666014, 0.523672913987878, 0.528880184110809, 0.534067619747614, 0.539235026354946, 0.544382210140606, 0.549508978070806, 0.554615137877409,
    0.559700498065141, 0.564764867918771, 0.569808057510266, 0.574829877705911, 0.579830140173403, 0.584808657388914, 0.589765242644124, 0.594699710053223,
    0.599611874559882, 0.604501551944193, 0.609368558829579, 0.614212712689668, 0.619033831855142, 0.623831735520549, 0.628606243751082, 0.633357177489328,
    0.638084358561984, 0.642787609686539, 0.647466754477921, 0.652121617455113, 0.656752024047734, 0.661357800602587, 0.665938774390168, 0.670494773611149,
    0.675025627402815, 0.679531165845478, 0.684011219968843, 0.688465621758352, 0.692894204161478, 0.697296801093995, 0.701673247446206, 0.706023379089129,
    0.710347032880664, 0.714644046671699, 0.718914259312199, 0.723157510657247, 0.727373641573049, 0.731562493942904, 0.735723910673132, 0.739857735698967,
    0.743963813990408, 0.748041991558035, 0.752092115458784, 0.756114033801680, 0.760107595753536, 0.764072651544610, 0.768009052474220, 0.771916650916321,
    0.775795300325042, 0.779644855240182, 0.783465171292666, 0.787256105209956, 0.791017514821427, 0.794749259063700, 0.798451197985926, 0.802123192755044,
    0.805765105660978, 0.809376800121809, 0.812958140688894, 0.816508993051943, 0.820029224044062, 0.823518701646744, 0.826977294994818, 0.830404874381361,
    0.833801311262559, 0.837166478262529, 0.840500249178097, 0.843802498983529, 0.847073103835222, 0.850311941076346, 0.853518889241446, 0.856693828060996,
    0.859836638465911, 0.862947202592010, 0.866025403784439, 0.869071126602043, 0.872084256821698, 0.875064681442594, 0.878012288690471, 0.880926968021813,
    0.883808610127994, 0.886657106939375, 0.889472351629358, 0.892254238618394, 0.895002663577940, 0.897717523434372, 0.900398716372854, 0.903046141841150,
    0.905659700553401, 0.908239294493846, 0.910784826920497, 0.913296202368769, 0.915773326655058, 0.918216106880274, 0.920624451433328, 0.922998269994563,
    0.925337473539143, 0.927641974340393, 0.929911685973088, 0.932146523316692, 0.934346402558556, 0.936511241197055, 0.938640958044684, 0.940735473231106,
    0.942794708206143, 0.944818585742724, 0.946807029939781, 0.948759966225094, 0.950677321358092, 0.952559023432593, 0.954405001879507, 0.956215187469480,
    0.957989512315489, 0.959727909875389, 0.961430314954410, 0.963096663707600, 0.964726893642220, 0.966320943620089, 0.967878753859872, 0.969400265939330,
    0.970885422797504, 0.972334168736858, 0.973746449425368, 0.975122211898556, 0.976461404561484, 0.977763977190679, 0.979029880936027, 0.980259068322594,
    0.981451493252418, 0.982607111006227, 0.983725878245124, 0.984807753012208, 0.985852694734150, 0.986860664222712, 0.987831623676219, 0.988765536680977,
    0.989662368212635, 0.990522084637503, 0.991344653713810, 0.992130044592916, 0.992878227820465, 0.993589175337494, 0.994262860481482, 0.994899257987354,
    0.995498343988421, 0.996060096017284, 0.996584493006670, 0.997071515290225, 0.997521144603253, 0.997933364083395, 0.998308158271268, 0.998645513111043,
    0.998945415950969, 0.999207855543850, 0.999432822047469, 0.999620307024951, 0.999770303445087, 0.999882805682590, 0.999957809518312, 0.999995312139400,
    0.999995312139400, 0.999957809518312, 0.999882805682590, 0.999770303445087, 0.999620307024951, 0.999432822047469, 0.999207855543850, 0.998945415950969,
    0.998645513111043, 0.998308158271268, 0.997933364083395, 0.997521144603253, 0.997071515290225, 0.996584493006670, 0.996060096017284, 0.995498343988421,
    0.994899257987354, 0.994262860481482, 0.993589175337494, 0.992878227820465, 0.992130044592916, 0.991344653713810, 0.990522084637503, 0.989662368212635,
    0.988765536680977, 0.987831623676219, 0.986860664222712, 0.985852694734150, 0.984807753012208, 0.983725878245124, 0.982607111006227, 0.981451493252418,
    0.980259068322594, 0.979029880936027, 0.977763977190679, 0.976461404561484, 0.975122211898556, 0.973746449425368, 0.972334168736858, 0.970885422797504,
    0.969400265939330, 0.967878753859872, 0.966320943620089, 0.964726893642220, 0.963096663707600, 0.961430314954410, 0.959727909875389, 0.957989512315489,
    0.956215187469480, 0.954405001879507, 0.952559023432593, 0.950677321358092, 0.948759966225094, 0.946807029939781, 0.944818585742724, 0.942794708206143,
    0.940735473231106, 0.938640958044684, 0.936511241197055, 0.934346402558556, 0.932146523316692, 0.929911685973088, 0.927641974340393, 0.925337473539143,
    0.922998269994563, 0.920624451433328, 0.918216106880274, 0.915773326655058, 0.913296202368769, 0.910784826920497, 0.908239294493846, 0.905659700553401,
    0.903046141841150, 0.900398716372854, 0.897717523434372, 0.895002663577940, 0.892254238618394, 0.889472351629358, 0.886657106939375, 0.883808610127994,
    0.880926968021813, 0.878012288690471, 0.875064681442594, 0.872084256821698, 0.869071126602043, 0.866025403784439, 0.862947202592010, 0.859836638465911,
    0.856693828060996, 0.853518889241446, 0.850311941076346, 0.847073103835222, 0.843802498983529, 0.840500249178097, 0.837166478262529, 0.833801311262559,
    0.830404874381361, 0.826977294994818, 0.823518701646744, 0.820029224044062, 0.816508993051943, 0.812958140688894, 0.809376800121809, 0.805765105660978,
    0.802123192755044, 0.798451197985926, 0.794749259063700, 0.791017514821427, 0.787256105209956, 0.783465171292666, 0.779644855240182, 0.775795300325042,
    0.771916650916321, 0.768009052474220, 0.764072651544610, 0.760107595753536, 0.756114033801680, 0.752092115458784, 0.748041991558035, 0.743963813990408,
    0.739857735698967, 0.735723910673132, 0.731562493942904, 0.727373641573049, 0.723157510657247, 0.718914259312199, 0.714644046671699, 0.710347032880664,
    0.706023379089129, 0.701673247446206, 0.697296801093995, 0.692894204161478, 0.688465621758352, 0.684011219968843, 0.679531165845478, 0.675025627402815,
    0.670494773611149, 0.665938774390168, 0.661357800602587, 0.656752024047734, 0.652121617455113, 0.647466754477921, 0.642787609686539, 0.638084358561984,
    0.633357177489328, 0.628606243751082, 0.623831735520549, 0.619033831855142, 0.614212712689668, 0.609368558829579, 0.604501551944193, 0.599611874559882,
    0.594699710053223, 0.589765242644124, 0.584808657388914, 0.579830140173403, 0.574829877705911, 0.569808057510266, 0.564764867918771, 0.559700498065141,
    0.554615137877409, 0.549508978070806, 0.544382210140606, 0.539235026354946, 0.534067619747614, 0.528880184110809, 0.523672913987878, 0.518446004666014,
    0.513199652168935, 0.507934053249534, 0.502649405382498, 0.497345906756902, 0.492023756268778, 0.486683153513655, 0.481324298779073, 0.475947393037074,
    0.470552637936660, 0.465140235796237, 0.459710389596022, 0.454263302970436, 0.448799180200462, 0.443318226205986, 0.437820646538112, 0.432306647371454,
    0.426776435496404, 0.421230218311373, 0.415668203815018, 0.410090600598440, 0.404497617837359, 0.398889465284271, 0.393266353260583, 0.387628492648723,
    0.381976094884232, 0.376309371947835, 0.370628536357493, 0.364933801160428, 0.359225379925137, 0.353503486733384, 0.347768336172165, 0.342020143325669,
    0.336259123767203, 0.330485493551114, 0.324699469204684, 0.318901267720007, 0.313091106545858, 0.307269203579530, 0.301435777158668, 0.295591046053079,
    0.289735229456525, 0.283868546978507, 0.277991218636026, 0.272103464845335, 0.266205506413667, 0.260297564530959, 0.254379860761556, 0.248452617035901,
    0.242516055642211, 0.236570399218142, 0.230615870742440, 0.224652693526577, 0.218681091206376, 0.212701287733626, 0.206713507367684, 0.200717974667059,
    0.194714914480997, 0.188704551941044, 0.182687112452607, 0.176662821686496, 0.170631905570464, 0.164594590280734, 0.158551102233514, 0.152501668076510,
    0.146446514680422, 0.140385869130437, 0.134319958717716, 0.128249010930863, 0.122173253447402, 0.116092914125230, 0.110008220994079, 0.103919402246959,
    0.097826686231604, 0.091730301441904, 0.085630476509339, 0.079527440194406, 0.073421421378035, 0.067312649053011, 0.061201352315381, 0.055087760355865,
    0.048972102451262, 0.042854607955846, 0.036735506292773, 0.030615026945467, 0.024493399449025, 0.018370853381597, 0.012247618355787, 0.006123924010034,
#endif/* #ifdef GSC_FIXED */
};
