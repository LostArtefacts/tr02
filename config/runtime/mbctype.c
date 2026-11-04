#include <windows.h>

typedef struct {
    int code_page;
    unsigned short mbulinfo[6];
    unsigned char rgrange[4][8];
} code_page_info;

extern unsigned char _mbctype[257];
extern int __mbcodepage;
extern int __mblcid;
extern unsigned short __mbulinfo[6];
extern int fSystemSet;
extern char __rgctypeflag[4];
extern code_page_info __rgcode_page_info[5];

int getSystemCP(int codepage);
int CPtoLCID(int codepage);
void setSBUpLow(void);

void setSBCS(void)
{
    int i;

    for (i = 0; i < 257; i++)
        _mbctype[i] = 0;
    __mbcodepage = 0;
    __mblcid = 0;
    for (i = 0; i < 6; i++)
        __mbulinfo[i] = 0;
}

int __cdecl _setmbcp(int codepage)
{
    unsigned int icp;
    unsigned int irg;
    unsigned int ich;
    unsigned char *rgptr;
    CPINFO cpinfo;

    codepage = getSystemCP(codepage);
    if (codepage == __mbcodepage)
        return 0;
    if (codepage == 0)
    {
        setSBCS();
        setSBUpLow();
        return 0;
    }
    for (icp = 0; icp < 5; icp++)
    {
        if (__rgcode_page_info[icp].code_page == codepage)
        {
            for (ich = 0; ich < 257; ich++)
                _mbctype[ich] = 0;
            for (irg = 0; irg < 4; irg++)
            {
                for (rgptr = (unsigned char *)__rgcode_page_info[icp].rgrange[irg];
                     rgptr[0] && rgptr[1]; rgptr += 2)
                {
                    for (ich = rgptr[0]; ich <= rgptr[1]; ich++)
                        _mbctype[ich + 1] |= __rgctypeflag[irg];
                }
            }
            __mbcodepage = codepage;
            __mblcid = CPtoLCID(__mbcodepage);
            for (irg = 0; irg < 6; irg++)
                __mbulinfo[irg] = __rgcode_page_info[icp].mbulinfo[irg];
            setSBUpLow();
            return 0;
        }
    }
    if (GetCPInfo(codepage, &cpinfo) == TRUE)
    {
        BYTE *lbptr;

        for (ich = 0; ich < 257; ich++)
            _mbctype[ich] = 0;
        __mbcodepage = codepage;
        __mblcid = 0;
        if (cpinfo.MaxCharSize > 1)
        {
            for (lbptr = cpinfo.LeadByte; *lbptr && *(lbptr + 1); lbptr += 2)
            {
                for (ich = *lbptr; ich <= *(lbptr + 1); ich++)
                    _mbctype[ich + 1] |= 4;
            }
            for (ich = 0x01; ich < 0xFF; ich++)
                _mbctype[ich + 1] |= 8;
            __mblcid = CPtoLCID(__mbcodepage);
        }
        for (irg = 0; irg < 6; irg++)
            __mbulinfo[irg] = 0;
        setSBUpLow();
        return 0;
    }
    if (fSystemSet)
    {
        setSBCS();
        setSBUpLow();
        return 0;
    }
    return -1;
}
