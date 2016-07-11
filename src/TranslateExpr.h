#ifndef SWEETJS_TRANSLATOR_EXPR_H
#define SWEETJS_TRANSLATOR_EXPR_H

#include "swift/AST/AST.h"
#include "swift/AST/ASTVisitor.h"

namespace swift {
    class TranslateExpr : public ExprVisitor<TranslateExpr, std::string> {
    public:
    #define EXPR(CLASS, PARENT) \
        std::string visit##CLASS##Expr(CLASS##Expr*);
    #include "swift/AST/ExprNodes.def"
    #undef DECL
    };
    
    Expr* unwrapBool(Expr* expr);
}

#endif
