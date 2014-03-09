#include "CVariable.h"

#ifndef CVARDB_H
#define CVARDB_H

#define SIZE_DB 20 //Max size of the database

//////////////////////////////////////////////////
//      Class CVarDB                            //
//////////////////////////////////////////////////

class CVarDB
{
        CVariable       m_pDB[SIZE_DB];
        int             m_nSize;    // size of the database
public:
        CVarDB();
        ~CVarDB(){};

        void            Init();

        // return a valid ptr if found, else a NULL
        CVariable*      search(const char*name);
        CMatrix          getVal(const char*name);

        // return a ptr of the new one, else a NULL
        CVariable*      createVar(const char*name);
        CVariable*      createVar(const char*name, const double& d);
        CVariable*      createVar(const char*name, const CMatrix& d);

        CVariable*      getAns() { return &m_pDB[0]; };
        CVariable*      at(int i)
        {
            if (i < m_nSize)
                return &m_pDB[i];
            else
                return 0;
        };
        int             size()   { return m_nSize; };

        void    dump();
};
#endif // CVARDB_H
