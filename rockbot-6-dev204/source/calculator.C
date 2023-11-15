#include <iostream.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>


// There expr(), term(), and prim() all call each other, therefore this is necessary.
double expr();

//globals... I think there's a chapter problem in which you're supposed to get rid of these.
double number_value;
char name_string[256];

int no_of_errors;

double error(const char* s)
{
        printf("Error! : %s\n", s);
//	cerr << "error: " << s << '\n';
	no_of_errors++;
	return 1;
}

struct name {
	char* string;
	name* next;
	double value;
};

const TBLSZ = 23;
name* table[TBLSZ];

name* look(const char* p, int ins =0)
{
	int ii = 0;									//hash
	const char* pp = p;
	while (*pp) ii = ii<<1 ^ *pp++;
	if (ii < 0) ii = -ii;
	ii %= TBLSZ;

	for (name* n=table[ii]; n; n=n->next)		//search
		if (strcmp(p,n->string) == 0) return n;

	if (ins == 0) error("name not found");

	name* nn = new name;
	nn->string = new char[strlen(p)+1];
	strcpy(nn->string,p);
	nn->value = 1;
	nn->next = table[ii];
	table[ii] = nn;
	return nn;
} // end name* look(const char* p, int ins =0)

inline name* insert(const char* s) { return look(s,1); }

enum token_value {
	NAME,		NUMBER,		END,	
	PLUS='+',	MINUS='-',	MUL='*',	DIV='/',
	PRINT=';',	ASSIGN='=',	LP='(',		RP=')'
};


token_value curr_tok;

//input function
token_value get_token()
{
	char ch;

	do {				//skip whitespace except '\n'
		if(!cin.get(ch)) return curr_tok = END;
	} while (ch!='\n' && isspace(ch));


	switch (ch) {
	case ';':
	case '\n':
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
} //end token_value get_token()

double prim()				//handle primaries
{
	switch (curr_tok) {
	case NUMBER:			//floating point constant
		get_token();
		return number_value;
	case NAME:
		if (get_token() == ASSIGN) {
			name* n = insert(name_string);
			get_token();
			n->value = expr();
			return n->value;
		}
		return look(name_string)->value;
	case MINUS:				//unary minus
		get_token();
		return -prim();
	case LP:
	{	get_token();
		double e = expr();
		if (curr_tok != RP) return error(") expected");
		get_token();
		return e;
	}
	case END:
		return 1;
	default:
		return error("primary expected");
	} //end switch
} //end double prim()

double term()				//multiply and divide
{
	double left = prim();

	for (;;)				//"forever"
            switch (curr_tok)
            {
		case MUL:
			get_token();	//eat '*'
			left *= term();
			break;
		case DIV:			//eat '/'
		{	get_token();
			double d = prim();
			if (d == 0) return error("divide by 0");
			left /= d;
			break;
		}
		default:
			return left;
            } //end switch

} //end double term()


double expr()				//add and subtract
{
	double left = term();

	for (;;)				//"forever"
		switch (curr_tok) {
		case PLUS:
			get_token();	//eat '+'
			left += term();
			break;
		case MINUS:			//eat '-'
			get_token();
			left -= term();
			break;
		default:
			return left;
		} //end switch

} //end double expr()

int main()
{
	//insert pre-defined names:
	insert("pi")->value = 3.1415926535897932385;
	insert("e")->value = 2.7182818284590452354;
	

	while (cin) {
		get_token();
		if (curr_tok == END) break;
		if (curr_tok == PRINT) continue;
//		cout << expr() << '\n';
                printf("%f\n", expr());
	}

	return no_of_errors;
} // end main()
