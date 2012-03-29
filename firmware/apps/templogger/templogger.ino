/*
Temperature logger with TMP102
Wakes every 20s and sends a message to Pachube.

SCL -> P2_4
SCL -> 10K pullup -> V+

SDA -> P2_2
SDA -> 10K pullup -> V+

ADD0 -> GND
*/

char strbuf[32];
#define TMP102_ADDR 0x48

void read_tmp102()
{
    uint16_t t = i2cRead16(TMP102_ADDR, 0);
    char fraction[8];

    itoa(t >> 8, strbuf);
    strcat(strbuf, ".");
    itoa(((t & 0xFF) >> 4) * 6.25, fraction);
    strcat(strbuf, fraction);
}

void setup()
{
    serialInit();
    i2cInit();
}

void loop()
{
    read_tmp102();
    serialWriteString(strbuf);
    serialWriteString("\r\n");

    pb_put("temp", strbuf, 0);

    sleepQs(80);
}


