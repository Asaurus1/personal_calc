#include "CMatrix.h"
#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <cassert>

using namespace std;

//Define nullzero
double CMatrix::nullzero = nan("");

CMatrix::CMatrix() : m_aData{0}
{
    makeNullMatrix();
}

//Generic private function to make a null matrix. Used to effect constructor chaining.
void CMatrix::makeNullMatrix()
{
    if (m_aData != 0)
        delete [] m_aData;

	m_nRow = 0;
	m_nCol = 0;
	m_isNull = true;
	m_aData = 0; //Null Pointer
	CMatrix::nullzero = nan(""); //Set the NAN element in case anyone tries to print this.
}

//Allocate for a single double and assign the value of d.
CMatrix::CMatrix(double d) : m_aData{0}
{
	m_nRow = 1;
	m_nCol = 1;
	m_isNull = false;
	m_aData = new double[1] {};
	*m_aData = d;
}

//Initialize a matrix from a string. This function first checks whether the matrix has the valid number of elements,
// then it reads those elements into newly allocated memory one by one.
CMatrix::CMatrix(char* str) : m_aData{0}
{
    int elemNum = 0;

    //Check whether the matrix is a valid matrix, and if it is, assign the number of elements to elemNum.
    if ((elemNum = isValidMatrix(str)) > 0)
    {
        //Reserve new memory. Since we know the exact number of elements, we don't need to worry about knowing rows and columns yet.
        m_aData = new double [elemNum] {}; //Initialize the new memory to zero.
        m_isNull = false; //Obviously, since we are proceeding with this process, we won't have a null matrix (hopefully)

        //Declare some default variables.
        int curElem = 0;
        int nRow = 1; //We know we have at least one row

        ++str; //Move inside the matrix

        //Loop while we haven't reached the end of the matrix.
        while (*str != ']')
        {
            //Truncate whitespace
            while (*str == ' ' || *str == ',')
            {
                ++str;
            }

            //Have we reached an element?
            if (isADigit(*str) || *str == '-')
            {
                int decpwr = 0;
                bool neg = (*str == '-' ? true : false);
                //Here we read in the string characters and convert them to a double.
                do
                {
                    //If we have reached or passed the decimal point, keep counting up.
                    if (*str =='.')
                    {
                        decpwr = 1;
                        str++;
                        continue;
                    }
                    if (decpwr > 0.0)
                        decpwr *= 10;

                    //Multiply whatever is in the element by 10 and add in the current value as a ones place.
                    m_aData[curElem] *= 10;
                    m_aData[curElem] += *str - '0';
                    //Move forward in the string
                    ++str;
                } while(isADigit(*str));

                //Divide by the proper power.
                if (decpwr > 0.0)
                    m_aData[curElem] /= decpwr;

                //Handle negatives
                m_aData[curElem] *= (neg ? -1 : 1);

                //Move to the next spot to write to.
                ++curElem;
            }
            //If we reach a row-break, add one to the row counter.
            else if (*str == ';')
            {
                ++nRow;
                ++str;
            }
            //If we find an invalid character (including the End of String (0)
            else
                break;
        }

        //Set the number of rows and columns accordingly.
        m_nRow = nRow;
        m_nCol = elemNum / nRow;
    }
    //If the matrix in the string is invalid, then we make a null matrix.
    else
    {
        makeNullMatrix();
    }
}

//Constructs a matrix from an array of elements. The number of elements in the array must be nRow * nCol.

CMatrix::CMatrix(double arr[], int nRow, int nCol) : m_aData{0}
{
    if (nRow <= 0 || nCol <=0)
        makeNullMatrix();
    else
    {
        m_nRow = nRow;
        m_nCol = nCol;
        m_isNull = false;
        m_aData = new double [nRow * nCol]  {}; //Initialize every element to zero (actually to double())
        for (int i = 0; i < nRow * nCol; ++i)
            m_aData[i] = arr[i];
    }
}

//Create an nRow by nCol matrix and initialize it to zero.
CMatrix::CMatrix(int nRow, int nCol) : m_aData{0}
{
    if (nRow <= 0 || nCol <=0 )
        makeNullMatrix();
    else
    {
        m_nRow = nRow;
        m_nCol = nCol;
        m_isNull = false;
        m_aData = new double [nRow * nCol]  {}; //Initialize every element to zero (actually to double())
    }
}

//Copy constructor
CMatrix::CMatrix(const CMatrix& m) : m_aData{0}
{
    copy(m);
}

//Destructor
CMatrix::~CMatrix()
{
    //If the matrix is not null (when it is null, no memory is allocated)
	if (!m_isNull && m_aData != 0)
        //Then delete the allocated data.
		delete [] m_aData;
	m_aData = 0; //Set to null, just for safety's sake;
}

void CMatrix::copy(const CMatrix& m)
{
    m_nRow = m.m_nRow;
    m_nCol = m.m_nCol;
    m_isNull = m.m_isNull;
    double* newdata = new double [m_nRow * m_nCol]; //Allocate new memory for the copy.

    //Copy the data
    for (int i = 0; i < m_nRow*m_nCol; ++i)
    {
            newdata[i] = m.m_aData[i];
    }

    if (m_aData != NULL)
        delete [] m_aData;

    m_aData = newdata;
}

void CMatrix::swap(CMatrix &m)
{
    //Initialize temporary variables for data members.
    int t_row, t_col, t_null;
    double* t_ptr;

    //Now we swap all the members of the classes.
    t_row = m.m_nRow;
    t_col = m.m_nCol;
    t_null = m.m_isNull;
    t_ptr = m.m_aData;

    m.m_nRow = m_nRow;
    m.m_nCol = m_nCol;
    m.m_isNull = m_isNull;
    m.m_aData = m_aData;

    m_nRow = t_row;
    m_nCol = t_col;
    m_isNull = t_null;
    m_aData = t_ptr;

    //And we're done!
}

//Resizes matrix to size nRow x nCol. This operation cannot be undone, and any new spaces are filled with zeros.
void CMatrix::resize(int nRow, int nCol)
{
    //Create a pointer to the new matrix;
    double* new_matrix = 0;

    //If we got bad inputs, this becomes a null matrix.
	if (nRow <= 0 && nCol <= 0)
	{
		m_isNull = true;
	}
	else
	{
		//Create a new matrix pointer to allocate the new memory to.
		new_matrix = new double[nRow * nCol] {};

		//Fill the new matrix will the elements from the old matrix.
		short i, j;

		for(i = 0; i < nRow; ++i)
		{
			for (j = 0; j < nCol; ++j)
			{
				//Check whether we are inside the bounds of the old matrix.
				if (i < m_nRow && j < m_nCol && !m_isNull)
				{
					new_matrix[i*nCol + j] = m_aData[i*m_nCol + j]; //Copy a value from the old matrix
				}
				else
				{
					new_matrix[i*nCol + j] = 0;                     //Add a zero if this is not a position which existed in the old matrix.
				}
			}
		}
        m_isNull = false; //The size is non-zero, so m_isNull should be false.
	}

	//Delete the old memory
	delete [] m_aData;
	m_aData = 0; //Set to null for safety;

	//Reassign pointers
    m_aData = new_matrix;

	//Update Rows and Columns
	m_nRow = nRow;
	m_nCol = nCol;
}

//Returns a reference to the i,j_th element. This can be used as both a r- and l- value, so we can assign and retrieve.
double &CMatrix::element(int i, int j)
{
	if (i < getNRow()
	  && j < getNCol()
	  && i >= 0
	  && j >= 0
	  && !m_isNull)
		return m_aData[i*m_nCol+ j];
    else
    {
        nullzero = nan("");
        return nullzero; //Return NAN*/
    }
}

const double &CMatrix::element(int i, int j) const
{
	if (i < getNRow()
	  && j < getNCol()
	  && i >= 0
	  && j >= 0
	  && !m_isNull)
		return m_aData[i*m_nCol+ j];
    else
    {
        nullzero = nan("");
        return nullzero; //Return NAN
    }
}

//Access like an array, but with round brackets
double& CMatrix::operator()(int i, int j)
{
    return this->element(i,j);
}

const double& CMatrix::operator()(int i, int j) const
{
	return this->element(i,j);
}

//Prints a matrix m to cout.
void PrintMatrix( CMatrix& m, std::ostream& out, const std::string& lnstart, bool single_as_matrix)
{
    if (m.IsNull())
        out << "\tnull matrix" << endl;
    else if (m.IsSingle() && !single_as_matrix)
        out << m.element(0,0);
    else
    {
        int i,j,r,c;
        r = m.getNRow();
        c = m.getNCol();

        left(out);
        //out << endl;
        for (i = 0; i < r; ++i)
        {
            if (i == 0)
                out << '[';
            else
                out << lnstart; // Print the characters that go at the start of a line.

            for (j = 0; j < c; ++j)
            {
                out << setw(6) << m.element(i,j); //Print the elements of the i^th row.
            }
            if (i == r-1)
                out << setw(2) << ']';
            out << endl; //Print a newline
        }
    }
    //Initialize counters and bounds

}

ostream& operator<<( ostream& out, const CMatrix& m)
{
    if (m.IsNull())
        out << "\tnull matrix" << endl;
    else if (m.IsSingle())
        out << m.element(0,0);
    else
    {
        int i,j,r,c;
        r = m.getNRow();
        c = m.getNCol();

        left(out);
        out << endl;
        for (i = 0; i < r; ++i)
        {
            out << "\t\t"; // Print tab character

            for (j = 0; j < c; ++j)
            {
                out << setw(6) << m.element(i,j); //Print the elements of the i^th row.
            }
            out << endl; //Print a newline
        }
    }
    return out;
}

//A function which checks whether a given matrix string is valid
int isValidMatrix(const char* str)
{
    int eCtr = 0, cCtr = 0; //counts total num of elements in matrix and the number of columns in the matrix

    //If the beginning is wrong, we just quit.
    if (*str != '[')
        return 0;

    ++str; //Move inside the matrix

    //Accumulation phase for the counter variables. We loop until we hit the end of the first row ';'
    while (*str != ']' && *str != 0)
    {
        //Truncate whitespace
        while (*str == ' ' || *str == ',')
        {
            ++str;
        }

        //If we've found an element
        if (isADigit(*str))
        {
            //Increment the counters by 1
            ++cCtr;
            ++eCtr;
            ++str;

            //Check for double decimals
            bool decCheck = false;
            while (isADigit(*str))
            {
                if (*str == '.')
                {
                    if (decCheck)
                        return 0;
                    else
                        decCheck = true;
                }
                ++str;
            }
        }
        //If we need to stop counting elements because we found one that was bad.
        else
            break;
    }

    //If we end on a close brace, this is a 1-row matrix.
    if (*str == ']')
    {
        return eCtr;
    }
    //Otherwise, if we have not finished on a row break (;) then the input is invalid.
    if (*str != ';')
    {
        return 0; //Cry
    }

    ++str; //If we do not return above, then keep going. Move the pointer forwards


    //Verification phase. Now that we have the number of columns, we check whether the rest of the matrix matches.
    int check = 0;

    while (true)
    {
        while (*str == ' ' || *str == ',') //Truncate whitespace
        {
            ++str;
        }

        //If we've reached an element
        if (isADigit(*str))
        {
            //Increment the counter by 1
            check += 1;
            eCtr += 1;
            ++str;

            //Skip the rest of the number and performs a decimal check.
            bool decCheck = false;
            while (isADigit(*str))
            {
                if (*str == '.')
                {
                    if (decCheck)
                        return 0;
                    else
                        decCheck = true;
                }
                ++str;
            }
        }
        //If we need to stop counting elements
        else if (*str == ';' || *str == ']')
        {
            //Check whether the column sizes match.
            if (check == cCtr)
            {
                check = 0;

                //Are we finished or is there supposed to be more?
                if (*str == ']')
                    break;
                else
                {
                    ++str;
                    continue;
                }
            }
            //If they do not, Cry!
            else
                return 0;
        }
       //If we find an invalid character (including the End of String (0))
        else
            return 0;
    }

    //If we haven't returned 0 yet, then we're good.
    return eCtr;
}

bool isADigit(char var)
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

//###################### OVERLOADS ######################

// assignment
	const CMatrix& CMatrix::operator=(const CMatrix& m)
	{
        copy(m);
        return *this;
	}
	const CMatrix& CMatrix::operator=(CMatrix&& m) //R-values
	{
	    swap(m);
	    return *this;
	}
	const CMatrix& CMatrix::operator=(const double& k)
	{
	    resize(1,1);
	    m_aData[0] = k;
	    return *this;
	}

// compare equal
	bool CMatrix::operator==(const CMatrix& m) const
	{
	    bool isEqual = false;
	    if (m_nRow == m.m_nRow && m_nCol == m.m_nCol)
        {
            isEqual = true;
            for (int i = 0; i < m_nRow * m_nCol; ++i)
            {
                if (m_aData[i] != m.m_aData[i])
                    isEqual = false;
            }
        }
	    return isEqual;
	}
	bool CMatrix::operator==(const double& v) const
	{
	    if (IsSingle() && *m_aData == v)
            return true;
        else
            return false;
	}

	// +/-/*
	CMatrix	CMatrix::operator+(const CMatrix& m) const
	{
	    if (m.IsSingle())
            return operator+(m.element(0,0)); //treat as a double

	    int nRow = getNRow(), nCol = getNCol();
	    CMatrix addMtrx{nRow, nCol};

	    if (nRow == m.getNRow() && nCol == m.getNCol())
        {
            for (int i = 0; i < nRow * nCol; ++i)
            {
                addMtrx.m_aData[i] = this->m_aData[i] + m.m_aData[i];
            }
            return addMtrx;
        }
        else return CMatrix{};

	}
	CMatrix	CMatrix::operator-(const CMatrix& m) const
	{
	    if (m.IsSingle())
            return operator-(m.element(0,0)); //treat as a double

	    int nRow = getNRow(), nCol = getNCol();
	    CMatrix addMtrx{nRow, nCol};

	    if (nRow == m.getNRow() && nCol == m.getNCol())
        {
            for (int i = 0; i < nRow * nCol; ++i)
            {
                addMtrx.m_aData[i] = this->m_aData[i] - m.m_aData[i];
            }
            return addMtrx;
        }
        else return CMatrix{};
	}
	CMatrix	CMatrix::operator*(const CMatrix& m) const// is.*, not matrix multiplication
	{
	    if (m.IsSingle())
            return operator*(m.element(0,0)); //treat as a double

        //If the sizes are such that we can perform traditional matrix multiplication, we do that
        if (getNCol() == m.getNRow())
        {
            //The new matrix will have size this.nRow and m.nCol
            int resnRow = getNRow(), resnCol = m.getNCol();
            CMatrix addMtrx{resnRow, resnCol};
            //Loop through the elements of the matrices
            double rowsum;
            for (int i = 0; i < resnRow; ++i)
            {
                for (int k = 0; k < resnCol; ++k)
                {
                    rowsum = 0;
                    for (int j = 0; j < getNCol(); ++j)
                    {
                        rowsum += this->element(i,j) * m.element(j, k);
                    }
                    addMtrx.element(i,k) = rowsum;
                }
            }
            return addMtrx;
        }
        //If the matrices are the same size, we do element multiplication (we can implement .* later)
	    else if (getNRow() == m.getNRow() && getNCol() == m.getNCol())
        {
            int nRow = getNRow(), nCol = getNCol();
            CMatrix addMtrx{nRow, nCol};
            for (int i = 0; i < nRow * nCol; ++i)
            {
                addMtrx.m_aData[i] = this->m_aData[i] * m.m_aData[i];
            }
            return addMtrx;
        }
        else return CMatrix{};
	}
	CMatrix	CMatrix::operator/(const CMatrix& m) const // is ./, not matrix inverse
	{
	    if (m.IsSingle())
            return operator/(m.element(0,0)); //treat as a double

        int nRow = getNRow(), nCol = getNCol();
        CMatrix addMtrx{nRow, nCol};

        if (nRow == m.getNRow() && nCol == m.getNCol())
        {
            for (int i = 0; i < nRow * nCol; ++i)
            {
                addMtrx.m_aData[i] = this->m_aData[i] / m.m_aData[i];
            }
            return addMtrx;
        }
        else return CMatrix{};
    }

    CMatrix	CMatrix::operator+(const double& t) const
    {
        CMatrix outmtrx = *this;
        outmtrx.sAdd(t);
        return outmtrx;
    }
	CMatrix	CMatrix::operator-(const double& t) const
    {
        CMatrix outmtrx = *this;
        outmtrx.sAdd(-t);
        return outmtrx;
    }
	CMatrix	CMatrix::operator*(const double& t) const
	{
        CMatrix outmtrx = *this;
        outmtrx.sMult(t);
        return outmtrx;
    }
	CMatrix	CMatrix::operator/(const double& t) const
	{
	    CMatrix outmtrx; //outmtrx is null
	    if (t != 0)
            outmtrx = *this;
            outmtrx.sMult(1/t);
        return outmtrx;
	}

	//EQUALITY OPERATORS += -= *= /=
	CMatrix&    CMatrix::operator+=(const CMatrix& m)
	{
	    if (m_nRow == m.getNRow() && m_nCol == m.getNCol())
        {
            for (int i = 0; i < m_nRow * m_nCol; ++i)
            {
                m_aData[i] += m.m_aData[i];
            }
        }
        return *this;
	}
	CMatrix&	CMatrix::operator-=(const CMatrix& m)
	{
	    if (m_nRow == m.getNRow() && m_nCol == m.getNCol())
        {
            for (int i = 0; i < m_nRow * m_nCol; ++i)
            {
                m_aData[i] -= m.m_aData[i];
            }
        }
        return *this;
	}
	CMatrix&	CMatrix::operator*=(const CMatrix& m) // is .*, not matric multiplication
	{
	    if (m_nRow == m.getNRow() && m_nCol == m.getNCol())
        {
            for (int i = 0; i < m_nRow * m_nCol; ++i)
            {
                m_aData[i] *= m.m_aData[i];
            }
        }
        return *this;
	}
	CMatrix&	CMatrix::operator/=(const CMatrix& m) // is ./, not matrix inverse
	{
	    if (m_nRow == m.getNRow() && m_nCol == m.getNCol())
        {
            for (int i = 0; i < m_nRow * m_nCol; ++i)
            {
                if (m.m_aData != 0)
                    m_aData[i] /= m.m_aData[i];
                else
                    makeNullMatrix();
            }
        }
        return *this;
    }

    CMatrix&	CMatrix::operator+=(const double& t)
    {
        sAdd(t);
        return *this;
    }
	CMatrix&	CMatrix::operator-=(const double& t)
    {
        sAdd(-t);
        return *this;
    }
	CMatrix&	CMatrix::operator*=(const double& t)
	{
        sMult(t);
        return *this;
    }
	CMatrix&	CMatrix::operator/=(const double& t)
	{
	    if (t == 0)
            makeNullMatrix();
        else
            sMult(1/t);
        return *this;
	}

//################### EXTRA FUNCTIONS ###################

CMatrix& CMatrix::Neg()
{
    for (int i = 0; i < m_nRow; ++i)
    {
        ero_multrow(i,-1);
    }
    return *this;
}

void CMatrix::fill(double d)
{
    //If we are null, do nothing
    if (!m_isNull)
    {
        //Set every element to d
        for (int i = 0; i < m_nRow * m_nCol; ++i)
        {
            m_aData[i] = d;
        }
    }
}

void CMatrix::identity(int n)
{
    fill(0);
    resize(n,n);
    for (int i = 0; i < n; ++i)
        m_aData[i*m_nCol+i] = 1;
}


void CMatrix::ero_switchrow(int i, int j)
{
    if (!m_isNull && i >= 0 && j >= 0 && i < m_nRow && j < m_nRow)
    {
        double temp;

        //Swap each member of the two rows specified
        for (int n = 0; n < m_nCol; ++n)
        {
            temp = m_aData[i*m_nCol+n];
            m_aData[i*m_nCol+n] = m_aData[j*m_nCol+n];
            m_aData[j*m_nCol+n] = temp;
        }
    }
}

void CMatrix::ero_multrow(int i, double d)
{
    if (!m_isNull && i >= 0 && i < m_nRow)
    {
        //Multiply each element by d
        for (int n = 0; n < m_nCol; ++n)
        {
            m_aData[i*m_nCol+n] *= d;
        }
    }
}

void CMatrix::ero_addrow(int i, int j, double d = 1)
{
    if (!m_isNull && i >= 0 && j >= 0 && i < m_nRow && j < m_nRow)
    {
        //Add the members of row i times double d to row j.
        for (int n = 0; n < m_nCol; ++n)
        {
             m_aData[j*m_nCol+n] += m_aData[i*m_nCol+n] * d;
        }
    }
}

CMatrix CMatrix::getRow(int i)
{
    if (!m_isNull && i >= 0 && i < m_nRow)
    {
        CMatrix newmatrix(*this);
        newmatrix.ero_switchrow(i,1);
        newmatrix.resize(1,m_nCol);
        return newmatrix;
    }
    else
    {
        CMatrix newmatrix;
        return newmatrix;
    }
}

void CMatrix::sAdd(double s)
{
    for (int i = 0; i < m_nCol * m_nRow; ++i)
        m_aData[i] += s;
}

void CMatrix::sMult(double s)
{
    for (int i = 0; i < m_nCol * m_nRow; ++i)
        m_aData[i] *= s;
}

/*CMatrix& CMatrix::mtrxMult(const CMatrix& m)
{
    if matrix
}*/
