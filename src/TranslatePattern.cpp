#include "Translator.h"
using namespace std;
using namespace swift;

#include "Tuple.h"

void TranslatePattern::translateTotalPatternBindingDecl
(PatternBindingDecl *decl, std::vector<VarDecl*> varDecls) {
    bool noNewVar = varDecls.size() == 0;
    string modifier;
    if (!noNewVar)
        modifier = varDecls.front()->isLet() ? "const" : "let";
    
    for (auto entry : decl->getPatternList()) {
        auto pattern = entry.getPattern();
        auto rhs = TranslateExpr().visit(entry.getInit());
        out.indent(indent);
        if (dyn_cast<AnyPattern>(pattern))
            out << fillTemplate("var $_ = @;\n", {rhs});
        else {
            out << fillTemplate("@ @ = @;\n", {
                modifier,
                translateTotalPattern(pattern),
                rhs
            });
        }
    }
}

string TranslatePattern::translateTotalPattern(Pattern *pattern) {
    if (auto namedP = dyn_cast<NamedPattern>(pattern)) {
        return namedP->getNameStr();
    }
    if (dyn_cast<AnyPattern>(pattern))
        return "";

    auto tupleP = dyn_cast<TuplePattern>(pattern);
    if (tupleP == nullptr)
        assert(false && "unhandled case when translating total patterns");
    
    vector<string> labels;
    vector<string> patterns;
    for (auto element : tupleP->getElements()) {
        auto labelPtr = element.getLabel().get();
        labels.push_back(labelPtr ? labelPtr : string{});
        patterns.push_back(translateTotalPattern(element.getPattern()));
    }

    auto isRepArray = isArrayRepTuple(labels);
    if (isRepArray) {
        return fillTemplate("[@]", {sepBy(", ", patterns)});
    } else {
        auto check = [](const pair<string, string>& x) {
            return !x.second.empty();
        };
        auto sum = filter(check, zip(labels, patterns));
        auto join = [](const string& label, const string& param) {
            return fillTemplate("@: @", {label, param});
        };
        return fillTemplate("{@}", {sepBy(", ", unzip(join, sum))});
    }
}
