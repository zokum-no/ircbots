/* Info bot type functions.. mostly taken from other info type bots and modified. */

#include <stdlib.h>
#include <ctype.h>
#include "config.h"
#include "send.h"
#include "misc.h"

void c_math(char *from, char *target, char *math)
{
        char            input[556];
        char            number_string[30];
        char            op = 0;

        unsigned int    index = 0;
        unsigned int    to = 0;
        unsigned int    input_length = 0;
        unsigned int    number_length = 0;
        double          result = 0.0;
        double          number = 0.0;

        strcpy(input, math);
        input_length = strlen(input);

        for (to = 0, index = 0; index <= input_length; index++)
                if (*(input + index) != ' ')
                        *(input + to++) = *(input + index);
        input_length = strlen(input);
        index = 0;
        if (input[index] == '=')
                index++;
        else {
                number_length = 0;
                if (input[index] == '+' || input[index] == '-')
                        *(number_string + number_length++) = *(input + index++);
                for (; isdigit(*(input + index)); index++)
                        *(number_string + number_length++) = *(input + index);
                if (*(input + index) == '.') {
                        *(number_string + number_length++) = *(input + index++);
                        for (; isdigit(*(input + index)); index++)
                                *(number_string + number_length++) = *(input + index);
                }
                *(number_string + number_length) = '\0';
                if (number_length > 0)
                        result = atof(number_string);
        }
        for (; index < input_length;) {
                op = *(input + index++);
                number_length = 0;
                if (input[index] == '+' || input[index] == '-')
                        *(number_string + number_length++) = *(input + index++);
                for (; isdigit(*(input + index)); index++)
                        *(number_string + number_length++) = *(input + index);
                if (*(input + index) == '.') {
                        *(number_string + number_length++) = *(input + index++);
                        for (; isdigit(*(input + index)); index++)
                                *(number_string + number_length++) = *(input + index);
                }
                *(number_string + number_length) = '\0';
                number = atof(number_string);
                switch (op) {
                case '+':
                        result += number;
                        break;
                case '-':
                        result -= number;
                        break;
                case '*':
                        result *= number;
                        break;
                case '/':
                        if (number == 0) {
                                sendprivmsg(target, "%s: Division by zero error!", getnick(from));
                                return;
                        } else
                                result /= number;
                        break;
                case '%':
                        if ((long) number == 0) {
                               sendprivmsg(target, "%s: Division by zero error!", getnick(from));
                                return;
                        } else
                                result = (double) ((long) result % (long) number);
                        break;
                default:
                        sendprivmsg(target, "%s: Illegal operation!", getnick(from));
                        return;
                }
        }
        sendprivmsg(target, "%s\002:\002 %f", getnick(from), result);
}
#ifdef notdef
struct calcname (
       char* string;
       calcname* next;
       double value;
};

const TBLSZ = 23;
calcname* table[TBLSZ];

calcname*  calclook(const char* p, int ins = 0)
{
   int ii = 0;
   const char* pp = p;
   while (*pp) ii = ii<<1 ^ *pp++;
   if (ii < 0) ii = -ii;
   ii %= TBLSZ;
   for (calcname* n=table[ii]; n; n=n->next)           //search
           if (strcmp(p,n->string) == 0) return n;
   if (ins == 0) error("name not found");
   calcname* nn = malloc(sizeof(*calcname));
   nn->string = malloc(strlen(p)+1);
   strcpy(nn->string,p);
   nn->value = 1;
   nn->next = table[ii];
   table[ii] = nn;
   return nn;
}

inline calcname* insert(const char* s) { return calclook(s,1); } 

enum token_value {
        NAME,           NUMBER,         END,
        PLUS='+',       MINUS='-',      MUL='*',        DIV='/',
        PRINT=';',      ASSIGN='=',     LP='(',         RP=')'
}; 

token_value get_token()
{
        char ch;

        do {                            //skip whitespace except '\n'
                if(!cin.get(ch)) return curr_tok = END;
        } while (ch!='\n' && isspace(ch));


        switch (ch) {
        case ';':
        case '\n':
        case '\0':
                return curr_tok=PRINT;
        case '*':
        case '/':
        case '+':
        case '-':
        case '(':
        case ')':
        case '=': 
                return curr_tok=token_value(ch);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case '.':
                cin.putback(ch);
                cin >> number_value;
                return curr_tok=NUMBER;
        default:
                if (isalpha(ch)) {
                        char* p = name_string;
                        *p++ = ch;
                        while (cin.get(ch) && isalnum(ch)) *p++ = ch;
                        cin.putback(ch);
                        *p = 0;
                        return curr_tok=NAME;
                }
                error("bad token");
                return curr_tok=PRINT;
        } //end switch
}
void c_math2(char *from, char to)
{


}

#endif
