
#include <stdlib.h>
#include <stdio.h>
#include "protocol.h"

unsigned char isWhitespace(char c)
{
    if ((c==13) || (c==10) || (c==32) || (c=='\t'))
    {
        return 1;
    }
    return 0;
}

unsigned char isDigit(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return 1;
    }
    return 0;
}

unsigned char isAlpha(char c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        return 1;
    }
    return 0;    
}

unsigned char protoSubmitByte(protocolState_t *protocol, char c)
{
    switch(protocol->state)
    {
    case S_IDLE:
        protocol->bufferIdx = 0;
        if (!isWhitespace(c))
        {
            protocol->buffer[protocol->bufferIdx++] = c;
            if (isAlpha(c))
            {
                protocol->state = S_IDLE;
                protocol->buffer[protocol->bufferIdx] = 0;    // add null terminator
                return TOK_CMD; // emit command ID
            }
            else if (isDigit(c))
            {
                protocol->state = S_INTEGER;
                return 0;
            }
            return 0; // unrecognized char - ignore.
        }
        break;
    case S_INTEGER:
        if (isDigit(c))
        {
            protocol->buffer[protocol->bufferIdx] = c;
            if (protocol->bufferIdx < 3)
            {
                protocol->bufferIdx++;
            }
            return 0;
        }
        protocol->state = S_IDLE;
        protocol->buffer[protocol->bufferIdx] = 0;    // add null terminator
        return TOK_INT;   // emit integer ID
    default:
        protocol->state = S_IDLE;
    }
    return 0;
};


unsigned char protoSubmitToken(protocolState_t *protocol, unsigned char token)
{
    unsigned char ret = 0;

    // check for null-token and return
    if (token == 0)
    {
        return 0;
    }

    switch(protocol->tok_state)
    {
    case 0: // idle
        if (token == TOK_CMD) // command
        {
            unsigned char cmd = protocol->buffer[0];
            if (cmd == 'L')
            {
                protocol->tok_state = 10;    // LED state
            }
            else if (cmd == 'M')
            {
                protocol->tok_state = 20;    // Motor state
            }
            else if (cmd == 'U')
            {
                // no arguments
                ret = CMD_UPDATE;
            };
        }
        break;
    case 10:   // LED n
        if (token == TOK_INT)
        {
            protocol->args[0] = atoi(protocol->buffer);
            protocol->tok_state = 11;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 11:   // LED r
        if (token == TOK_INT)
        {
            protocol->args[1] = atoi(protocol->buffer);
            protocol->tok_state = 12;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 12:   // LED g
        if (token == TOK_INT)
        {
            protocol->args[2] = atoi(protocol->buffer);
            protocol->tok_state = 13;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 13:   // LED b
        if (token == TOK_INT)
        {
            protocol->args[3] = atoi(protocol->buffer);
            ret = CMD_LED;
        }
        protocol->tok_state = 0;
        break;
    case 20:   // Motor id
        if (token == TOK_INT)
        {
            protocol->args[0] = atoi(protocol->buffer);
            protocol->tok_state = 21;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 21:   // Motor pos
        if (token == TOK_INT)
        {
            protocol->args[1] = atoi(protocol->buffer);
            ret = CMD_MOTOR;
        }
        protocol->tok_state = 0;
        break;
    default:
        protocol->tok_state = 0;
    }

    return ret;
}