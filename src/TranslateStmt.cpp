#include "Translator.h"
using namespace std;
using namespace swift;

template <typename IterT, typename EndIterT>
vector<VarDecl*> getFollowingVarDecls(IterT& i, const EndIterT& end) {
    vector<VarDecl*> varDecls;
    for (++i; i != end; ++i) {
        ASTNode node = *i;
        auto decl = node.dyn_cast<Decl*>();
        if (decl == nullptr)
            break;
        auto varDecl = dyn_cast<VarDecl>(decl);
        if (varDecl == nullptr)
            break;
        varDecls.push_back(varDecl);
    }
    --i;
    return varDecls;
}

void TranslateStmt::visitBraceStmt(BraceStmt* braceStmt) {
    out << "{\n";
    indent += 2;
    
    auto elements = braceStmt->getElements();
    for (auto i = elements.begin(); i != elements.end(); ++i) {
        auto element = *i;
        if (auto subExpr = element.dyn_cast<Expr*>()) {
            out.indent(indent) << TranslateExpr().visit(subExpr) << ";\n";
        } else if (auto subStmt = element.dyn_cast<Stmt*>()) {
            visit(subStmt);
        } else {
            auto subDecl = element.dyn_cast<Decl*>();
            if (auto pbd = dyn_cast<PatternBindingDecl>(subDecl)) {
                auto varDecls = getFollowingVarDecls(i, elements.end());
                auto translate = TranslatePattern(out, err, indent);
                translate.translateTotalPatternBindingDecl(pbd, varDecls);
                continue;
            }
        }
    }
    
    indent -= 2;
    out.indent(indent) << "}";
}

Expr* unwrapSimpleCond(const StmtCondition& cond) {
    if (cond.size() > 1)
        assert(false && "expected only one condition\n");
    return unwrapBool(cond[0].getBoolean());
}

void TranslateStmt::visitIfStmt(IfStmt* ifStmt) {
    out.indent(indent);
    while (true) {
        out << fillTemplate("if (@) ", {
            TranslateExpr().visit(unwrapSimpleCond(ifStmt->getCond()))
        });
        visit(ifStmt->getThenStmt());
        auto elseStmt = ifStmt->getElseStmt();
        if (elseStmt == nullptr)
            break;
        out << " else ";
        if (dyn_cast<IfStmt>(elseStmt) == nullptr) {
            visit(elseStmt);
            break;
        } else {
            ifStmt = dyn_cast<IfStmt>(elseStmt);
        }
    }
    out << "\n";
}

void TranslateStmt::visitRepeatWhileStmt(RepeatWhileStmt *rws) {
    out.indent(indent) << "do ";
    visit(rws->getBody());
    out << fillTemplate(" while (@);\n", {
        TranslateExpr().visit(unwrapBool(rws->getCond()))
    });
}

void TranslateStmt::visitReturnStmt(ReturnStmt* stmt) {
    out.indent(indent) << fillTemplate("return @;\n", {
        TranslateExpr().visit(stmt->getResult())
    });
}

#define UNIMPL(NAME) \
    void TranslateStmt::visit##NAME##Stmt(NAME##Stmt* stmt) { \
        assert(false && "visit##NAME##Stmt unimplemented\n"); \
    }
UNIMPL(Guard)
UNIMPL(Do)
UNIMPL(Defer)
UNIMPL(For)
UNIMPL(ForEach)
UNIMPL(Switch)
UNIMPL(Case)
UNIMPL(Fallthrough)
UNIMPL(Throw)
UNIMPL(Catch)
UNIMPL(Fail)
UNIMPL(Break)
UNIMPL(Continue)
UNIMPL(IfConfig)
UNIMPL(While)
UNIMPL(DoCatch)

#undef UNIMPL
