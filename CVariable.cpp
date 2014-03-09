#include "CVariable.h"
#include <cstring>
#include <iostream>

CVariable::CVariable() : m_xValue{}, m_sName{NULL}
{}

CVariable::CVariable(const char* name, const CMatrix& v) : m_xValue{v}, m_sName{NULL}
{
    //Set the name
    SetName(name);
}

CVariable::CVariable(const char*name, const double& d) : m_xValue{d}, m_sName{NULL}
{
    //Set the name
    SetName(name);
}

CVariable::~CVariable()
{
   if (m_sName != NULL)
   {
       delete [] m_sName;
       m_sName = NULL;
   }
}

CVariable::CVariable(const CVariable& var) : m_sName{NULL}
{
    SetName(var.m_sName);
    //Copy the value
    m_xValue = var.m_xValue;
}

const CVariable& CVariable::operator=(CVariable&& var) //overload = for other variables
{
    //Check for self-assignment
    if (var == *this)
    {
        return *this;
    }

    m_xValue = var.m_xValue;

    if (m_sName == NULL && var.m_sName != NULL)
    {
        //swap pointers, since this is a temporary object and will be deleted anyways.
        m_sName = var.m_sName;
        var.m_sName = 0;
    }
    return *this; //Return this object.
}

const CVariable& CVariable::operator=(const CVariable& var) //overload = for other variables
{
    //Check for self-assignment
    if (var == *this)
    {
        return *this;
    }

    //Set the value of the variable.
    m_xValue = var.m_xValue;

    //If I don't have a name already, give me the one in the other object.
    if (m_sName == NULL)
    {
        SetName(var.m_sName);
    }
    return *this; //Return this object.
}

const CVariable& CVariable::operator=(const CMatrix& m)
{
    m_xValue = m;
    return *this;
}
const CVariable& CVariable::operator=(const double& m)
{
    m_xValue = m;
    return *this;
}

bool CVariable::SetName(const char* name)
{
        //Allocate enough memory for this new name.
        char* newname = new char [strlen(name)];

        //Check for a failure to allocate
        if (newname == NULL)
            return false;

        //Copy
        strcpy(newname, name);

        //Destroy the old name string
        if (m_sName != NULL)
            delete [] m_sName;

        //And hand over the pointer
        m_sName = newname;
        return true;
}

void CVariable::Clear()
{
    delete [] m_sName;
    m_sName = NULL;
    m_xValue.resize(0,0); //Set my matrix to null.
}
