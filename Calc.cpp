#include "Calc.h"
#include <math.h>
#include <iomanip>

#define OPLEVELRANGE 3 //How many op levels there are in the basic operators we have. Moving into a parenthesized expression increases the opLevel by at least this much

/****************** Calculator Definition *******************

This file defines all the functions used directly by the Calc object. It consists of 5 main functions:

    - Run           :: The main loop of the Calculator. This calls all the other functions below and prompts the user.
    - Partitioner   :: Separates the input string into "parts" which are stored in a private vector of part objects.
    - Converter     :: Reads through the part vector and converts the data stored in the string for each part to the
                       appropriate form (double, operator, matrix, string, etc.).
    - Interpreter   :: Interprets the parts in the part vector and calls the calculator on the appropriate sections,
                       and searches the variable database for the right variable to store the result in. The Interpreter
                       also sets OpLevels (levels of recursion) on each of the parts, allowing the calculator to perform
                       proper order of operations following the rules of mathematics.
    - Calculator    :: A recursive calculator which loops through each "calculation" level (OpLevel) of the parts vector
                       and accumulates a result, which it returns to the Interpreter.

Several smaller functions which act as aids and building blocks are also defined, most of them at the end of the file.*/

using namespace std;

//A string copy function that allows concatenation of a substring with either a string or a character array.
void substr_cpy(string&, strItr, strItr);
void substr_cpy(char*, strItr, strItr);

/**************** Run *****************

run() function should be called by the programmer to start the calculator. All looping and error handling is done by
this function. Once execution ends (the user types "quit), control will be returned to the calling function.

*/
void Calc::run()
{
    int num_case = 0; // For numbering the prompts
    quitNext = false; // Toggles whether we quit the next time around or not.

    while (!quitNext)
    {
        ++num_case; // Increment the prompt counter

        // Reset the part vector and the error members
        m_Expr.clear();
        isErr = false;

        cout << "#" << num_case << " Input an expression to calculate: ";
        if (!ReadInput())
        {
            // Check whether we have reached an end of a file
            if (Source != &cin) // If we're not reading from user input
            {
                if (Source->eof()) // If we reached the end of the file on the last read
                {
                    //Set the source to cin
                    cout << "Reached EOF" << endl << endl;
                    setSource(cin);
                    continue;
                }
            }
            else
            {
                // There was a reading error, so stop execution.
                cout << endl << endl << "\tSorry, I cannot read the input." << endl;
                break;
            }
        }

        // We only echo input to the command line if we are not reading from cin.
        if (*Source != cin)
        {
            //string echo = Input;
            cout << Input << endl;
        }

        // Add a line break after input
        cout << endl;

        // Call the Partitioner
        Partition();
        if (isErr)
        {
            printError();
            cout << "\tPartition Error" << endl << endl;
            continue;
        }

        // Call the Converter
        Convert();
        if (isErr)
        {
            printError();
            cout << "\tConvert Error" << endl << endl;
            continue;
        }

        // If the user inputs a special command, this will catch and execute it.
        if (CommandCheck())
            continue;

        // Call the Interpreter
        Interpret();
        if (isErr)
        {
            printError();
            cout << "\tInterpret Error" << endl << endl;
            continue;
        }

        //Rinse and Repeat

    } //End while
}

// Reads input from Source into Input
bool Calc::ReadInput()
{
    //Get a line from my source and store it in Input
    return getline(*Source,Input, '\n');
}

// List all the variables in the database. Called when the user types "who".
void Calc::enumerateVars()
{
    for (int i = 0; i < m_db->size(); ++i)
    {
        cout << left << "\t" << setw(4) << m_db->at(i)->Name() << " =  ";
        PrintMatrix(m_db->at(i)->Value(), cout, "\t\t ");
        cout << endl << endl;
    }
}

// Checks whether the user has typed a special command. Commands will consist of 1 or 2 word parts.
// the first being the name of the command and the second being an optional argument (not actually needed now)
bool Calc::CommandCheck()
{
    prtItr command = m_Expr.begin();
    short  ExprLen = m_Expr.size();
    string cmdstr, args;

    if (ExprLen <= 2 && command->type == WORD) // Is this a single-word command?
    {
        cmdstr = command->wdata;
        if (cmdstr == "who")
            enumerateVars();
        else if (cmdstr == "quit")
        {
            cout << "\tGoodbye!" << endl;
            quitNext = true;
        }
        else if (ExprLen == 2 && (command+1)->type == WORD) //Is this a double-word command.
        {
            args = (command+1)->wdata;
            if (cmdstr == "open")
            {
                //open a file (not implemented)
                //Source = new ifstream(args);
            }
            else
                return false;
        }
        else if (cmdstr == "open the doors") // special case of open
            cout << "\tI'm sorry, Dave. I can't do that. \n\n";
        else return false;
    }
    else
        return false;

    return true;
}

/*********** Partitioner *************

The Partitioner separates the Input string into segments based on word/number/matrix/operator boundaries and fills the vector
m_Expr with part objects. The partitioner does not convert strings to doubles or operators, it simply stores start and end
iterators in each part object so that the converter can go back later and convert things.

Accepted signatures:
    - Double   (any digit 0-9 and the decimal point "."; truncated at first non-digit.)
    - Word     (any character A-Z and a-z as well as "_" and any digits not in the first position.)
    - Operator (any single operator +, -, *, /, ^, %, =, or the double operators +=, -=, *=, /=, ++, --.)
    - Paren    (any close or open parenthesis.)
    - Matrix   (any sequence between two square brackets [ and ]; partitioner does not check the validity of the matrix, but it does check for invalid characters. )
*/
bool Calc::Partition()
{
    strItr curChr   = Input.begin();    // Tracks the current position
    strItr startChr = curChr;           // Tracks the start position of the current part we're reading
    PARTTYPE curType;                   // The current part type to add to the vector
    strItr endChr   = Input.end();      // The end position of the input string

    strItr startErr = curChr;           // Tracks the beginning of a lexical segment which is used for nice error reporting

    // Loop through the whole string
    while(curChr < endChr)
    {
        startChr = curChr; // Update the start tracker to the current position, which is always at the beginning of a part.

        // Check for spaces between parts and skip over them.
        if (*curChr == ' ')
        {
            ++curChr;           // Move to the next spot
            startErr = curChr;  // Move the error tracker up as well.
            continue;           // Start over
        }

        // Look for a number
        if (isDigit(*curChr))
        {
            curType = DOUBLE;
            // Continue through the string until we reach a non-digit or the end.
            while (curChr < endChr)
            {
                if (isDigit(*curChr))
                    ++curChr; //Move forward
                else
                    break; //Continue in the main loop. We've reached the edge of this number.
            }
        }
        // Look for a word
        else if (isChar(*curChr))
        {
            curType = WORD;
            // Continue through the string until we reach a non-word character or the end.
            while (curChr < endChr)
            {
                if ((isChar(*curChr) || isDigit(*curChr)) && *curChr != '.')
                    ++curChr;
                else
                    break;
            }
        }
        //Look for an operator
        else if (isOp(*curChr))
        {
            curType = OPERATOR;

            // Stop if we've reached the end, there are no more characters.
            if (curChr+1 == endChr)
                break;

            strItr nextChr = curChr + 1;

            switch (*curChr)
            {
            case '+':
                if (*nextChr == '+' || *nextChr == '=' ) // If this is a ++ or a +=
                    ++curChr;
                break;
            case '-':
                if (*nextChr == '-' || *nextChr == '=' ) // If this is a ++ or a +=)
                    ++curChr;
                break;
            case '*':
            case '/':
                if (*nextChr == '=') // If this is a ++ or a +=)
                    ++curChr;
                break;
            }

            // Whether a double op or not, we need to move to the next element in the string:
            ++curChr;
        }
        // Look for a parenthesis
        else if (isParen(*curChr))
        {
            curType = BRACKET;
            curChr++;
        }
        // Look for a matrix
        else if (*curChr == '[')
        {
            curType = MATRIX;
            ++curChr; // Move inside the bracket.

            // Loop through the parts inside the matrix
            while (curChr < endChr)
            {
                // Check for the end of the matrix
                if (*curChr == ']')
                {
                    ++curChr; // Move outside the matrix
                    break;
                }

                // Check for spaces.
                if (*curChr == ' ' || *curChr == '\t')
                {
                    ++curChr;

                    // Skip remaining whitespace
                    while (*curChr == ' ' || *curChr == '\t')
                        ++curChr;

                    continue; // Restart the main matrix loop.
                }

                // Check whether this is a digit element or a semicolon or a comma (we accept spaces, but the above 'if' throws extras out.
                if (isDigit(*curChr) || *curChr == ';' || *curChr == ',')
                {
                    ++curChr;
                }
                else if (curChr >= endChr) // If we reach the end of the line prematurely
                {
                    // Create the error message and return a fail.
                    isErr = true;
                    lastErr = "Unexpected End-of-expression in matrix.";
                    substr_cpy(lastErr, startErr, curChr+1); //Append descriptor

                    return FAILURE;
                }
                else
                {
                    // Create the error message and return a fail.
                    isErr = true;
                    lastErr = "Invalid matrix element: ";
                    substr_cpy(lastErr, startErr, curChr+1); //Append descriptor

                    return FAILURE; //FAIL! AN ERROR!
                }
            } // End of matrix loop
        } // End of matrix test
        // If the character is invalid in the whole expression
        else
        {
            //Create the error message and return a fail.
            isErr = true;
            lastErr = "Invalid syntax or character: ";
            lastErr += *curChr;

            return FAILURE; //FAIL! OH NO!
        }

        // Now curChr is at the end of the current part, and startChr is at the beginning.
        // Add the part to the expression vector
        m_Expr.push_back(part{curType, startChr, curChr});

    } //End of main while
    return SUCCESS;
}


/*********** Converter *************

The Converter takes the part objects in m_Expr and looks at the portion of the input string between the start and end
iterators for each part. Based on the type of the part, it converts the data in the string to a valid computer
representation. It then adds this data to the data union in the part object.

- For string->double conversion, I loop through the digits and add on each successive digit to 10 times the sum of the previous digits.
  When I reach a decimal point, I begin counting until the end of the loop. Then I divide the entire sum by 10^(decimal_count) to get
  final double.

- Strings are copied directly as character arrays into a dynamic array whose pointer is given to the part object.

- Parentheses are converted into a signed value (+- OPLEVELRANGE) which is later used in opLevel calculations to ensure that recursion levels don't overlap.

- Matrices are created by calling their string constructor on a temporary character array which has had the matrix string from Input copied into it.

- Operators are encoded via the EncodeOp() function.

*/
bool Calc::Convert()
{
    prtItr e_st = m_Expr.begin();
    prtItr e_ed = m_Expr.end();

    // First go through and convert all the data to the appropriate forms.
    for(; e_st < e_ed; ++e_st)
    {
        switch(e_st->type)
        {
        case DOUBLE:
        {
            // Convert the data to a double
            int decpwr = 0;
            strItr chr = e_st->st;

            // Set ndata to zero
            e_st->ndata = 0.0;

            // Here we read in the string characters and convert them to a double.
            do
            {
                //If we have reached or passed the decimal point, keep counting up.
                if (*chr =='.')
                {
                    decpwr = 1;
                    chr++;
                    continue;
                }
                if (decpwr > 0.0)
                    decpwr *= 10;

                // Multiply whatever is in the element by 10 and add in the current value as a ones place.
                e_st->ndata *= 10;
                e_st->ndata  += *chr - '0';
                // Move forward in the string
                ++chr;
            } while(chr < e_st->ed );

            // Divide by the proper power.
            if (decpwr > 0.0)
                e_st->ndata  /= decpwr;
        break; } //End of DOUBLE
        case WORD: {
            // Allocate new memory for storing a copy of just this word
            size_t strlen = std::distance(e_st->st, e_st->ed);
            e_st->wdata = new char [strlen+1]; //Make a new wordstring
            substr_cpy(e_st->wdata, e_st->st, e_st->ed); //Copy the characters from Input
            e_st->wdata[strlen] = 0; //Append null char

            break; }
        case OPERATOR: {
            // Set the op type to be right
            e_st->odata = EncodeOP(e_st->st);
            break; }
        case MATRIX: {
            // Create a new matrix object
            size_t strlen = std::distance(e_st->st, e_st->ed);
            char* tmpstr = new char [strlen+1]; // Make a new temporary string to hold the data to go into the matrix.
            substr_cpy(tmpstr, e_st->st, e_st->ed); // Copy the characters from Input
            tmpstr[strlen] = 0; // Append null char

            e_st->mdata = new CMatrix{tmpstr}; // Create a new matrix.
            break; }
        case BRACKET:
                // Set to +OPLEVELRANGE if left bracket, -OPLEVELRANGE if right bracket.
                e_st->bdata = OPLEVELRANGE;
                if (*e_st->st == ')') {e_st->bdata *= -1;}
                break;
        case END:
            break;
        } // End of switch
    } // End of for
    return SUCCESS;
}

// Defines a macro which allows cleaner access to parts at an offset of (a) from the part pointed to by e_st.
#define PRTOFST(a) (*(e_st+a))

/*********** Interpreter *************

The Interpreter reads the parts in m_Expr and figures out whether an increment, assignment, and/or calculations
is being expressed. It then performs the appropriate actions.

- If there is an increment or decrement operator and the expression is exactly two parts long,
  the interpreter searches the variable database for a variable to increment or decrement. If
  it cannot find one, it returns an error, otherwise it performs the operation.

- If there is no equals sign


Accepted signatures:
    - Double   (any digit 0-9 and the decimal point "."; truncated at first non-digit.)
    - Word     (any character A-Z and a-z as well as "_" and any digits not in the first position.)
    - Operator (any single operator +, -, *, /, ^, %, =, or the double operators +=, -=, *=, /=, ++, --.)
    - Paren    (any close or open parenthesis.)
    - Matrix   (any sequence between two square brackets [ and ]; partitioner does not check the validity of the matrix, but it does check for invalid characters. )
*/
bool Calc::Interpret()
{
    prtItr e_st = m_Expr.begin();
    prtItr e_ed = m_Expr.end();
    size_t ExprLen = e_ed - e_st;
    CVariable* asnTo;
    CMatrix calcValue{0.0}; //Holds temporary calculation variable so we can check for errors.

    //Find the next operator for later on.
    OP nxtop = FindNextOp(e_st,e_ed)->odata;

    // Check whether this is a basic increment or decrement.
    if ((nxtop == INC || nxtop == DEC ) && ExprLen == 2)
    {
        // We only handle increments or decrement in 2-part expressions: OP + WORD or WORD + OP.
        if (PRTOFST(0).type == WORD)
            asnTo = m_db->search(PRTOFST(0).wdata);
        else if (PRTOFST(1).type == WORD)
            asnTo = m_db->search(PRTOFST(1).wdata);
        else
        {
            isErr = true;
            lastErr = "Expected variable expression. Cannot ";
            lastErr += (nxtop == INC) ? "increment" : "decrement";
            lastErr += " numbers or matrices.";
            return FAILURE;
        }

        // Check whether the variable existed
        if (asnTo == 0)
        {
            isErr = true;
            lastErr = "Could not find varaible for ";
            lastErr += (nxtop == INC) ? "increment" : "decrement";
            lastErr += ". Please use an existing variable.";
            return FAILURE;
        }

        // Calculate using the value of nxt
        *asnTo = CalcOP(asnTo->Value(),nxtop);

    }
    else
    {
        // The first element can't be an operator.
        if (e_st->type == OPERATOR)
        {
            isErr = true;
            lastErr = "Invalid Syntax. Must have non-operator element in first position.";
            return FAILURE;
        }
        // The last element can't be an operator.
        if ((e_ed-1)->type == OPERATOR)
        {
            isErr = true;
            lastErr = "Invalid Syntax. Must have non-operator element in last position.";
            return FAILURE;
        }

        // Calculate opLevels -- This is used for correctly recursing through the mathematical order of operations.
        int baseOp = 0; // base opLevel, so we can add opLevels for bracketing.
        bool firstelem = true; // Retrieving the opLevel of the previous element should not happen the first time.

        // Loop through the part array
        for (; e_st < e_ed; ++e_st, firstelem = false)
        {
            // If this is an operator, it has its own opLevel
            if (e_st->type == OPERATOR)
            {
                int myOpLevel = baseOp + GetOpPrec(e_st->odata);
                // Give me my own opLevel
                e_st->opLevel = myOpLevel;

                // The elements on either side of me should be numbers. If they are not operators then give them the maximum of my opLevel and their opLevel.
                if (PRTOFST(-1).type != OPERATOR && PRTOFST(-1).opLevel < myOpLevel)
                    PRTOFST(-1).opLevel = myOpLevel;
            }
            // If the current part is a bracket, we need to step into or out of another level of recursion.
            else if (e_st->type == BRACKET)
            {
                // Increment or decrement baseOp by OPLEVELRANGE
                baseOp += e_st->bdata;
                if (baseOp < 0)
                {
                    isErr = true;
                    lastErr = "Unmatched parentheses. Cannot parse.";
                    return FAILURE;
                }
                e_st->opLevel = baseOp;

            }
            // If the part is something else and it's not the first element
            else if (!firstelem)
            {
                // Everything else always associates right, unless it associates left (because some operator sets it to do so).
                e_st->opLevel = PRTOFST(-1).opLevel;
            }
        }

        // When we finish, we should be back at the recursion level we started at.
        // If we're not, the user forgot a parenthesis somewhere.
        if (baseOp != 0)
        {
            isErr = true;
            lastErr = "Unmatched parentheses. Cannot parse.";
            return FAILURE;
        }

        //********** We could compress the op levels in the future here to reduce recursion times.************

        // Move the start iterator back to the beginning
        e_st = m_Expr.begin();

        // Check for just a word. If it is an existing variable, echo the value of that variable.
        if (PRTOFST(0).type == WORD && ExprLen == 1)
        {
            // Look for the variable in the database
            asnTo = m_db->search(PRTOFST(0).wdata);

            // If the Interpreter cannot find the variable, then return an error.
            if (asnTo == 0)
            {
                isErr = true;
                lastErr = "Unknown command or variable: ";
                substr_cpy(lastErr, PRTOFST(0).st, PRTOFST(0).ed);

                return FAILURE;
            }

        }
        // Check for an assignment to a variable explicitly.
        else if (ExprLen > 2 && isAssign(PRTOFST(1)))
        {
            // Are we trying to assign to something other than a word? This is an error.
            if (PRTOFST(0).type != WORD)
            {
                isErr = true;
                lastErr = "Cannot assign to non-variable type. That would break math.";
                return FAILURE;
            }

            // See if the variable is already in the database and if not, create it.
            asnTo = m_db->search(PRTOFST(0).wdata);
            if (asnTo == 0)
                asnTo = m_db->createVar(PRTOFST(0).wdata);

            // Get the type of assignment (=, +=, etc.)
            OP asnType = AssignOpToOp(PRTOFST(1).odata);

            // Is it just plain old assignment...
            if (asnType == ASN)
            {
                // Calculate whatever is beyond the equals sign.
                e_st += 2;
                calcValue = CalcExpr(e_st, e_ed);
                if (!isErr)
                    *asnTo = calcValue;
                else
                    return FAILURE;
            }
            // ...or a fancy one?
            else
            {
                // Call the calculator on the part of the input which is beyond the equals sign.
                e_st += 2;
                calcValue = CalcExpr(e_st, e_ed);

                //If there is no error in the calculation, then perform the fancy assignment.
                if (!isErr)
                    *asnTo = CalcOP(asnTo->Value(), asnType, calcValue);
                else
                    return FAILURE;
            }
        }
        // Handle non-assignments (i.e. assign to "ans")
        else
        {
            asnTo = m_db->getAns();

            // Call the calculator on the entire expression (there is no equals sign, it is implied).
            calcValue = CalcExpr(e_st,e_ed);

            // If there is no error, then actually assign the returned value.
            if (!isErr)
                *asnTo = calcValue;
            else
                return FAILURE;
        }
    }

    //Echo the variable onto the screen
    Echo(asnTo);

    return SUCCESS;
}

// Echos a variable to the terminal.
void Calc::Echo(CVariable* var)
{
    cout << '\t' << var->Name() << " = " << var->Value() << endl << endl;
}

/*********** Calculator *************

The Calculator steps through the parts in m_Expr between the two iterators passed to it and sequentially (left-to-right)
calculates the value of the expression as a double. In order to follow order of operations, it calls itself recursively.

    - When it reaches an element with a higher opLevel, it calls itself with st pointing to that element with one-higher opLevel
    - When it reaches an element with a lower  opLevel, it returns cumulativeValue to whatever called it, whether
      that be the Interpreter or another level of calculator recursion.
    - Elements of the same opLevel are processed in left-to-right order.

Note that after the Calculator runs, the iterator st is left pointing at the last element in m_Expr. If you need to preserve
the location of st, pass in an lvalue copy of the iterator.

*/
CMatrix Calc::CalcExpr(prtItr& st, prtItr& ed, int opLevel)
{
    CMatrix cumulativeValue{0.0};    //Holds the current value of the calculation for this level of recursion
    CMatrix nextValue{0.0};          //Holds the next value of the equation. Then we perform thisOp on cumulativeValue and nextValue
    prtItr thisOp = st;         //Default to start, although this is actually the location of the first integer.
    prtItr thisValue = st;        //Get the very first token, which should be an integer (although if it's not, we don't have error checking yet, so oops)
    bool firsttime = true;   //Set to true if we are at the top of the loop of this level of recursion. This allows us to 'add in' the first value in the expression to the cumulative value.;

    prtItr nextOp = FindNextOp(st,ed);
    bool exit = false;

    // Begin to loop through reading 2 (actually three) ahead at a time
    while (true)
    {
        // Skip any parentheses (we just use the precalculated opLevels.
        while (st->type == BRACKET && st != ed)
            ++st;

        // Exit when we get to the end.
        if (st == ed)
            break;

        // Read in the tokens needed for this loop.
        if (!firsttime)
        {
            // Get the token at start, which should be an operator, but we're going to check
            thisOp = st++;

            if (thisOp->type != OPERATOR)
            {
                isErr = true;
                lastErr = "Expected operator at ";
                lastErr += *(thisOp->st);
                return CMatrix{};
            }

            // Read the next token, which should be an double, matrix, or variable
            thisValue = st;
        }
        else
        {
            // Set thisValue to the first element in the expression, but don't move the start pointer forward just yet, since there's no operator.
            thisValue = st;
        }

        // Handle cases where we get an operator in an invalid spot.
        if (thisValue->type == OPERATOR)
        {
            isErr = true;
            lastErr = "Expected numerical value, variable, or matrix at ";
            lastErr += *(thisValue->st);
            return CMatrix{};
        }

        // Find the next operator, but don't move start forward
        nextOp = FindNextOp(st,ed);

        if (nextOp != ed && isAssign(*nextOp))
        {
            isErr = true;
            lastErr = "Cannot perform assignment within an expression.";
            return CMatrix{};
        }

        // Handle order of operations
        if (nextOp != ed && nextOp->opLevel > opLevel) // If the next operator has a higher opLevel:
        {
            // We need to recurse to find the proper nextValue to use. Enter recursion and print that we are doing so.
            nextValue = CalcExpr(st,ed,opLevel+1);
            // st now points to the next operator on our level after the subexpression we just consumed.

            //Find the next operator, so we can tell whether we need to exit.
            nextOp = FindNextOp(st,ed);

            //If the recursion triggered an error, we need to abort all levels of recursion.
            if (isErr)
            {
                return CMatrix{};
            }
        }
        else
        {
            // Get and print the value we need to calculate with next.
            switch (thisValue -> type)
            {
            // If this value is a number
            case DOUBLE:
                nextValue = thisValue->ndata;
                break;
            // If this value is a variable that we have to look up.
            case WORD: {
                CVariable* thisVar = m_db->search(thisValue->wdata);

                // Check whether this variable actually exists in the database.
                if (thisVar == NULL)
                {
                    isErr = true;
                    lastErr = "Unknown quantity \"";
                    substr_cpy(lastErr, thisValue->st, thisValue->ed);
                    lastErr += "\". Type \"who\" to list variables.";
                    return CMatrix{};
                }
                // Set nextValue to the value stored in the variable
                nextValue = thisVar->Value();
                break; }
            case MATRIX:
                nextValue = *(thisValue->mdata);
                break;
            default:
                isErr = true;
                lastErr = "Unexpected lexical element ";
                substr_cpy(lastErr, thisValue->st, thisValue->ed);
                return CMatrix{};
            }

            // Move start to the next position (we're hoping this is an operator, though for things like unitary operators this may not work in the future).
            ++st;
        }

        // If the next operator has a lower opLevel, then we need to break out of recursion
        if (st == ed || (nextOp != ed && nextOp->opLevel < opLevel))
        {
            // Exit the loop after this calculation and print that we are doing so.
            exit = true;
        }

        // Perform the calculation and store the cumulative result in cumulativeValue.
        if (firsttime)
        {
            // If this is the first time through, we just assign the value to cumulative value
            cumulativeValue = nextValue;
            firsttime = false;
        }
        else
        {
            // Otherwise we calculate it
            cumulativeValue = CalcOP(cumulativeValue,thisOp->odata,nextValue);
            if (cumulativeValue.IsNull())
            {
                isErr = true;
                lastErr = "Operation ";
                substr_cpy(lastErr, thisOp->st, thisOp->ed);
                lastErr += " returned null value. Check your operators.";
                return CMatrix{};
            }
        }

        // If we need to exit, break;
        if (exit)
            break;
    }
    return cumulativeValue;
}

//*** Various calculator functions ****

//Calculate a simple binary operator
CMatrix Calc::CalcOP(const CMatrix& a, const OP& op, const CMatrix& b)
{
    switch (op)
    {
    case ADD:
        return a + b;
    case SUB:
        return a - b;
    case MULT:
        return a * b;
    case DIV:

        if (b == 0)
        {
            isErr = true;
            lastErr = "Friends don't let friends divide by zero. Are you my friend?";
            return CMatrix{};
        }

        return a / b;
    case EXP:
        //Matrix powers not implemented yet
        if (! a.IsSingle() || !b.IsSingle())
        {
            isErr = true;
            lastErr = "Cannot use ^ with matrices.";
            return CMatrix{};
        }
        return pow(double(a.element(0,0)),double(b.element(0,0)));
    case MOD:
        if (a.IsSingle() && b.IsSingle())
            return int(a.element(0,0)) % int(b.element(0,0));
        else
        return CMatrix{};
    default:
        return CMatrix{}; //Not something we can compute
    }
}

//Calculate a simple unary operator
CMatrix Calc::CalcOP(const CMatrix& a, const OP& op)
{
    if (!a.IsSingle())
    {
        return CMatrix{};
    }

    switch (op)
    {
    case INC:
        return a+1;
    case DEC:
        return a-1;
    default:
        return CMatrix{}; // Invalid operator
    }
}

bool Calc::isAssign(const part& p)
{
    return (p.type == OPERATOR && (p.odata == ASN || p.odata == ASNADD || p.odata == ASNSUB || p.odata == ASNMULT || p.odata == ASNDIV));
}

//Operator encoding
OP Calc::EncodeOP(const strItr& chr)
{
    switch (*chr)
    {
    case '+':
        if (*(chr+1) == '+')
            return INC;
        else if (*(chr+1) == '=')
            return ASNADD;
        else
            return ADD;

    case '-':
        if (*(chr+1) == '-')
            return DEC;
         else if (*(chr+1) == '=')
            return ASNSUB;
         else
            return SUB;

    case '*':
        if (*(chr+1) == '=')
            return ASNMULT;
        else
            return MULT;

    case '/':
    case '\\':
        if (*(chr+1) == '=')
            return ASNDIV;
        else
            return DIV;

    case '^': return EXP;
    case '%': return MOD;

    case '=':
        return ASN;
    default:
        return NULLOP;
    }
}

//Convert an assignment+operator op into just the operator without the assignment.
OP Calc::AssignOpToOp(OP op)
{
    switch(op)
    {
    case ASNADD:
        return ADD;
    case ASNSUB:
        return SUB;
    case ASNMULT:
        return MULT;
    case ASNDIV:
        return DIV;
    default:
        return ASN;
    }
}

//Get the next Operator-type part in m_Expr after (and possibly including) st and before ed.
Calc::prtItr Calc::FindNextOp(prtItr st, prtItr ed)
{
    if (st >= ed) return ed; // check whether start and end are in the right order
    while (st != ed)
    {
        if (st->type == OPERATOR) return st;
        ++st;
    }
    return ed; //if no operator was found
}

//Get the opLevel associated with the given operator.
int Calc::GetOpPrec(OP op)
{
    switch (op)
    {
        case ADD:   return 0;
        case SUB:   return 0;
        case MULT:   return 1;
        case DIV:   return 1;
        case EXP:   return 2;
        case MOD:   return 1;
        case ASN:   return 0;
        default:    return 0;
    }
}

// ***** Various Partitioner functions ******

bool Calc::isChar(char var)
{
	// check if the input is a character or not ('a'~'z', 'A'~'Z')
	// Input : one character in data type 'char'
	// Output:
	// return 1 when the input is valid character
	// return 0 when the input is invalid
	if ((var >= 'a' && var <= 'z') || (var >= 'A' && var <= 'Z') || var == '_')
        return true;
    else
        return false;
}
bool Calc::isDigit(char var)
{
	// check if the input is a digit or not ('0'~'9' or the dot '.' in decimal, like 3.14)
	// Input : one character in data type 'char'
	// Output:
	// return 1 when the input is valid digit
	// return 0 when the input is invalid
	if ((var >= '0' && var <= '9') || var == '.')
        return true;
    else
        return false;
}

bool Calc::isOp(char var)
{
	// check if the input is a operator or not (+ , - , * , ++, -- are valid)
	// Input : one character in data type 'char'
	// Output:
	// return 1 when the input is valid operator
	// return 0 when the input is invalid
	if (var == '+' || var == '-' || var == '*' || var == '/' || var == '\\' || var == '^' || var == '%' || var == '=') //Note: ++ and -- cannot be checked because they are two-character strings.
        return true;
    else
        return false;
}

bool Calc::isParen(char var)
{
    // check if the input is a bracket type ( (, ) )
	// Input : one character in data type 'char'
	// Output:
	// return 1 when the input is valid operator
	// return 0 when the input is invalid
	if ( var == ')' || var == '(')
        return true;
    else
        return false;
}

// Create a variable database for this Calc object.
bool Calc::createDB()
{
    m_db = new CVarDB;
    m_ans = m_db->getAns();
    return (m_db != 0);
}

// Print out an error
void Calc::printError()
{
    cout << "\tError: " << lastErr << endl;
}

// Declaration of string copy functions.
void substr_cpy(char* chr, strItr start, strItr end)
{
    for (int i = 0; start < end; ++start, ++i)
    {
        chr[i] = *start;
    }
}

void substr_cpy(string& out, strItr start, strItr end)
{
    for (; start < end; ++start)
    {
        out.push_back(*start);
    }
}
