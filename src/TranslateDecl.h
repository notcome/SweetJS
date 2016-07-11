#ifndef SWEETJS_TRANSLATOR_DECL_H
#define SWEETJS_TRANSLATOR_DECL_H

#include "swift/AST/AST.h"
#include "swift/AST/ASTVisitor.h"

namespace swift {
    class TranslateDecl : public DeclVisitor<TranslateDecl> {
    public:
        raw_ostream &out;
        raw_ostream &err;
        unsigned indent;
        
        TranslateDecl(raw_ostream &out, raw_ostream &err, unsigned indent)
        : out(out), err(err), indent(indent) {}
    #define DECL(CLASS, PARENT) \
        void visit##CLASS##Decl(CLASS##Decl*);
    #include "swift/AST/DeclNodes.def"
    #undef DECL
    };
}

#endif
