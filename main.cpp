#include <cstdio>
#include <cstring>
#include <cctype>
#include <iostream>

//define a vector for integers
#define T int
#define Vector int_vector
#include "vector.h"

// undefine T and Vector so they can be reused
#undef T
#undef Vector

//define a vector for characters
#define T char
#define Vector char_vector
#include "vector.h"

#undef T
#undef Vector

#define MAX_LINE 300  // we assume that all input lines are 300 characters or less in length

int debugMode = false;

// Enumerated Type specifying all of the Tokens
enum TokenType{
    ERROR, OPERATOR, VALUE, EOLN, QUIT, HELP, EOFILE
};

void printCommands() {
    printf ("The commands for this program are:\n\n");
    printf ("q - to quit the program\n");
    printf ("? - to list the accepted commands\n");
    printf ("or any infix mathematical expression using operators of (), *, /, +, -\n");
}

// Class to hold the Token information
class Token {
private:
    TokenType type;
    int       val;      // using -999 as undefined/error
    char     op;       // using '$' as undefined/error

public:
    using op_t = char;  //operator type
    using value_t = int;
    constexpr static const int valueError = -999;
    constexpr static const op_t opError = '$';

    // Parameterized constructor with default values
    constexpr Token(TokenType t, op_t op, value_t val) : type(t), op(op), val(val) {}

    explicit constexpr Token(TokenType t): Token(t, opError, valueError) {}

    explicit constexpr Token(char op): Token(OPERATOR, op, valueError) {}

    explicit constexpr Token(value_t val): Token(VALUE, opError, val) {}

    // return true if the Current Token is of the given TokenType
    constexpr bool equalsType(TokenType t) { return type == t; }

    constexpr TokenType getType() { return type; }

    // return true if the Current Token is of the OPERATOR TokenType
    // and contains the given operator character
    [[maybe_unused]] constexpr bool equalsOperator(char c) { return type == OPERATOR && op == c; }

    // Return the Operator for the current Token
    // verify the current Token is of the OPERATOR TokenType
    constexpr op_t getOperator() {
        return (type == OPERATOR? op : '$'); // using $ to indicate an error value
    }

    // Return the Value for the current Token
    // verify the current token is of the value TokenType
    constexpr value_t getValue() {
        return (type == VALUE? val : -999); // using -999 to indicate an error value
    }
};


class TokenReader {
    char inputLine[MAX_LINE]{};
    bool needLine;
    int pos;
    int length;

public:

    // initialize the TokenReader class to read from Standard Input
    TokenReader() : pos(0), needLine(true), length(0) {
        // set to read from Standard Input
        inputLine[0] = '\0';
    }

    // Force the next getNextToken to read in a line of input
    void clearToEoln() { needLine = true; }

    // Return the next Token from the input line
    Token getNextToken() {
        // get a new line of input from user
        if (needLine) {
            if (fgets (inputLine, MAX_LINE, stdin) == nullptr ) {
                printf ("Error in reading");
                Token t(EOFILE);
                return t;
            }
            length = strlen(inputLine);
            needLine = false;
            pos = 0;
        }

        // skip over any white space characters in the beginning of the input
        while (pos < length && isspace(inputLine[pos])) ++pos;

        // check for the end of the current line of input
        if (pos >= length) { // at end of line
            needLine = true;
            Token t(EOLN);
            return t;
        }

        // Get the next character from the input line
        char ch = inputLine[pos]; pos++;

        switch(ch) {
            // check if 'q' or 'Q' was entered ==> QUIT Token
            case 'q': case 'Q': return Token (QUIT);

            // check if "?" was entered ==> HELP Token
            case '?': return Token (HELP);

            // check for Operator values
            case '+': case '-': case '*': case '/': case '(': case ')': return Token(ch);

            default: break;
        }

        // check for a number  ==> VALUE Token
        if (isdigit(ch)) {
            int number = ch - '0';  // subtract ascii value of ch from ascii value of '0'
            ch = inputLine[pos]; pos++;
            while (isdigit(ch)) {
                number = number * 10 + (ch - '0');
                ch = inputLine[pos++];
            }
            pos--; // since number calculation check one character after the last digit
            return Token(number);
        }
        // Input in not valid if code get to this part ==> ERROR Token
        char* s = inputLine + (pos - 1);
        while (pos < length && !isspace(inputLine[pos])) ++pos;
        inputLine[pos] = '\0';

        //print error message.
        printf("Error: Unrecognized symbol \"%s\"\n", s);
        return Token (ERROR);
    }

};

Token::value_t eval(Token::value_t rhs, Token::value_t lhs, Token::op_t op) {
    switch (op) {
        case '+':
            return lhs + rhs;
        case '-':
            return lhs - rhs;
        case '*':
            return lhs * rhs;
        case '/':
            return lhs / rhs;
        default:
            throw std::invalid_argument("Unknown operator");
    }
}

inline void popAndEval(int_vector& valuesStack, char_vector& operatorStack) {
    int rhs = valuesStack.back();
    valuesStack.pop_back();
    int lhs = valuesStack.back();
    valuesStack.pop_back();
    char op = operatorStack.back();
    operatorStack.pop_back();
    valuesStack.push_back(eval(rhs, lhs, op));
}

void processExpression (Token inputToken, TokenReader& tr);

int main(int argc, char *argv[]){
    //check for debugMode
    if (argc > 1) {
        for (int i = 0; i < argc; ++i) {
            if (strcmp(argv[i], "-d") == 0) {
                debugMode = true;
                puts("Debugging mode ON.");
                break;
            }
        }
    }

    printf ("Starting Expression Evaluation Program\n");

    while(true) {
        printf ("\nEnter Expression: ");

        TokenReader tr;
        auto inputToken = tr.getNextToken ();

        switch (inputToken.getType()) {
            case QUIT:
                printf ("Quitting Program\n");
                return 1;
            case HELP:
                printCommands();
                tr.clearToEoln(); break;
            case ERROR:
                printf ("Invalid Input - For a list of valid commands, type ?\n");
                tr.clearToEoln();  break;
            case EOLN:
                printf ("Blank Line - Do Nothing\n");  break;
            default:
                processExpression(inputToken, tr);
                break;
        }
    }
}

#define LOG(format, ...) if (debugMode) printf(format, __VA_ARGS__)

void processExpression (Token inputToken, TokenReader& tr) {
    int_vector valueStack;
    char_vector operatorStack;

#define tryPopEval()                                \
    try {                                           \
        popAndEval(valueStack, operatorStack);      \
    } catch(...) {                                  \
        printf("Error: Too many operators.\n");     \
        return;                                     \
    }

    ///// Loop until the expression reaches its End /////
    while (!inputToken.equalsType(EOLN)) {
        switch(inputToken.getType()) {
            case VALUE: {
                auto val = inputToken.getValue();
                LOG("Val: %d, ", val);
                valueStack.push_back(val);
            }break;

            case OPERATOR: {
                auto op = inputToken.getOperator();
                LOG("OP: %c, ", op);

                switch (op) {
                    case '(':
                        operatorStack.push_back(op); break;
                    case '+': case '-':
                        loop1: if (!operatorStack.empty()) {
                            switch (operatorStack.back()) {
                                case '+': case '-': case '*': case '/':
                                    tryPopEval()
                                    goto loop1;
                                default: break;
                            }
                        }
                        operatorStack.push_back(op); break;
                    case '*': case '/':
                        loop2: if (!operatorStack.empty()) {
                            switch (operatorStack.back()) {
                                case '*': case '/':
                                    tryPopEval()
                                    goto loop2;
                                default: break;
                            }
                        }
                        operatorStack.push_back(op); break;
                    case ')':
                        while (!operatorStack.empty() && operatorStack.back() != '(')
                            tryPopEval()
                        if (operatorStack.empty()) {
                            printf("Error: missing an opening parenthesis\n");
                            return;
                        }
                        else
                            operatorStack.pop_back();
                    default: break;
                }
            }break;
            default: return;
        }
        inputToken = tr.getNextToken(); // get next token from input
    }

    // The expression has reached its end
    while (!operatorStack.empty()) { tryPopEval() }

    auto result = valueStack.back();
    valueStack.pop_back();

    valueStack.empty()? printf("Result: %d\n", result) : printf("Error: Not enough operators.\n");

#undef tryPopEval
}

#undef LOG
