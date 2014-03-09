// matrix.h: interface of the CMatrix class

#ifndef CMATRIX_H
#define CMATRIX_H

#include <iostream>

class CMatrix
{
	int		m_nRow; // # of rows
	int		m_nCol; // # of columns
	bool 	m_isNull;
	double	*m_aData;
	static double nullzero;

	void makeNullMatrix();

public:
	CMatrix(); 	// make a null matrix

	CMatrix(double d); // 1 by 1 matrix
	CMatrix(int nRow, int nCol); // nRow by nCol zero matrix

	CMatrix(char *str); // matrix from string

    CMatrix(double arr[], int nRow, int nCol); // initializes a vector from an array.

	CMatrix(const CMatrix& m); //Copy Constructor

	~CMatrix();

	// Is this matrix a null matrix?
	bool IsNull() const {return m_isNull;};
	// Is this matrix representing a single double
	bool IsSingle() const {return (m_nRow == 1 && m_nCol == 1);};

	void swap(CMatrix &m); // swap this and m
	void copy(const CMatrix& m); //copy matrix m to me

	void resize(int nRow, int nCol);

	// get
	int	getNRow() const { return (m_isNull) ? 0 : m_nRow; } // return # of rows
	int	getNCol() const { return (m_isNull) ? 0 : m_nCol; }// return # of columns
	int Size() const { return (m_isNull) ? 0 : m_nRow*m_nCol; };

	// return the element at i-th row and j-th column
	double &element(int i, int j);
    const double &element(int i, int j) const;
	double &operator() (int i, int j); //allows access to the i,j-th element of the matrix.
	const double &operator() (int i, int j) const; //allows reading to the i,j-th element of the matrix.

	// operator overloading
	// assignment
	const CMatrix& operator=(const CMatrix& m);
	const CMatrix& operator=(CMatrix&& m); //R-values
	const CMatrix& operator=(const double& k);
	// compare equal
	bool		   operator==(const CMatrix& m) const;
	bool		   operator==(const double& v) const;
	// not equal
	bool		   operator!=(const CMatrix &m) const { return !(*this==m); };
	bool		   operator!=(const double& v) const { return !(*this==v); };

	// +/-/*
	CMatrix		operator+(const CMatrix& m) const;
	CMatrix		operator-(const CMatrix& m) const;
	CMatrix		operator*(const CMatrix& m) const; // is.*, not matric multiplication
	CMatrix		operator/(const CMatrix& m) const; // is ./, not matrix inverse

	CMatrix		operator+(const double& t) const;
	CMatrix		operator-(const double& t) const;
	CMatrix		operator*(const double& t) const;
	CMatrix		operator/(const double& t) const;

	CMatrix&	operator+=(const CMatrix& m);
	CMatrix&	operator-=(const CMatrix& m);
	CMatrix&	operator*=(const CMatrix& m);
	CMatrix&	operator/=(const CMatrix& m);

	CMatrix&	operator+=(const double& t);
	CMatrix&	operator-=(const double& t);
	CMatrix&	operator*=(const double& t);
	CMatrix&	operator/=(const double& t);

	CMatrix&	Neg(); // this is for matrix subtraction, to get the negative value of every element in the matrix

	// stream I/O
	friend	std::ostream &operator<<( std::ostream &, const CMatrix &);

    /* Extra Functions ------
        These extra functions provide several operations which users might find useful when working with matrices.
        fill(double num)    ::: Fills a matrix with a single value num.

        identity(int n)     ::: Sets the matrix to n x n and fills it along the diagonal with ones. The other elements are zero.

        isSquare            ::: Returns whether the matrix is square.

        getTranspose()      ::: Returns a new matrix object which is the transpose of the calling instance. If the calling instance is a null, this is also null.

        sMult(double d)     ::: Multiplies every element in the matrix by the scalar d.

        sAdd(double d)      ::: Adds the scalar d to every element in the matrix.

        ero_switchrow(i, j) ::: Swaps rows i and j of the matrix. If the two rows are out of bounds, nothing is done.

        ero_multrow(i,d)    ::: Multiply row i by scalar d.

        ero_addrow(i,j,d=1) ::: Add d times row i to row j.

        getRow(i)           ::: Returns a CMatrix which holds a copy of the ith row.
    */

    void fill(double d);

    void identity(int n);

    bool isSquare()
    {
        return m_nRow == m_nCol;
    };

    void sMult(double s);

    void sAdd(double s);

    void ero_switchrow(int,int);

    void ero_multrow(int,double);

    void ero_addrow(int,int,double);

    CMatrix getRow(int);


}; // class CMatrix

// output the matrix (external function)
void PrintMatrix( CMatrix&, std::ostream& = std::cout, const std::string& = "", bool = false);

// loop the matrix and determine whether it has the correct number of elements. If so, it returns the number of elements in the matrix. If not, it returns 0.
int isValidMatrix(const char* str);

bool isADigit(char var);
#endif // CMATRIX_H
