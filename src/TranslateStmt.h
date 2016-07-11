#ifndef SWEETJS_TRANSLATOR_STMT_H
#define SWEETJS_TRANSLATOR_STMT_H

#include "swift/AST/AST.h"
#include "swift/AST/ASTVisitor.h"

namespace swift {
    class TranslateStmt : public StmtVisitor<TranslateStmt> {
    public:
        raw_ostream &out;
        raw_ostream &err;
        unsigned indent;
        
        TranslateStmt(raw_ostream &out, raw_ostream &err, unsigned indent)
        : out(out), err(err), indent(indent) {}
    #define STMT(CLASS, PARENT) \
        void visit##CLASS##Stmt(CLASS##Stmt*);
    #include "swift/AST/StmtNodes.def"
    #undef STMT
    };
}

#endif
