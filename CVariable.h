#ifndef CVARIABLE_H
#define CVARIABLE_H

#include <CMatrix.h>

//////////////////////////////////////////////////
//      Class CVariable                         //
//////////////////////////////////////////////////

class CVariable
{
        CMatrix  m_xValue;
        char*   m_sName;
public:
        // constructors and destructors
        CVariable();
        CVariable(const char*name, const CMatrix& v = 0.0);
        CVariable(const char*name, const double& v);
        ~CVariable();
        CVariable(const CVariable& var); // copy constructor
        const CVariable& operator=(const CVariable& var); // overload =
        const CVariable& operator=(CVariable&& var); // Move semantics for temporary objects.
        const CVariable& operator=(const CMatrix& m);
        const CVariable& operator=(const double& d);


        operator double() const { return m_xValue(0,0); }; //returns the first value of the matrix

        // getting and setting
        CMatrix& Value() { return m_xValue; };   // reference return creates a lvalue
        const CMatrix&   Value() const { return m_xValue; }; // const ref reture creates a rvalue
        char*   Name() const { return m_sName; };
        void    SetValue(const CMatrix& v) { m_xValue = v; };
        void    SetValue(CMatrix&& v) { m_xValue = v; }; //setValue for rvalues
        bool    SetName(const char* name);
        void    Clear();
};

typedef CVariable CVar;

// Declare (but don't define) function specializations

#endif // CVARIABLE_H

