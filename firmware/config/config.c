#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "common.h"
#pragma pack(push,1)  /* push current alignment to stack */
#include "flashvars.h"
#pragma pack(pop)

static struct option long_options[] =
{
    {"eui64",   required_argument, 0, 'e'},
    {"mac",     required_argument, 0, 'm'},
    {"server",  required_argument, 0, 's'},
    {"port",    required_argument, 0, 'p'},
    {"apikey",  required_argument, 0, 'a'},
    {"feedid",  required_argument, 0, 'f'},
    {"keyenc",  required_argument, 0, '1'},
    {"keymac",  required_argument, 0, '2'},
    {0, 0, 0, 0}
};

uint8_t tolowercase(uint8_t ch)
{
    if ((ch >= 'A') && (ch <= 'Z'))
        return ch + 0x20;       // Convert uppercase to lowercase
    return ch;      // Simply return original character if it doesn't require any adjustments
}

int8_t parseHexDigit(uint8_t digit)
{
    digit = tolowercase(digit);
    if (isdigit(digit))
        return (int8_t)digit - '0';
    if ((digit >= 'a') && (digit <= 'f'))
        return (int8_t)digit + 0xA - 'a';
    return -1;      // Error case - input wasn't a valid hex digit
}

int hexstring_parse(const char *hexstr, uint8_t *buf, size_t *buflen)
{
    size_t hexstrlen = strlen(hexstr);
    size_t i;

    if (hexstrlen & 0x1)
    {
        fprintf(stderr, "odd number of hex bytes!\n");
        return 1;
    }

    if (hexstrlen/2 > *buflen)
        hexstrlen = (*buflen) * 2;

    for (i=0;i<hexstrlen;i+=2)
    {
        int8_t a, b;
        if (-1 == (a = parseHexDigit(hexstr[i])))
            return 1;
        if (-1 == (b = parseHexDigit(hexstr[i+1])))
            return 1;
        *buf++ = (a << 4) | b;
    }

    *buflen = hexstrlen/2;

    return 0;
}

int parse_flashvars(flashvars_t *flashvars, int argc, char *argv[])
{
    int option_index;
    int c;
    size_t len;

    memset(flashvars, 0, sizeof(flashvars));

    // some sane defaults
    strcpy((char *)flashvars->SERVER_HOST, "www.hodgepig.org");
    flashvars->SERVER_PORT[0] = 54321 >> 8;
    flashvars->SERVER_HOST[1] = 54321 & 0xFF;
    flashvars->MAGIC = MAGIC_OK;

    while(1)
    {
        c = getopt_long (argc, argv, ":e:m:s:p:1:2:", long_options, &option_index);
        if (c == -1)
            break;
        switch(c)
        {
            case 'e':
                len = 8;
                if (0 != hexstring_parse(optarg, flashvars->EUI64, &len) || len != 8)
                    return 1;
            break;
            case '1':
                len = 16;
                if (0 != hexstring_parse(optarg, flashvars->KEY_ENC, &len) || len != 16)
                    return 1;
            break;
            case '2':
                len = 16;
                if (0 != hexstring_parse(optarg, flashvars->KEY_MAC, &len) || len != 16)
                    return 1;
            break;
            case 'm':
                len = 6;
                if (0 != hexstring_parse(optarg, flashvars->MAC, &len) || len != 6)
                    return 1;
            break;
            case 's':
                strncpy((char *)flashvars->SERVER_HOST, optarg, sizeof(flashvars->SERVER_HOST));
            break;
            case 'a':
                strncpy((char *)flashvars->APIKEY, optarg, sizeof(flashvars->APIKEY));
            break;
            case 'f':
                strncpy((char *)flashvars->FEEDID, optarg, sizeof(flashvars->FEEDID));
            break;
            case 'p':
                flashvars->SERVER_PORT[0] = atoi(optarg) >> 8;
                flashvars->SERVER_PORT[1] = atoi(optarg) & 0xFF;
            break;
            default:
                return 1;
            break;
        }
    }
    return 0;
}


