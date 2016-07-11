#include <string>
#include <vector>
#include <initializer_list>
using std::string;
using std::vector;
using std::initializer_list;

#include "Translator.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/APFloat.h"
using namespace swift;

#include "Tuple.h"

namespace swift {
    Expr* unwrapBool(Expr* expr) {
        auto callExpr = dyn_cast<CallExpr>(expr);
        expr = callExpr->getFn();
        auto dotSyntaxCallExpr = dyn_cast<DotSyntaxCallExpr>(expr);
        return dotSyntaxCallExpr->getArg();
    }
}

IntegerLiteralExpr* unwrapAsIntegerLiteral(CallExpr* callExpr) {
    auto tupleExpr = dyn_cast<TupleExpr>(callExpr->getArg());
    if (tupleExpr == nullptr)
        return nullptr;
    return dyn_cast<IntegerLiteralExpr>(tupleExpr->getElement(0));
}

FloatLiteralExpr* unwrapAsFloatLiteral(CallExpr* callExpr) {
    auto tupleExpr = dyn_cast<TupleExpr>(callExpr->getArg());
    if (tupleExpr == nullptr)
        return nullptr;
    return dyn_cast<FloatLiteralExpr>(tupleExpr->getElement(0));
}

BooleanLiteralExpr* unwrapAsBooleanLiteral(CallExpr* callExpr) {
    auto tupleExpr = dyn_cast<TupleExpr>(callExpr->getArg());
    if (tupleExpr == nullptr)
        return nullptr;
    return dyn_cast<BooleanLiteralExpr>(tupleExpr->getElement(0));
}

StringLiteralExpr* unwrapAsStringLiteral(CallExpr* callExpr) {
    return dyn_cast<StringLiteralExpr>(callExpr->getArg());
}

string TranslateExpr::visitCallExpr(CallExpr* callExpr) {
    if (auto lit = unwrapAsIntegerLiteral(callExpr))
        return visit(lit);
    if (auto lit = unwrapAsFloatLiteral(callExpr))
        return visit(lit);
    if (auto lit = unwrapAsBooleanLiteral(callExpr))
        return visit(lit);
    if (auto lit = unwrapAsStringLiteral(callExpr))
        return visit(lit);
    
    visit(callExpr->getFn());
    if (auto parenExpr = dyn_cast<ParenExpr>(callExpr->getArg())) {
        return visitParenExpr(parenExpr);
    }
    if (auto tupleExpr = dyn_cast<TupleExpr>(callExpr->getArg())) {
        auto strRpn = visitTupleExpr(tupleExpr);
        if (strRpn[0] == '[') {
            strRpn[0] = '(';
            strRpn[strRpn.size() - 1] = ')';
            return strRpn;
        }
        return fillTemplate("(@)", {strRpn});
    }
    assert(false && "unhandled case");
}

string TranslateExpr::visitDeclRefExpr(DeclRefExpr* declRef) {
    auto declName = declRef->getDecl()->getFullName();
    return string{declName.getBaseName().get()};
}

string TranslateExpr::visitBinaryExpr(BinaryExpr* binaryExpr) {
    auto op = visit(binaryExpr->getFn());
    auto argPair = dyn_cast<TupleExpr>(binaryExpr->getArg());
    auto lhs = visit(argPair->getElement(0));
    auto rhs = visit(argPair->getElement(1));
    return sepBy(" ", initializer_list<string>{lhs, op, rhs});
}

string TranslateExpr::visitParenExpr(ParenExpr* parenExpr) {
    return fillTemplate("(@)", {visit(parenExpr->getSubExpr())});
}

string TranslateExpr::visitIfExpr(IfExpr* ifExpr) {
    return fillTemplate("@ ? @ : @", {
        visit(ifExpr->getCondExpr()),
        visit(ifExpr->getThenExpr()),
        visit(ifExpr->getElseExpr())
    });
}

string TranslateExpr::visitNilLiteralExpr(NilLiteralExpr* nilLit) {
    return "null";
}

string TranslateExpr::visitIntegerLiteralExpr(IntegerLiteralExpr* intLit) {
    auto value = intLit->getValue();
    assert(value.getActiveBits() < 53 && "integer literal > 2^53-1");
    return value.toString(10, true);
}

string TranslateExpr::visitFloatLiteralExpr(FloatLiteralExpr* floatLit) {
    return floatLit->getDigitsText();
}

string TranslateExpr::visitBooleanLiteralExpr(BooleanLiteralExpr* boolLit) {
    return boolLit->getValue() ? "true" : "false";
}

string TranslateExpr::visitStringLiteralExpr(StringLiteralExpr* stringLit) {
    return stringLit->getValue();
}

string TranslateExpr::visitInterpolatedStringLiteralExpr
(InterpolatedStringLiteralExpr* isl) {
    vector<string> segments_;
    segments_.reserve(isl->getSegments().size());
    for (auto segment : isl->getSegments()) {
        if (auto stringLit = dyn_cast<StringLiteralExpr>(segment))
            segments_.push_back(visit(stringLit));
        else if (auto parenExpr = dyn_cast<ParenExpr>(segment))
            segments_.push_back(fillTemplate("${@}", {
                visit(parenExpr->getSubExpr())
            }));
        else
            segments_.push_back(visit(segment));
    }
    return fillTemplate("`@`", {sepBy("", segments_)});
}

string TranslateExpr::visitTupleExpr(TupleExpr* tupleExpr) {
    auto isArrayRep = isArrayRepTuple(tupleExpr->getElementNames());
    vector<string> subExprs;
    for (auto element : tupleExpr->getElements()) {
        subExprs.push_back(visit(element));
    }
    
    if (!isArrayRep) {
        auto labels = completeObjRepTupleLabels(tupleExpr->getElementNames());
        for (size_t i = 0; i < subExprs.size(); ++ i) {
            subExprs[i] = fillTemplate("@: @", {labels[i], subExprs[i]});
        }
    }
    return fillTemplate(isArrayRep ? "[@]" : "{@}", {sepBy(", ", subExprs)});
}

string TranslateExpr::visitAssignExpr(AssignExpr* assignExpr) {
    return fillTemplate("@ = @", {
        visit(assignExpr->getDest()),
        visit(assignExpr->getSrc())
    });
}

string TranslateExpr::visitLoadExpr(LoadExpr* loadExpr) {
    return visit(loadExpr->getSubExpr());
}

#define UNIMPL(NAME) \
    string TranslateExpr::visit##NAME##Expr(NAME##Expr* expr) { \
        assert(false && "visit##NAME##Expr unimplemented\n"); \
    }

UNIMPL(Error)
UNIMPL(ObjectLiteral)
UNIMPL(MagicIdentifierLiteral)
UNIMPL(DiscardAssignment)
UNIMPL(SuperRef)
UNIMPL(Type)
UNIMPL(OtherConstructorDeclRef)
UNIMPL(DotSyntaxBaseIgnored)
UNIMPL(OverloadedDeclRef)
UNIMPL(OverloadedMemberRef)
UNIMPL(UnresolvedDeclRef)
UNIMPL(MemberRef)
UNIMPL(DynamicMemberRef)
UNIMPL(DynamicSubscript)
UNIMPL(UnresolvedSpecialize)
UNIMPL(UnresolvedMember)
UNIMPL(UnresolvedDot)
UNIMPL(Sequence)
UNIMPL(DotSelf)
UNIMPL(Try)
UNIMPL(ForceTry)
UNIMPL(OptionalTry)
UNIMPL(Array)
UNIMPL(Dictionary)
UNIMPL(Subscript)
UNIMPL(TupleElement)
UNIMPL(CaptureList)
UNIMPL(Closure)
UNIMPL(AutoClosure)
UNIMPL(InOut)
UNIMPL(DynamicType)
UNIMPL(RebindSelfInConstructor)
UNIMPL(OpaqueValue)
UNIMPL(BindOptional)
UNIMPL(OptionalEvaluation)
UNIMPL(ForceValue)
UNIMPL(OpenExistential)
UNIMPL(PrefixUnary)
UNIMPL(PostfixUnary)
UNIMPL(DotSyntaxCall)
UNIMPL(ConstructorRefCall)
UNIMPL(TupleShuffle)
UNIMPL(UnresolvedTypeConversion)
UNIMPL(FunctionConversion)
UNIMPL(CovariantFunctionConversion)
UNIMPL(CovariantReturnConversion)
UNIMPL(MetatypeConversion)
UNIMPL(CollectionUpcastConversion)
UNIMPL(Erasure)
UNIMPL(DerivedToBase)
UNIMPL(ArchetypeToSuper)
UNIMPL(InjectIntoOptional)
UNIMPL(ClassMetatypeToObject)
UNIMPL(ExistentialMetatypeToObject)
UNIMPL(ProtocolMetatypeToObject)
UNIMPL(InOutToPointer)
UNIMPL(ArrayToPointer)
UNIMPL(StringToPointer)
UNIMPL(PointerToPointer)
UNIMPL(LValueToPointer)
UNIMPL(ForeignObjectConversion)
UNIMPL(UnevaluatedInstance)
UNIMPL(ForcedCheckedCast)
UNIMPL(ConditionalCheckedCast)
UNIMPL(Is)
UNIMPL(Coerce)
UNIMPL(Arrow)
UNIMPL(EnumIsCase)
UNIMPL(DefaultValue)
UNIMPL(CodeCompletion)
UNIMPL(UnresolvedPattern)
UNIMPL(EditorPlaceholder)
UNIMPL(ObjCSelector)
UNIMPL(ObjCKeyPath)
#undef UNIMPL

