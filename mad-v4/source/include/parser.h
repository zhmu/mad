/* are we already included? */
#ifndef __PARSER_INCLUDED__
/* no. set flag we are included, and define things */
#define __PARSER_INCLUDED__

#include "types.h"

#define  PARSER_SPAREX  20

class PARSER {
public:
    void init();
    void done();

    void handlevent();

private:
    _UINT dialog_no,text_no;
};

#endif /* __PARSER_INCLUDED__ */
