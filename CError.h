#ifndef CERROR_H
#define CERROR_H

#include <string>

using namespace std;

enum ERTYPE
{
    EPARTI,
    EINTER,
    ECONVE,
    ECALCU,
    EMATRI,
    EOPERAT
};

class CError
{
    public:
        /** Default constructor */
        CError(const string& str, ERTYPE tp, bool ab) : errStr{str}, type{tp}, abortCalc{ab} {};
        /** Default destructor */
        virtual ~CError();
        const string&   get()      const  { return errStr;   };
        const ERTYPE    getType()  const  { return type;     };
        const bool      getAbort() const  { return abortCalc;    };
    protected:
    private:
        string errStr;
        ERTYPE type;
        bool abortCalc;
};

#endif // CERROR_H
