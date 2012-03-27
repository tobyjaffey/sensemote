#ifndef CONFPROTO_H
#define CONFPROTO_H 1

int hexstring_parse(const char *hexstr, uint8_t *buf, size_t *buflen);
int parse_flashvars(flashvars_t *flashvars, int argc, char *argv[]);

#endif

