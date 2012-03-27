#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include "common.h"
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
#include "flashvars.h"
#pragma pack(pop)
#include "confproto.h"

#define MAXRECORD 16

void usage(void)
{
    fprintf(stderr, "<input hex file> [options]\n");
    fprintf(stderr, "  --eui64/-e  <HHHHHHHHHHHHHHHH>\n");
    fprintf(stderr, "  --mac/-m    <HHHHHHHHHHHH>\n");
    fprintf(stderr, "  --server/-s <foo.bar.com>\n");
    fprintf(stderr, "  --port/-p   <port>\n");
    fprintf(stderr, "  --feedid/-f <feed id>\n");
    fprintf(stderr, "  --apikey/-a <apikey>\n");
    fprintf(stderr, "  --keyenc/-1  <HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH>\n");
    fprintf(stderr, "  --keymac/-2  <HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH>\n");
    exit(1);
}

static int read_record(uint8_t *buf, size_t buflen, const char *line, bool *eof)
{
    size_t len;
    uint8_t sum = 0;
    uint8_t record_sum;
    uint8_t record_len;
    uint8_t type;
    uint16_t addr;
    uint8_t data[256];
    int i;

    *eof = false;

    if (line[0] != ':')
    {
        fprintf(stderr, "bad hexfile: no start\n");
        return 1;
    }
    line+=1;

    len = 1;
    if (0 != hexstring_parse(line, &record_len, &len))
    {
        fprintf(stderr, "bad hexfile: no len '%s'\n", line);
        return 1;
    }
    line+=len * 2;

    len = 2;
    if (0 != hexstring_parse(line, (uint8_t *)&addr, &len))
    {
        fprintf(stderr, "bad hexfile: no addr\n");
        return 1;
    }
    addr = ntohs(addr);
    line+=len * 2;

    len = 1;
    if (0 != hexstring_parse(line, &type, &len))
    {
        fprintf(stderr, "bad hexfile: no type\n");
        return 1;
    }
    line+=len * 2;

    len = record_len;
    if (0 != hexstring_parse(line, data, &len))
    {
        fprintf(stderr, "bad hexfile: no data\n");
        return 1;
    }
    line+=len * 2;

    len = 1;
    if (0 != hexstring_parse(line, &record_sum, &len))
    {
        fprintf(stderr, "bad hexfile: no sum\n");
        return 1;
    }
    line+=len * 2;

    if (type == 0)
    {
        sum += record_len;
        sum += addr >> 8;
        sum += addr & 0xFF;

        for (i=0;i<record_len;i++)
            sum += data[i];
        sum = (sum ^ 0xFF) + 1;

        if (sum != record_sum)
        {
            fprintf(stderr, "bad hexfile, checksum mismatch\n");
            return 1;
        }

        if (addr + record_len > buflen)
        {
            fprintf(stderr, "bad hexfile, too big @0x%04X len=%d\n", addr, record_len);
            return 1;
        }
        memcpy(buf + addr, data, record_len);
    }
    else
    if (type == 1)
    {
        *eof = true;
    }
    else
    {
        fprintf(stderr, "bad hexfile: unknown record type %02X\n", type);
    }

    return 0;
}

static int read_hexfile(uint8_t *buf, size_t buflen, const char *filename)
{
    FILE *fp;
    char line[1024];
    bool eof;

    if (NULL == (fp = fopen(filename, "ro")))
        return 1;

    while (NULL != fgets(line, sizeof(line), fp))
    {
        char *p = (line + strlen(line)) - 1;

        while(p > line)
        {
            if (isspace((int)(*p)))
                *p = 0;
            p--;
        }

        if (0 != read_record(buf, buflen, line, &eof))
            goto fail;

        if (eof)
            break;
    }

    fclose(fp);
    return 0;
fail:
    fclose(fp);
    return 1;
}


static void write_record(uint16_t addr, const uint8_t *data, uint8_t len)
{
    uint8_t sum = 0;

    printf(":%02X%02X%02X00", len, addr >> 8, addr & 0xFF);
    sum += len;
    sum += addr >> 8;
    sum += addr & 0xFF;

    while(len--)
    {
        sum += *data;
        printf("%02X", *data++);
    }
    printf("%02X\n", (uint8_t)((sum ^ 0xFF)+1));
}

void write_end_record(void)
{
    printf(":00000001FF\n");
}

void write_data(uint16_t addr, const uint8_t *data, size_t len)
{
    while(len > 0)
    {
        if (len >= MAXRECORD)
        {
#if 0
            // skip any record which is entirely FF
            int i;
            bool skip = true;
            for (i=0;i<MAXRECORD;i++)
            {
                if (data[i] != 0xFF)
                {
                    skip = false;
                    break;
                }
            }

            if (!skip)
#endif
                write_record(addr, data, MAXRECORD);
            len -= MAXRECORD;
            addr += MAXRECORD;
            data += MAXRECORD;
        }
        else
        {
            write_record(addr, data, len);
            len = 0;
        }
    }
    write_end_record();
}

int main(int argc, char *argv[])
{
    uint8_t image[32*1024];
    flashvars_t *flashvars = (flashvars_t *)(image + FLASHVARS_ADDR);

    if (sizeof(flashvars_t) > (0x8000 - FLASHVARS_ADDR))
    {
        fprintf(stderr, "!! Flashvars struct too big\n");
        exit(1);
    }

    memset(image, 0xFF, sizeof(image));

    if (argc < 2)
        usage();

    if (read_hexfile(image, sizeof(image), argv[1]) != 0)
    {
        fprintf(stderr, "Bad hexfile %s\n", optarg);
        exit(1);
    }

    if (0 != parse_flashvars(flashvars, argc, argv))
        usage();

    write_data(0x0000, image, sizeof(image));
    return 0;
}
