// fade LED

int val = 0;
int dir = 1;
void setup()
{
    pinMode(19, OUTPUT);
}
void loop()
{
    analogWrite(19, val);
    delay(10);
    val += dir;
    if (val == 0 || val == 255)
        dir = -dir;
}

