#include <string>
#include <vector>
using std::string;
using std::vector;
using std::pair;

#include "Translator.h"
using namespace swift;

#include "Tuple.h"

void TranslateDecl::visitFuncDecl(FuncDecl* funcDecl) {
    out.indent(indent) << "function ";
    out << funcDecl->getFullName().getBaseName().get();
    
    if (funcDecl->getNaturalArgumentCount() != 1)
        assert(false && "unexpected argument count");

    auto parameterList = funcDecl->getParameterList(0);
    vector<string> labels;
    vector<string> params;
    for (auto parameter : *parameterList) {
        auto labelPtr = parameter->getArgumentName().get();
        labels.push_back(labelPtr ? labelPtr : string{});
        params.push_back(parameter->getFullName().getBaseName().get());
    }
    bool isPassByArray = isArrayRepTuple(labels);
    if (isPassByArray) {
        genericRenameNames(params, [](size_t i) {
            return "$_" + std::to_string(i);
        });
        out << fillTemplate("(@) ", {sepBy(", ", params)});
    } else {
        auto check = [](const pair<string, string>& x) {
            return !x.first.empty();
        };
        auto sum = filter(check, zip(labels, params));
        auto join = [](const string& label, const string& param) {
            return fillTemplate("@: @", {label, param});
        };
        out << fillTemplate("({@}) ", {sepBy(", ", unzip(join, sum))});
    }
    auto body = funcDecl->getBody(/*canSynthesize=*/false);
    TranslateStmt(out, err, indent).visit(body);
    out << "\n\n";
}

void TranslateDecl::visitPatternBindingDecl(PatternBindingDecl *pbd) {
    llvm_unreachable("shouldn be handled in brace_stmt with var_decl");
}

void TranslateDecl::visitVarDecl(VarDecl *varDecl) {
    llvm_unreachable("should be handled in brace_smt with "
                     "pattern_binding_decl");
}

#define UNIMPL(NAME) \
    void TranslateDecl::visit##NAME##Decl(NAME##Decl* decl) { \
        assert(false && "visit##NAME##Decl unimplemented\n"); \
    }

UNIMPL(Import)
UNIMPL(Extension)
UNIMPL(EnumCase)
UNIMPL(TopLevelCode)
UNIMPL(IfConfig)
UNIMPL(InfixOperator)
UNIMPL(PrefixOperator)
UNIMPL(PostfixOperator)
UNIMPL(GenericTypeParam)
UNIMPL(AssociatedType)
UNIMPL(TypeAlias)
UNIMPL(Enum)
UNIMPL(Struct)
UNIMPL(Class)
UNIMPL(Protocol)
UNIMPL(Module)
UNIMPL(Param)
UNIMPL(Subscript)
UNIMPL(Constructor)
UNIMPL(Destructor)
UNIMPL(EnumElement)

#undef UNIMPL
