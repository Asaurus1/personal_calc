#ifndef CALC_H
#define CALC_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "CVarDB.h"
#include "CVariable.h"
#include "CMatrix.h"

#define SUCCESS 1
#define FAILURE 0

using namespace std;

enum OP {ASN, ADD, SUB, MULT, DIV, EXP, MOD, INC, DEC, ASNADD, ASNSUB, ASNMULT, ASNDIV, NULLOP};

enum PARTTYPE {DOUBLE,WORD,OPERATOR,MATRIX,BRACKET,END};

typedef string::iterator strItr;

typedef struct part
{
    PARTTYPE type;
    //Store where this part beings and ends in INPUT
    strItr st;
    strItr ed;
    union {
        double ndata;
        char* wdata;
        OP odata;
        CMatrix* mdata;
        short bdata; //Bracket data
        };
    int opLevel; //For handling order of operations at a later date.

    //type and bounds constructor
    part(PARTTYPE t, strItr s, strItr e)
    {
        type = t;
        st = s;
        ed = e;
        opLevel = 0;
        wdata = 0; //Set the union to a default value of zero.
    }

    //Destructor for killing our matrix or word.
    ~part()
    {
        switch (type)
        {
        case WORD:
            if (wdata != 0) //if memory has been allocated
                delete [] wdata;
            break;
        case MATRIX:
            if (mdata != 0) //if memory has been allocated
                delete mdata;
            break;
        default:
            break;
        };
    }
} part;

/*********************************
        Calculator Class
*********************************/

class Calc
{
    typedef vector<part>::iterator prtItr;
    vector<part>    m_Expr;
    string          Input;
    istream*        Source;
    CVarDB*         m_db;
    CVariable*      m_ans;
    bool            quitNext;

    //sub-routines that I will use.
    bool createDB();            //Creates a variable database
    void enumerateVars();
    bool ReadInput();           //Reads input from Source into Input
    bool CommandCheck();        //Checks for special commands such as who and quit.
    bool Partition();           //Partitions the Input string and fills m_Expr;
    bool Convert();             //Converts the character references in m_Expr to actual values and operators and matrices.
    bool Interpret();           //Interpret the expression and call the calculator functions to find its value.
    void Echo(CVariable*);

    //Various calculator functions
    CMatrix  CalcExpr(prtItr& st, prtItr& ed, int opLevel = 0); //Calculates the entire expression between the two places in the vector string and returns the value.
    CMatrix CalcOP(const CMatrix& a,const OP& op,const CMatrix& b);
    CMatrix CalcOP(const CMatrix& a, const OP& op);
    bool    isAssign(const part& p);
    OP      AssignOpToOp(OP op);
    OP      EncodeOP(const strItr& chr);
    prtItr  FindNextOp(prtItr st,prtItr ed);
    int     GetOpPrec(OP op);

    //Partitioner functions
    bool isChar(char);
    bool isOp(char);
    bool isDigit(char);
    bool isParen(char);

    //Error handling
    string  lastErr;
    bool    isErr;
    void    printError();

public:
    Calc() : Source(&cin), isErr{false} {
        if (!createDB())
            cout << "Unable to allocate variable database.";
    };

    Calc(istream& in) : Source(&in), isErr{false}  {
        if (!createDB())
            cout << "Unable to allocate variable database.";
    };

    ~Calc(){
        delete m_db;
    }

    //Run the calculator.
    void run();

    //Allows the program to redefine the source, if I ever figure out how to make new streams which are not temporary.
    void setSource( istream& in) { Source = &in; };
};

#endif //CALC_H
