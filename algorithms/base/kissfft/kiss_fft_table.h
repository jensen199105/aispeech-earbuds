/************************************************************
  Copyright (C), AISpeechTech. Co., Ltd.

  FileName   : kiss_fft_table.h
  Project    :
  Module     :
  Version    :
  Date       : 2019/07/25
  Author     : Youhai.Jiang
  Document   :
  Description:

  Function List :

  History :
  <author>      <time>            <version >        <desc>
  Youhai.Jiang  2019/07/25         1.00              Create

************************************************************/
#ifndef __KISS_FFT_TABLE_H__
#define __KISS_FFT_TABLE_H__


const int fft_table_256[] =
{
    2147483647, 0,
    2144896909, -105372028,
    2137142926, -210490206,
    2124240379, -315101294,
    2106220351, -418953276,
    2083126253, -521795963,
    2055013722, -623381597,
    2021950483, -723465451,
    1984016188, -821806413,
    1941302224, -918167571,
    1893911493, -1012316784,
    1841958164, -1104027236,
    1785567395, -1193077990,
    1724875039, -1279254515,
    1660027308, -1362349204,
    1591180425, -1442161874,
    1518500249, -1518500249,
    1442161874, -1591180425,
    1362349204, -1660027308,
    1279254515, -1724875039,
    1193077990, -1785567395,
    1104027236, -1841958164,
    1012316784, -1893911493,
    918167571,  -1941302224,
    821806413,  -1984016188,
    723465451,  -2021950483,
    623381597,  -2055013722,
    521795963,  -2083126253,
    418953276,  -2106220351,
    315101294,  -2124240379,
    210490206,  -2137142926,
    105372028,  -2144896909,
    0,  -2147483647,
    -105372028, -2144896909,
    -210490206, -2137142926,
    -315101294, -2124240379,
    -418953276, -2106220351,
    -521795963, -2083126253,
    -623381597, -2055013722,
    -723465451, -2021950483,
    -821806413, -1984016188,
    -918167571, -1941302224,
    -1012316784,    -1893911493,
    -1104027236,    -1841958164,
    -1193077990,    -1785567395,
    -1279254515,    -1724875039,
    -1362349204,    -1660027308,
    -1442161874,    -1591180425,
    -1518500249,    -1518500249,
    -1591180425,    -1442161874,
    -1660027308,    -1362349204,
    -1724875039,    -1279254515,
    -1785567395,    -1193077990,
    -1841958164,    -1104027236,
    -1893911493,    -1012316784,
    -1941302224,    -918167571,
    -1984016188,    -821806413,
    -2021950483,    -723465451,
    -2055013722,    -623381597,
    -2083126253,    -521795963,
    -2106220351,    -418953276,
    -2124240379,    -315101294,
    -2137142926,    -210490206,
    -2144896909,    -105372028,
    -2147483647,    0,
    -2144896909,    105372028,
    -2137142926,    210490206,
    -2124240379,    315101294,
    -2106220351,    418953276,
    -2083126253,    521795963,
    -2055013722,    623381597,
    -2021950483,    723465451,
    -1984016188,    821806413,
    -1941302224,    918167571,
    -1893911493,    1012316784,
    -1841958164,    1104027236,
    -1785567395,    1193077990,
    -1724875039,    1279254515,
    -1660027308,    1362349204,
    -1591180425,    1442161874,
    -1518500249,    1518500249,
    -1442161874,    1591180425,
    -1362349204,    1660027308,
    -1279254515,    1724875039,
    -1193077990,    1785567395,
    -1104027236,    1841958164,
    -1012316784,    1893911493,
    -918167571, 1941302224,
    -821806413, 1984016188,
    -723465451, 2021950483,
    -623381597, 2055013722,
    -521795963, 2083126253,
    -418953276, 2106220351,
    -315101294, 2124240379,
    -210490206, 2137142926,
    -105372028, 2144896909,
    0,  2147483647,
    105372028,  2144896909,
    210490206,  2137142926,
    315101294,  2124240379,
    418953276,  2106220351,
    521795963,  2083126253,
    623381597,  2055013722,
    723465451,  2021950483,
    821806413,  1984016188,
    918167571,  1941302224,
    1012316784, 1893911493,
    1104027236, 1841958164,
    1193077990, 1785567395,
    1279254515, 1724875039,
    1362349204, 1660027308,
    1442161874, 1591180425,
    1518500249, 1518500249,
    1591180425, 1442161874,
    1660027308, 1362349204,
    1724875039, 1279254515,
    1785567395, 1193077990,
    1841958164, 1104027236,
    1893911493, 1012316784,
    1941302224, 918167571,
    1984016188, 821806413,
    2021950483, 723465451,
    2055013722, 623381597,
    2083126253, 521795963,
    2106220351, 418953276,
    2124240379, 315101294,
    2137142926, 210490206,
    2144896909, 105372028,
};

const int fft_supertable_256[] =
{
    -52701887,  -2146836865,
    -105372028, -2144896909,
    -157978697, -2141664947,
    -210490206, -2137142926,
    -262874923, -2131333571,
    -315101294, -2124240379,
    -367137860, -2115867625,
    -418953276, -2106220351,
    -470516330, -2095304369,
    -521795963, -2083126253,
    -572761285, -2069693341,
    -623381597, -2055013722,
    -673626408, -2039096240,
    -723465451, -2021950483,
    -772868706, -2003586778,
    -821806413, -1984016188,
    -870249095, -1963250500,
    -918167571, -1941302224,
    -965532978, -1918184580,
    -1012316784,    -1893911493,
    -1058490807,    -1868497585,
    -1104027236,    -1841958164,
    -1148898640,    -1814309215,
    -1193077990,    -1785567395,
    -1236538675,    -1755750016,
    -1279254515,    -1724875039,
    -1321199780,    -1692961061,
    -1362349204,    -1660027308,
    -1402677999,    -1626093615,
    -1442161874,    -1591180425,
    -1480777044,    -1555308767,
    -1518500249,    -1518500249,
    -1555308767,    -1480777044,
    -1591180425,    -1442161874,
    -1626093615,    -1402677999,
    -1660027308,    -1362349204,
    -1692961061,    -1321199780,
    -1724875039,    -1279254515,
    -1755750016,    -1236538675,
    -1785567395,    -1193077990,
    -1814309215,    -1148898640,
    -1841958164,    -1104027236,
    -1868497585,    -1058490807,
    -1893911493,    -1012316784,
    -1918184580,    -965532978,
    -1941302224,    -918167571,
    -1963250500,    -870249095,
    -1984016188,    -821806413,
    -2003586778,    -772868706,
    -2021950483,    -723465451,
    -2039096240,    -673626408,
    -2055013722,    -623381597,
    -2069693341,    -572761285,
    -2083126253,    -521795963,
    -2095304369,    -470516330,
    -2106220351,    -418953276,
    -2115867625,    -367137860,
    -2124240379,    -315101294,
    -2131333571,    -262874923,
    -2137142926,    -210490206,
    -2141664947,    -157978697,
    -2144896909,    -105372028,
    -2146836865,    -52701887,
    -2147483647,    0,
    -2146836865,    52701887,
    -2144896909,    105372028,
    -2141664947,    157978697,
    -2137142926,    210490206,
    -2131333571,    262874923,
    -2124240379,    315101294,
    -2115867625,    367137860,
    -2106220351,    418953276,
    -2095304369,    470516330,
    -2083126253,    521795963,
    -2069693341,    572761285,
    -2055013722,    623381597,
    -2039096240,    673626408,
    -2021950483,    723465451,
    -2003586778,    772868706,
    -1984016188,    821806413,
    -1963250500,    870249095,
    -1941302224,    918167571,
    -1918184580,    965532978,
    -1893911493,    1012316784,
    -1868497585,    1058490807,
    -1841958164,    1104027236,
    -1814309215,    1148898640,
    -1785567395,    1193077990,
    -1755750016,    1236538675,
    -1724875039,    1279254515,
    -1692961061,    1321199780,
    -1660027308,    1362349204,
    -1626093615,    1402677999,
    -1591180425,    1442161874,
    -1555308767,    1480777044,
    -1518500249,    1518500249,
    -1480777044,    1555308767,
    -1442161874,    1591180425,
    -1402677999,    1626093615,
    -1362349204,    1660027308,
    -1321199780,    1692961061,
    -1279254515,    1724875039,
    -1236538675,    1755750016,
    -1193077990,    1785567395,
    -1148898640,    1814309215,
    -1104027236,    1841958164,
    -1058490807,    1868497585,
    -1012316784,    1893911493,
    -965532978, 1918184580,
    -918167571, 1941302224,
    -870249095, 1963250500,
    -821806413, 1984016188,
    -772868706, 2003586778,
    -723465451, 2021950483,
    -673626408, 2039096240,
    -623381597, 2055013722,
    -572761285, 2069693341,
    -521795963, 2083126253,
    -470516330, 2095304369,
    -418953276, 2106220351,
    -367137860, 2115867625,
    -315101294, 2124240379,
    -262874923, 2131333571,
    -210490206, 2137142926,
    -157978697, 2141664947,
    -105372028, 2144896909,
    -52701887,  2146836865,
    0,  2147483647,
};

const int ifft_table_256[] =
{
    2147483647, 0,
    2144896909, 105372028,
    2137142926, 210490206,
    2124240379, 315101294,
    2106220351, 418953276,
    2083126253, 521795963,
    2055013722, 623381597,
    2021950483, 723465451,
    1984016188, 821806413,
    1941302224, 918167571,
    1893911493, 1012316784,
    1841958164, 1104027236,
    1785567395, 1193077990,
    1724875039, 1279254515,
    1660027308, 1362349204,
    1591180425, 1442161874,
    1518500249, 1518500249,
    1442161874, 1591180425,
    1362349204, 1660027308,
    1279254515, 1724875039,
    1193077990, 1785567395,
    1104027236, 1841958164,
    1012316784, 1893911493,
    918167571,  1941302224,
    821806413,  1984016188,
    723465451,  2021950483,
    623381597,  2055013722,
    521795963,  2083126253,
    418953276,  2106220351,
    315101294,  2124240379,
    210490206,  2137142926,
    105372028,  2144896909,
    0,  2147483647,
    -105372028, 2144896909,
    -210490206, 2137142926,
    -315101294, 2124240379,
    -418953276, 2106220351,
    -521795963, 2083126253,
    -623381597, 2055013722,
    -723465451, 2021950483,
    -821806413, 1984016188,
    -918167571, 1941302224,
    -1012316784,    1893911493,
    -1104027236,    1841958164,
    -1193077990,    1785567395,
    -1279254515,    1724875039,
    -1362349204,    1660027308,
    -1442161874,    1591180425,
    -1518500249,    1518500249,
    -1591180425,    1442161874,
    -1660027308,    1362349204,
    -1724875039,    1279254515,
    -1785567395,    1193077990,
    -1841958164,    1104027236,
    -1893911493,    1012316784,
    -1941302224,    918167571,
    -1984016188,    821806413,
    -2021950483,    723465451,
    -2055013722,    623381597,
    -2083126253,    521795963,
    -2106220351,    418953276,
    -2124240379,    315101294,
    -2137142926,    210490206,
    -2144896909,    105372028,
    -2147483647,    0,
    -2144896909,    -105372028,
    -2137142926,    -210490206,
    -2124240379,    -315101294,
    -2106220351,    -418953276,
    -2083126253,    -521795963,
    -2055013722,    -623381597,
    -2021950483,    -723465451,
    -1984016188,    -821806413,
    -1941302224,    -918167571,
    -1893911493,    -1012316784,
    -1841958164,    -1104027236,
    -1785567395,    -1193077990,
    -1724875039,    -1279254515,
    -1660027308,    -1362349204,
    -1591180425,    -1442161874,
    -1518500249,    -1518500249,
    -1442161874,    -1591180425,
    -1362349204,    -1660027308,
    -1279254515,    -1724875039,
    -1193077990,    -1785567395,
    -1104027236,    -1841958164,
    -1012316784,    -1893911493,
    -918167571, -1941302224,
    -821806413, -1984016188,
    -723465451, -2021950483,
    -623381597, -2055013722,
    -521795963, -2083126253,
    -418953276, -2106220351,
    -315101294, -2124240379,
    -210490206, -2137142926,
    -105372028, -2144896909,
    0,  -2147483647,
    105372028,  -2144896909,
    210490206,  -2137142926,
    315101294,  -2124240379,
    418953276,  -2106220351,
    521795963,  -2083126253,
    623381597,  -2055013722,
    723465451,  -2021950483,
    821806413,  -1984016188,
    918167571,  -1941302224,
    1012316784, -1893911493,
    1104027236, -1841958164,
    1193077990, -1785567395,
    1279254515, -1724875039,
    1362349204, -1660027308,
    1442161874, -1591180425,
    1518500249, -1518500249,
    1591180425, -1442161874,
    1660027308, -1362349204,
    1724875039, -1279254515,
    1785567395, -1193077990,
    1841958164, -1104027236,
    1893911493, -1012316784,
    1941302224, -918167571,
    1984016188, -821806413,
    2021950483, -723465451,
    2055013722, -623381597,
    2083126253, -521795963,
    2106220351, -418953276,
    2124240379, -315101294,
    2137142926, -210490206,
    2144896909, -105372028,

};

const int ifft_supertable_256[] =
{
    -52701887,  2146836865,
    -105372028, 2144896909,
    -157978697, 2141664947,
    -210490206, 2137142926,
    -262874923, 2131333571,
    -315101294, 2124240379,
    -367137860, 2115867625,
    -418953276, 2106220351,
    -470516330, 2095304369,
    -521795963, 2083126253,
    -572761285, 2069693341,
    -623381597, 2055013722,
    -673626408, 2039096240,
    -723465451, 2021950483,
    -772868706, 2003586778,
    -821806413, 1984016188,
    -870249095, 1963250500,
    -918167571, 1941302224,
    -965532978, 1918184580,
    -1012316784,    1893911493,
    -1058490807,    1868497585,
    -1104027236,    1841958164,
    -1148898640,    1814309215,
    -1193077990,    1785567395,
    -1236538675,    1755750016,
    -1279254515,    1724875039,
    -1321199780,    1692961061,
    -1362349204,    1660027308,
    -1402677999,    1626093615,
    -1442161874,    1591180425,
    -1480777044,    1555308767,
    -1518500249,    1518500249,
    -1555308767,    1480777044,
    -1591180425,    1442161874,
    -1626093615,    1402677999,
    -1660027308,    1362349204,
    -1692961061,    1321199780,
    -1724875039,    1279254515,
    -1755750016,    1236538675,
    -1785567395,    1193077990,
    -1814309215,    1148898640,
    -1841958164,    1104027236,
    -1868497585,    1058490807,
    -1893911493,    1012316784,
    -1918184580,    965532978,
    -1941302224,    918167571,
    -1963250500,    870249095,
    -1984016188,    821806413,
    -2003586778,    772868706,
    -2021950483,    723465451,
    -2039096240,    673626408,
    -2055013722,    623381597,
    -2069693341,    572761285,
    -2083126253,    521795963,
    -2095304369,    470516330,
    -2106220351,    418953276,
    -2115867625,    367137860,
    -2124240379,    315101294,
    -2131333571,    262874923,
    -2137142926,    210490206,
    -2141664947,    157978697,
    -2144896909,    105372028,
    -2146836865,    52701887,
    -2147483647,    0,
    -2146836865,    -52701887,
    -2144896909,    -105372028,
    -2141664947,    -157978697,
    -2137142926,    -210490206,
    -2131333571,    -262874923,
    -2124240379,    -315101294,
    -2115867625,    -367137860,
    -2106220351,    -418953276,
    -2095304369,    -470516330,
    -2083126253,    -521795963,
    -2069693341,    -572761285,
    -2055013722,    -623381597,
    -2039096240,    -673626408,
    -2021950483,    -723465451,
    -2003586778,    -772868706,
    -1984016188,    -821806413,
    -1963250500,    -870249095,
    -1941302224,    -918167571,
    -1918184580,    -965532978,
    -1893911493,    -1012316784,
    -1868497585,    -1058490807,
    -1841958164,    -1104027236,
    -1814309215,    -1148898640,
    -1785567395,    -1193077990,
    -1755750016,    -1236538675,
    -1724875039,    -1279254515,
    -1692961061,    -1321199780,
    -1660027308,    -1362349204,
    -1626093615,    -1402677999,
    -1591180425,    -1442161874,
    -1555308767,    -1480777044,
    -1518500249,    -1518500249,
    -1480777044,    -1555308767,
    -1442161874,    -1591180425,
    -1402677999,    -1626093615,
    -1362349204,    -1660027308,
    -1321199780,    -1692961061,
    -1279254515,    -1724875039,
    -1236538675,    -1755750016,
    -1193077990,    -1785567395,
    -1148898640,    -1814309215,
    -1104027236,    -1841958164,
    -1058490807,    -1868497585,
    -1012316784,    -1893911493,
    -965532978, -1918184580,
    -918167571, -1941302224,
    -870249095, -1963250500,
    -821806413, -1984016188,
    -772868706, -2003586778,
    -723465451, -2021950483,
    -673626408, -2039096240,
    -623381597, -2055013722,
    -572761285, -2069693341,
    -521795963, -2083126253,
    -470516330, -2095304369,
    -418953276, -2106220351,
    -367137860, -2115867625,
    -315101294, -2124240379,
    -262874923, -2131333571,
    -210490206, -2137142926,
    -157978697, -2141664947,
    -105372028, -2144896909,
    -52701887,  -2146836865,
    0,  -2147483647,

};



const int fft_table_512[] =
{
    2147483647, 0,
    2146836865, -52701887,
    2144896909, -105372028,
    2141664947, -157978697,
    2137142926, -210490206,
    2131333571, -262874923,
    2124240379, -315101294,
    2115867625, -367137860,
    2106220351, -418953276,
    2095304369, -470516330,
    2083126253, -521795963,
    2069693341, -572761285,
    2055013722, -623381597,
    2039096240, -673626408,
    2021950483, -723465451,
    2003586778, -772868706,
    1984016188, -821806413,
    1963250500, -870249095,
    1941302224, -918167571,
    1918184580, -965532978,
    1893911493, -1012316784,
    1868497585, -1058490807,
    1841958164, -1104027236,
    1814309215, -1148898640,
    1785567395, -1193077990,
    1755750016, -1236538675,
    1724875039, -1279254515,
    1692961061, -1321199780,
    1660027308, -1362349204,
    1626093615, -1402677999,
    1591180425, -1442161874,
    1555308767, -1480777044,
    1518500249, -1518500249,
    1480777044, -1555308767,
    1442161874, -1591180425,
    1402677999, -1626093615,
    1362349204, -1660027308,
    1321199780, -1692961061,
    1279254515, -1724875039,
    1236538675, -1755750016,
    1193077990, -1785567395,
    1148898640, -1814309215,
    1104027236, -1841958164,
    1058490807, -1868497585,
    1012316784, -1893911493,
    965532978, -1918184580,
    918167571, -1941302224,
    870249095, -1963250500,
    821806413, -1984016188,
    772868706, -2003586778,
    723465451, -2021950483,
    673626408, -2039096240,
    623381597, -2055013722,
    572761285, -2069693341,
    521795963, -2083126253,
    470516330, -2095304369,
    418953276, -2106220351,
    367137860, -2115867625,
    315101294, -2124240379,
    262874923, -2131333571,
    210490206, -2137142926,
    157978697, -2141664947,
    105372028, -2144896909,
    52701887, -2146836865,
    0, -2147483647,
    -52701887, -2146836865,
    -105372028, -2144896909,
    -157978697, -2141664947,
    -210490206, -2137142926,
    -262874923, -2131333571,
    -315101294, -2124240379,
    -367137860, -2115867625,
    -418953276, -2106220351,
    -470516330, -2095304369,
    -521795963, -2083126253,
    -572761285, -2069693341,
    -623381597, -2055013722,
    -673626408, -2039096240,
    -723465451, -2021950483,
    -772868706, -2003586778,
    -821806413, -1984016188,
    -870249095, -1963250500,
    -918167571, -1941302224,
    -965532978, -1918184580,
    -1012316784, -1893911493,
    -1058490807, -1868497585,
    -1104027236, -1841958164,
    -1148898640, -1814309215,
    -1193077990, -1785567395,
    -1236538675, -1755750016,
    -1279254515, -1724875039,
    -1321199780, -1692961061,
    -1362349204, -1660027308,
    -1402677999, -1626093615,
    -1442161874, -1591180425,
    -1480777044, -1555308767,
    -1518500249, -1518500249,
    -1555308767, -1480777044,
    -1591180425, -1442161874,
    -1626093615, -1402677999,
    -1660027308, -1362349204,
    -1692961061, -1321199780,
    -1724875039, -1279254515,
    -1755750016, -1236538675,
    -1785567395, -1193077990,
    -1814309215, -1148898640,
    -1841958164, -1104027236,
    -1868497585, -1058490807,
    -1893911493, -1012316784,
    -1918184580, -965532978,
    -1941302224, -918167571,
    -1963250500, -870249095,
    -1984016188, -821806413,
    -2003586778, -772868706,
    -2021950483, -723465451,
    -2039096240, -673626408,
    -2055013722, -623381597,
    -2069693341, -572761285,
    -2083126253, -521795963,
    -2095304369, -470516330,
    -2106220351, -418953276,
    -2115867625, -367137860,
    -2124240379, -315101294,
    -2131333571, -262874923,
    -2137142926, -210490206,
    -2141664947, -157978697,
    -2144896909, -105372028,
    -2146836865, -52701887,
    -2147483647, 0,
    -2146836865, 52701887,
    -2144896909, 105372028,
    -2141664947, 157978697,
    -2137142926, 210490206,
    -2131333571, 262874923,
    -2124240379, 315101294,
    -2115867625, 367137860,
    -2106220351, 418953276,
    -2095304369, 470516330,
    -2083126253, 521795963,
    -2069693341, 572761285,
    -2055013722, 623381597,
    -2039096240, 673626408,
    -2021950483, 723465451,
    -2003586778, 772868706,
    -1984016188, 821806413,
    -1963250500, 870249095,
    -1941302224, 918167571,
    -1918184580, 965532978,
    -1893911493, 1012316784,
    -1868497585, 1058490807,
    -1841958164, 1104027236,
    -1814309215, 1148898640,
    -1785567395, 1193077990,
    -1755750016, 1236538675,
    -1724875039, 1279254515,
    -1692961061, 1321199780,
    -1660027308, 1362349204,
    -1626093615, 1402677999,
    -1591180425, 1442161874,
    -1555308767, 1480777044,
    -1518500249, 1518500249,
    -1480777044, 1555308767,
    -1442161874, 1591180425,
    -1402677999, 1626093615,
    -1362349204, 1660027308,
    -1321199780, 1692961061,
    -1279254515, 1724875039,
    -1236538675, 1755750016,
    -1193077990, 1785567395,
    -1148898640, 1814309215,
    -1104027236, 1841958164,
    -1058490807, 1868497585,
    -1012316784, 1893911493,
    -965532978, 1918184580,
    -918167571, 1941302224,
    -870249095, 1963250500,
    -821806413, 1984016188,
    -772868706, 2003586778,
    -723465451, 2021950483,
    -673626408, 2039096240,
    -623381597, 2055013722,
    -572761285, 2069693341,
    -521795963, 2083126253,
    -470516330, 2095304369,
    -418953276, 2106220351,
    -367137860, 2115867625,
    -315101294, 2124240379,
    -262874923, 2131333571,
    -210490206, 2137142926,
    -157978697, 2141664947,
    -105372028, 2144896909,
    -52701887, 2146836865,
    0, 2147483647,
    52701887, 2146836865,
    105372028, 2144896909,
    157978697, 2141664947,
    210490206, 2137142926,
    262874923, 2131333571,
    315101294, 2124240379,
    367137860, 2115867625,
    418953276, 2106220351,
    470516330, 2095304369,
    521795963, 2083126253,
    572761285, 2069693341,
    623381597, 2055013722,
    673626408, 2039096240,
    723465451, 2021950483,
    772868706, 2003586778,
    821806413, 1984016188,
    870249095, 1963250500,
    918167571, 1941302224,
    965532978, 1918184580,
    1012316784, 1893911493,
    1058490807, 1868497585,
    1104027236, 1841958164,
    1148898640, 1814309215,
    1193077990, 1785567395,
    1236538675, 1755750016,
    1279254515, 1724875039,
    1321199780, 1692961061,
    1362349204, 1660027308,
    1402677999, 1626093615,
    1442161874, 1591180425,
    1480777044, 1555308767,
    1518500249, 1518500249,
    1555308767, 1480777044,
    1591180425, 1442161874,
    1626093615, 1402677999,
    1660027308, 1362349204,
    1692961061, 1321199780,
    1724875039, 1279254515,
    1755750016, 1236538675,
    1785567395, 1193077990,
    1814309215, 1148898640,
    1841958164, 1104027236,
    1868497585, 1058490807,
    1893911493, 1012316784,
    1918184580, 965532978,
    1941302224, 918167571,
    1963250500, 870249095,
    1984016188, 821806413,
    2003586778, 772868706,
    2021950483, 723465451,
    2039096240, 673626408,
    2055013722, 623381597,
    2069693341, 572761285,
    2083126253, 521795963,
    2095304369, 470516330,
    2106220351, 418953276,
    2115867625, 367137860,
    2124240379, 315101294,
    2131333571, 262874923,
    2137142926, 210490206,
    2141664947, 157978697,
    2144896909, 105372028,
    2146836865, 52701887,
};

const int fft_supertable_512[] =
{
    -26352928, -2147321945,
    -52701887, -2146836865,
    -79042909, -2146028479,
    -105372028, -2144896909,
    -131685278, -2143442325,
    -157978697, -2141664947,
    -184248325, -2139565042,
    -210490206, -2137142926,
    -236700388, -2134398965,
    -262874923, -2131333571,
    -289009871, -2127947205,
    -315101294, -2124240379,
    -341145265, -2120213650,
    -367137860, -2115867625,
    -393075166, -2111202958,
    -418953276, -2106220351,
    -444768293, -2100920555,
    -470516330, -2095304369,
    -496193509, -2089372637,
    -521795963, -2083126253,
    -547319836, -2076566159,
    -572761285, -2069693341,
    -598116478, -2062508835,
    -623381597, -2055013722,
    -648552837, -2047209132,
    -673626408, -2039096240,
    -698598533, -2030676268,
    -723465451, -2021950483,
    -748223418, -2012920200,
    -772868706, -2003586778,
    -797397602, -1993951624,
    -821806413, -1984016188,
    -846091463, -1973781966,
    -870249095, -1963250500,
    -894275670, -1952423376,
    -918167571, -1941302224,
    -941921200, -1929888719,
    -965532978, -1918184580,
    -988999351, -1906191569,
    -1012316784, -1893911493,
    -1035481765, -1881346201,
    -1058490807, -1868497585,
    -1081340445, -1855367580,
    -1104027236, -1841958164,
    -1126547765, -1828271355,
    -1148898640, -1814309215,
    -1171076495, -1800073848,
    -1193077990, -1785567395,
    -1214899812, -1770792043,
    -1236538675, -1755750016,
    -1257991319, -1740443580,
    -1279254515, -1724875039,
    -1300325059, -1709046738,
    -1321199780, -1692961061,
    -1341875532, -1676620431,
    -1362349204, -1660027308,
    -1382617710, -1643184190,
    -1402677999, -1626093615,
    -1422527050, -1608758157,
    -1442161874, -1591180425,
    -1461579513, -1573363067,
    -1480777044, -1555308767,
    -1499751575, -1537020243,
    -1518500249, -1518500249,
    -1537020243, -1499751575,
    -1555308767, -1480777044,
    -1573363067, -1461579513,
    -1591180425, -1442161874,
    -1608758157, -1422527050,
    -1626093615, -1402677999,
    -1643184190, -1382617710,
    -1660027308, -1362349204,
    -1676620431, -1341875532,
    -1692961061, -1321199780,
    -1709046738, -1300325059,
    -1724875039, -1279254515,
    -1740443580, -1257991319,
    -1755750016, -1236538675,
    -1770792043, -1214899812,
    -1785567395, -1193077990,
    -1800073848, -1171076495,
    -1814309215, -1148898640,
    -1828271355, -1126547765,
    -1841958164, -1104027236,
    -1855367580, -1081340445,
    -1868497585, -1058490807,
    -1881346201, -1035481765,
    -1893911493, -1012316784,
    -1906191569, -988999351,
    -1918184580, -965532978,
    -1929888719, -941921200,
    -1941302224, -918167571,
    -1952423376, -894275670,
    -1963250500, -870249095,
    -1973781966, -846091463,
    -1984016188, -821806413,
    -1993951624, -797397602,
    -2003586778, -772868706,
    -2012920200, -748223418,
    -2021950483, -723465451,
    -2030676268, -698598533,
    -2039096240, -673626408,
    -2047209132, -648552837,
    -2055013722, -623381597,
    -2062508835, -598116478,
    -2069693341, -572761285,
    -2076566159, -547319836,
    -2083126253, -521795963,
    -2089372637, -496193509,
    -2095304369, -470516330,
    -2100920555, -444768293,
    -2106220351, -418953276,
    -2111202958, -393075166,
    -2115867625, -367137860,
    -2120213650, -341145265,
    -2124240379, -315101294,
    -2127947205, -289009871,
    -2131333571, -262874923,
    -2134398965, -236700388,
    -2137142926, -210490206,
    -2139565042, -184248325,
    -2141664947, -157978697,
    -2143442325, -131685278,
    -2144896909, -105372028,
    -2146028479, -79042909,
    -2146836865, -52701887,
    -2147321945, -26352928,
    -2147483647, 0,
};

const int ifft_table_512[] =
{
    2147483647, 0,
    2146836865, 52701887,
    2144896909, 105372028,
    2141664947, 157978697,
    2137142926, 210490206,
    2131333571, 262874923,
    2124240379, 315101294,
    2115867625, 367137860,
    2106220351, 418953276,
    2095304369, 470516330,
    2083126253, 521795963,
    2069693341, 572761285,
    2055013722, 623381597,
    2039096240, 673626408,
    2021950483, 723465451,
    2003586778, 772868706,
    1984016188, 821806413,
    1963250500, 870249095,
    1941302224, 918167571,
    1918184580, 965532978,
    1893911493, 1012316784,
    1868497585, 1058490807,
    1841958164, 1104027236,
    1814309215, 1148898640,
    1785567395, 1193077990,
    1755750016, 1236538675,
    1724875039, 1279254515,
    1692961061, 1321199780,
    1660027308, 1362349204,
    1626093615, 1402677999,
    1591180425, 1442161874,
    1555308767, 1480777044,
    1518500249, 1518500249,
    1480777044, 1555308767,
    1442161874, 1591180425,
    1402677999, 1626093615,
    1362349204, 1660027308,
    1321199780, 1692961061,
    1279254515, 1724875039,
    1236538675, 1755750016,
    1193077990, 1785567395,
    1148898640, 1814309215,
    1104027236, 1841958164,
    1058490807, 1868497585,
    1012316784, 1893911493,
    965532978, 1918184580,
    918167571, 1941302224,
    870249095, 1963250500,
    821806413, 1984016188,
    772868706, 2003586778,
    723465451, 2021950483,
    673626408, 2039096240,
    623381597, 2055013722,
    572761285, 2069693341,
    521795963, 2083126253,
    470516330, 2095304369,
    418953276, 2106220351,
    367137860, 2115867625,
    315101294, 2124240379,
    262874923, 2131333571,
    210490206, 2137142926,
    157978697, 2141664947,
    105372028, 2144896909,
    52701887, 2146836865,
    0, 2147483647,
    -52701887, 2146836865,
    -105372028, 2144896909,
    -157978697, 2141664947,
    -210490206, 2137142926,
    -262874923, 2131333571,
    -315101294, 2124240379,
    -367137860, 2115867625,
    -418953276, 2106220351,
    -470516330, 2095304369,
    -521795963, 2083126253,
    -572761285, 2069693341,
    -623381597, 2055013722,
    -673626408, 2039096240,
    -723465451, 2021950483,
    -772868706, 2003586778,
    -821806413, 1984016188,
    -870249095, 1963250500,
    -918167571, 1941302224,
    -965532978, 1918184580,
    -1012316784, 1893911493,
    -1058490807, 1868497585,
    -1104027236, 1841958164,
    -1148898640, 1814309215,
    -1193077990, 1785567395,
    -1236538675, 1755750016,
    -1279254515, 1724875039,
    -1321199780, 1692961061,
    -1362349204, 1660027308,
    -1402677999, 1626093615,
    -1442161874, 1591180425,
    -1480777044, 1555308767,
    -1518500249, 1518500249,
    -1555308767, 1480777044,
    -1591180425, 1442161874,
    -1626093615, 1402677999,
    -1660027308, 1362349204,
    -1692961061, 1321199780,
    -1724875039, 1279254515,
    -1755750016, 1236538675,
    -1785567395, 1193077990,
    -1814309215, 1148898640,
    -1841958164, 1104027236,
    -1868497585, 1058490807,
    -1893911493, 1012316784,
    -1918184580, 965532978,
    -1941302224, 918167571,
    -1963250500, 870249095,
    -1984016188, 821806413,
    -2003586778, 772868706,
    -2021950483, 723465451,
    -2039096240, 673626408,
    -2055013722, 623381597,
    -2069693341, 572761285,
    -2083126253, 521795963,
    -2095304369, 470516330,
    -2106220351, 418953276,
    -2115867625, 367137860,
    -2124240379, 315101294,
    -2131333571, 262874923,
    -2137142926, 210490206,
    -2141664947, 157978697,
    -2144896909, 105372028,
    -2146836865, 52701887,
    -2147483647, 0,
    -2146836865, -52701887,
    -2144896909, -105372028,
    -2141664947, -157978697,
    -2137142926, -210490206,
    -2131333571, -262874923,
    -2124240379, -315101294,
    -2115867625, -367137860,
    -2106220351, -418953276,
    -2095304369, -470516330,
    -2083126253, -521795963,
    -2069693341, -572761285,
    -2055013722, -623381597,
    -2039096240, -673626408,
    -2021950483, -723465451,
    -2003586778, -772868706,
    -1984016188, -821806413,
    -1963250500, -870249095,
    -1941302224, -918167571,
    -1918184580, -965532978,
    -1893911493, -1012316784,
    -1868497585, -1058490807,
    -1841958164, -1104027236,
    -1814309215, -1148898640,
    -1785567395, -1193077990,
    -1755750016, -1236538675,
    -1724875039, -1279254515,
    -1692961061, -1321199780,
    -1660027308, -1362349204,
    -1626093615, -1402677999,
    -1591180425, -1442161874,
    -1555308767, -1480777044,
    -1518500249, -1518500249,
    -1480777044, -1555308767,
    -1442161874, -1591180425,
    -1402677999, -1626093615,
    -1362349204, -1660027308,
    -1321199780, -1692961061,
    -1279254515, -1724875039,
    -1236538675, -1755750016,
    -1193077990, -1785567395,
    -1148898640, -1814309215,
    -1104027236, -1841958164,
    -1058490807, -1868497585,
    -1012316784, -1893911493,
    -965532978, -1918184580,
    -918167571, -1941302224,
    -870249095, -1963250500,
    -821806413, -1984016188,
    -772868706, -2003586778,
    -723465451, -2021950483,
    -673626408, -2039096240,
    -623381597, -2055013722,
    -572761285, -2069693341,
    -521795963, -2083126253,
    -470516330, -2095304369,
    -418953276, -2106220351,
    -367137860, -2115867625,
    -315101294, -2124240379,
    -262874923, -2131333571,
    -210490206, -2137142926,
    -157978697, -2141664947,
    -105372028, -2144896909,
    -52701887, -2146836865,
    0, -2147483647,
    52701887, -2146836865,
    105372028, -2144896909,
    157978697, -2141664947,
    210490206, -2137142926,
    262874923, -2131333571,
    315101294, -2124240379,
    367137860, -2115867625,
    418953276, -2106220351,
    470516330, -2095304369,
    521795963, -2083126253,
    572761285, -2069693341,
    623381597, -2055013722,
    673626408, -2039096240,
    723465451, -2021950483,
    772868706, -2003586778,
    821806413, -1984016188,
    870249095, -1963250500,
    918167571, -1941302224,
    965532978, -1918184580,
    1012316784, -1893911493,
    1058490807, -1868497585,
    1104027236, -1841958164,
    1148898640, -1814309215,
    1193077990, -1785567395,
    1236538675, -1755750016,
    1279254515, -1724875039,
    1321199780, -1692961061,
    1362349204, -1660027308,
    1402677999, -1626093615,
    1442161874, -1591180425,
    1480777044, -1555308767,
    1518500249, -1518500249,
    1555308767, -1480777044,
    1591180425, -1442161874,
    1626093615, -1402677999,
    1660027308, -1362349204,
    1692961061, -1321199780,
    1724875039, -1279254515,
    1755750016, -1236538675,
    1785567395, -1193077990,
    1814309215, -1148898640,
    1841958164, -1104027236,
    1868497585, -1058490807,
    1893911493, -1012316784,
    1918184580, -965532978,
    1941302224, -918167571,
    1963250500, -870249095,
    1984016188, -821806413,
    2003586778, -772868706,
    2021950483, -723465451,
    2039096240, -673626408,
    2055013722, -623381597,
    2069693341, -572761285,
    2083126253, -521795963,
    2095304369, -470516330,
    2106220351, -418953276,
    2115867625, -367137860,
    2124240379, -315101294,
    2131333571, -262874923,
    2137142926, -210490206,
    2141664947, -157978697,
    2144896909, -105372028,
    2146836865, -52701887,
};

const int ifft_supertable_512[] =
{
    -26352928, 2147321945,
    -52701887, 2146836865,
    -79042909, 2146028479,
    -105372028, 2144896909,
    -131685278, 2143442325,
    -157978697, 2141664947,
    -184248325, 2139565042,
    -210490206, 2137142926,
    -236700388, 2134398965,
    -262874923, 2131333571,
    -289009871, 2127947205,
    -315101294, 2124240379,
    -341145265, 2120213650,
    -367137860, 2115867625,
    -393075166, 2111202958,
    -418953276, 2106220351,
    -444768293, 2100920555,
    -470516330, 2095304369,
    -496193509, 2089372637,
    -521795963, 2083126253,
    -547319836, 2076566159,
    -572761285, 2069693341,
    -598116478, 2062508835,
    -623381597, 2055013722,
    -648552837, 2047209132,
    -673626408, 2039096240,
    -698598533, 2030676268,
    -723465451, 2021950483,
    -748223418, 2012920200,
    -772868706, 2003586778,
    -797397602, 1993951624,
    -821806413, 1984016188,
    -846091463, 1973781966,
    -870249095, 1963250500,
    -894275670, 1952423376,
    -918167571, 1941302224,
    -941921200, 1929888719,
    -965532978, 1918184580,
    -988999351, 1906191569,
    -1012316784, 1893911493,
    -1035481765, 1881346201,
    -1058490807, 1868497585,
    -1081340445, 1855367580,
    -1104027236, 1841958164,
    -1126547765, 1828271355,
    -1148898640, 1814309215,
    -1171076495, 1800073848,
    -1193077990, 1785567395,
    -1214899812, 1770792043,
    -1236538675, 1755750016,
    -1257991319, 1740443580,
    -1279254515, 1724875039,
    -1300325059, 1709046738,
    -1321199780, 1692961061,
    -1341875532, 1676620431,
    -1362349204, 1660027308,
    -1382617710, 1643184190,
    -1402677999, 1626093615,
    -1422527050, 1608758157,
    -1442161874, 1591180425,
    -1461579513, 1573363067,
    -1480777044, 1555308767,
    -1499751575, 1537020243,
    -1518500249, 1518500249,
    -1537020243, 1499751575,
    -1555308767, 1480777044,
    -1573363067, 1461579513,
    -1591180425, 1442161874,
    -1608758157, 1422527050,
    -1626093615, 1402677999,
    -1643184190, 1382617710,
    -1660027308, 1362349204,
    -1676620431, 1341875532,
    -1692961061, 1321199780,
    -1709046738, 1300325059,
    -1724875039, 1279254515,
    -1740443580, 1257991319,
    -1755750016, 1236538675,
    -1770792043, 1214899812,
    -1785567395, 1193077990,
    -1800073848, 1171076495,
    -1814309215, 1148898640,
    -1828271355, 1126547765,
    -1841958164, 1104027236,
    -1855367580, 1081340445,
    -1868497585, 1058490807,
    -1881346201, 1035481765,
    -1893911493, 1012316784,
    -1906191569, 988999351,
    -1918184580, 965532978,
    -1929888719, 941921200,
    -1941302224, 918167571,
    -1952423376, 894275670,
    -1963250500, 870249095,
    -1973781966, 846091463,
    -1984016188, 821806413,
    -1993951624, 797397602,
    -2003586778, 772868706,
    -2012920200, 748223418,
    -2021950483, 723465451,
    -2030676268, 698598533,
    -2039096240, 673626408,
    -2047209132, 648552837,
    -2055013722, 623381597,
    -2062508835, 598116478,
    -2069693341, 572761285,
    -2076566159, 547319836,
    -2083126253, 521795963,
    -2089372637, 496193509,
    -2095304369, 470516330,
    -2100920555, 444768293,
    -2106220351, 418953276,
    -2111202958, 393075166,
    -2115867625, 367137860,
    -2120213650, 341145265,
    -2124240379, 315101294,
    -2127947205, 289009871,
    -2131333571, 262874923,
    -2134398965, 236700388,
    -2137142926, 210490206,
    -2139565042, 184248325,
    -2141664947, 157978697,
    -2143442325, 131685278,
    -2144896909, 105372028,
    -2146028479, 79042909,
    -2146836865, 52701887,
    -2147321945, 26352928,
    -2147483647, 0,
};


#endif
