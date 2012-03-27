/*
* Copyright (c) 2012, Toby Jaffey <toby@sensemote.com>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "common.h"
#include <string.h>
#include <ctype.h>

#include "json.h"

/*
Shonky JSON parser.
Simply scans for a key and returns the value, ignoring arrays and objects entirely.
Will only work for "key":value or "key":"value"
*/

static BOOLEAN myisspace(char c)
{
    if (c == ' ' || c == '\t')
        return TRUE;
    return FALSE;
}

// token must be "\"token\""
static const char *json_get(const __xdata char *json, const char *token, uint16_t *vallen)
{
    const char *tokenp;
    const char *valp;

    if (NULL == (tokenp = strstr(json, token)))
        return NULL;

    tokenp += strlen(token);

    // skip whitespace
    while(myisspace(*tokenp))
        tokenp++;

    // skip ':'
    if (':' != *tokenp++)
        return NULL;

    // skip whitespace
    while(myisspace(*tokenp))
        tokenp++;

    if ('\"' == *tokenp)  // quoted string
    {
        valp = ++tokenp;
        // find end of value
        while(*tokenp != 0 && *tokenp != '\"')
            tokenp++;
    }
    else
    {
        valp = tokenp;
        // find end of value
        while(*tokenp != 0 && !myisspace(*tokenp) && *tokenp != ',' && *tokenp != '{' && *tokenp != '}' && *tokenp != '[' && *tokenp != ']')
            tokenp++;
    }

    *vallen = tokenp - valp;

    return valp;
}


void json_getstr(const __xdata char *json, const char *token, char *buf, uint16_t buflen)
{
    const char *p;
    uint16_t len;

    buf[0] = 0;

    if (NULL != (p = json_get(json, token, &len)) && len < buflen)
    {
        memcpy(buf, p, len);
        buf[len] = 0;
    }
}

