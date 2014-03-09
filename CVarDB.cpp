#include "CVarDB.h"
#include <cstring>

CVarDB::CVarDB() : m_nSize{1}
{
    //ctor

    //Create default ans variable
    if (m_pDB[0].SetName("ans"))
        m_pDB[0] = 0;
    else
        throw "Bad Allocation of ans";
}

CVariable* CVarDB::search(const char* name)
{
    //search the database for the variable name
    for (int i = 0; i < m_nSize; ++i)
    {
        if (strcmp(m_pDB[i].Name(), name) == 0)
            return &m_pDB[i];
    }
    return NULL; //Return null if we can't find the name.
}

CMatrix      CVarDB::getVal(const char*name)
{
    return search(name)->Value();
}

CVariable* CVarDB::createVar(const char* name)
{
    //Actually just create a new variable of the same name with value 0.
    return createVar(name, 0);
}

CVariable* CVarDB::createVar(const char* name, const double& d)
{
    if (m_nSize < SIZE_DB) //if we have room
    {
        m_pDB[m_nSize].SetName(name);
        m_pDB[m_nSize].SetValue(d);
        return &m_pDB[m_nSize++];
    }
    else
        return NULL;
}

CVariable* CVarDB::createVar(const char* name, const CMatrix& m)
{
    if (m_nSize < SIZE_DB) //if we have room
    {
        m_pDB[m_nSize].SetName(name);
        m_pDB[m_nSize].SetValue(m);
        return &m_pDB[m_nSize++];
    }
    else
        return NULL;
}

void CVarDB::dump() //Essentially clears the contents of the DB
{
    //Set the ans variable to zero.
    m_pDB[0] = 0;

    for (int i = 1; i < SIZE_DB; ++i)
    {
        m_pDB[0].Clear();
    }

    //Move the counter for the number of elements back to the beginning, so we write over the old variables.
    m_nSize = 1;
}
