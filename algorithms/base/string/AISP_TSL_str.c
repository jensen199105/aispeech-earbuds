#include "AISP_TSL_str.h"

#define STR_NULL ((void *)0)

S32 AISP_TSL_isspace(S32 iChr)
{
    //return iChr == ' ' || iChr == '\t' || iChr == '\r' || iChr == '\n' || iChr == '\0';
    return iChr == ' ' || iChr == '\t' || iChr == '\r' || iChr == '\n';
}

S32 AISP_TSL_strlen(const S8 *pcStr)
{
    S32 iLen;

    for (iLen = 0; *pcStr != '\0'; pcStr++)
    {
        iLen++;
    }

    return iLen;
}

F32 AISP_TSL_strtof(const S8 *pcStr)
{
    F32 fResult = 0.0f;
    F32 fFact = 1.0f;
    S32 iPointFlag = 0;
    S32 iNum;
    const S8 *p = pcStr;

    if (*p == '-')
    {
        p++;
        fFact = -1;
    }
    else
        if (*p == '+')
        {
            p++;
        }

    for (; *p; p++)
    {
        if (*p == '.')
        {
            iPointFlag = 1;
            continue;
        }

        iNum = *p - '0';

        if (iNum < 0 || iNum > 9)
        {
            return 0.0f;
        }

        if (iPointFlag)
        {
            fFact /= 10.0f;
        }

        fResult = fResult * 10.0f + (F32)iNum;
    }

    return fResult * fFact;
}

S8 *AISP_TSL_strncat(S8 *dest, const S8 *src, S32 count)
{
    S8 *tmp = dest;

    while (*dest)
    {
        dest++;
    }

    while (count && (*src != '\0'))
    {
        *dest++ = *src++;
        --count;
    }

    dest++;
    *dest = '\0';
    return tmp;
}



VOID AISP_TSL_itoa(S32 n, S8 *s)
{
    S32 i = 0, j = 0, sign = 0;
    S8 tempChar;
    S8 len = 0;

    if (n < 0)
    {
        sign = -1;
        n = -n;
    }

    do
    {
        s[i++] = n % 10 + '0'; //get next num
        len ++;
    }
    while ((n /= 10) > 0); //delete current character

    if (sign < 0)
    {
        s[i++] = '-';
        len ++;
    }

    s[i] = '\0';
    len ++;

    for (j = 0; j < len / 2; j++) //invers
    {
        tempChar = s[j];
        s[j] = s[len - j - 2];
        s[len - j - 2] = tempChar;
    }
}
VOID *AISP_TSL_memset(VOID *s, S32 c, U64 count)
{
    S8 *xs = (S8 *) s;

    while (count--)
    {
        *xs++ = (S8)c;
    }

    return s;
}



S32 AISP_TSL_strcmp(const S8 *cs, const S8 *ct)
{
    U8 c1, c2;

    while (1)
    {
        c1 = *cs++;
        c2 = *ct++;

        if (c1 != c2)
        {
            return c1 < c2 ? -1 : 1;
        }

        if (!c1)
        {
            break;
        }
    }

    return 0;
}


S8 *AISP_TSL_strcpy(S8 *dest, const S8 *src)
{
    S8 *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        /* nothing */;

    return tmp;
}

S32 AISP_TSL_strncmp(const S8 *cs, const S8 *ct, S32 count)
{
    U8 c1, c2;

    while (count)
    {
        c1 = *cs++;
        c2 = *ct++;

        if (c1 != c2)
        {
            return c1 < c2 ? -1 : 1;
        }

        if (!c1)
        {
            break;
        }

        count--;
    }

    return 0;
}


VOID *AISP_TSL_memcpy(VOID *dest, const VOID *src, U64 count)
{
    S8 *tmp = (S8 *) dest, *s = (S8 *) src;

    while (count--)
    {
        *tmp++ = *s++;
    }

    return dest;
}



VOID *AISP_TSL_memmove(VOID *dest, const VOID *src, U64 count)
{
    S8 *tmp, *s;

    if (dest <= src)
    {
        tmp = (S8 *) dest;
        s = (S8 *) src;

        while (count--)
        {
            *tmp++ = *s++;
        }
    }
    else
    {
        tmp = (S8 *) dest + count;
        s = (S8 *) src + count;

        while (count--)
        {
            *--tmp = *--s;
        }
    }

    return dest;
}


S32 AISP_TSL_memcmp(const VOID *cs, const VOID *ct, U64 count)
{
    const U8 *su1, *su2;
    S8 res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
    {
        if ((res = (U8)(*su1 - *su2)) != 0)
        {
            break;
        }
    }

    return res;
}


S8 *AISP_TSL_strstr(const S8 *s1, const S8 *s2)
{
    S32 l1, l2;
    l2 = AISP_TSL_strlen(s2);

    if (!l2)
    {
        return (S8 *) s1;
    }

    l1 = AISP_TSL_strlen(s1);

    while (l1 >= l2)
    {
        l1--;

        if (!AISP_TSL_memcmp(s1, s2, l2))
        {
            return (S8 *) s1;
        }

        s1++;
    }

    return STR_NULL;
}

U32 AISP_TSL_atoi(const S8 *str)
{
    const S8 *cp;
    U32 data = 0;

    for (cp = str, data = 0; *cp != 0; ++cp)
    {
        if (*cp >= '0' && *cp <= '9')
        {
            data = data * 10 + *cp - '0';
        }
        else
        {
            break;
        }
    }

    return data;
}

S8 *AISP_TSL_strchr(const S8 *s, S32 c)
{
    for (; *s != (S8) c; ++s)
    {
        if (*s == '\0')
        {
            return 0;
        }
    }

    return (S8 *) s;
}

static U64 AISP_TSL_strspn(const S8 *s, const S8 *accept)
{
    const S8 *p;
    const S8 *a;
    U64 count = 0;

    for (p = s; *p != '\0'; ++p)
    {
        for (a = accept; *a != '\0'; ++a)
        {
            if (*p == *a)
            {
                break;
            }
        }

        if (*a == '\0')
        {
            return count;
        }

        ++count;
    }

    return count;
}

static S8 *AISP_TSL_strpbrk(const S8 *cs, const S8 *ct)
{
    const S8 *sc1, *sc2;

    for (sc1 = cs; *sc1 != '\0'; ++sc1)
    {
        for (sc2 = ct; *sc2 != '\0'; ++sc2)
        {
            if (*sc1 == *sc2)
            {
                return (S8 *) sc1;
            }
        }
    }

    return STR_NULL;
}



S8 *AISP_TSL_strtok_r(S8 *s, const S8 *delim, S8 **save_ptr)
{
    S8 *token;

    if (s == STR_NULL)
    {
        s = *save_ptr;
    }

    /* Scan leading delimiters.  */
    s += AISP_TSL_strspn(s, delim);

    if (*s == '\0')
    {
        return STR_NULL;
    }

    /* Find the end of the token.  */
    token = s;
    s = AISP_TSL_strpbrk(token, delim);

    if (s == STR_NULL)
        /* This token finishes the string.  */
    {
        *save_ptr = AISP_TSL_strchr(token, '\0');
    }
    else
    {
        /* Terminate the token and make *SAVE_PTR point past it.  */
        *s = '\0';
        *save_ptr = s + 1;
    }

    return token;
}
