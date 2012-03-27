char targetValue[8];
int storedseq = 0;
uint8_t polls = 0;

void click()
{
    cons_putsln("click");
    strcpy(targetValue, "1");
}

void acked()
{
    digitalWrite(20, LOW);
}

void putcomplete()
{
    cons_putsln("complete ");
    cons_putdec(pachubeSeq());
    cons_putsln("");
    if (pachubeSeq() == storedseq)
    {
        cons_putsln("match");
        targetValue[0] = 0;
        digitalWrite(20, LOW);
        polls = 0;
    }
}

void setup()
{
    pinMode(13, INPUT);
    attachInterrupt(13, click, FALLING);

    pinMode(20, OUTPUT);
    digitalWrite(20, LOW);

    targetValue[0] = 0;
}

void loop()
{
    if (targetValue[0] != 0)
    {
        digitalWrite(20, HIGH);
        if (polls-- == 0)
        {
            storedseq = pachubePut("switch", targetValue, putcomplete);
            cons_puts("tx seq="); cons_putdec(storedseq);
            cons_putsln("");
            polls = 5;
        }
        else
        {
            cons_putc('#');
            pb_poll();
        }
    }

    if (targetValue[0] != 0)
        sleep_powersave(4);
    else
        sleep_powersave(80);
    cons_putc('.');
}

