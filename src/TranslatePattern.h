//===--- TranslatePattern.h - SweetJS Pattern Translator ------------------===//
//
// # SweetJS Pattern Translator
//
// Swift patterns have three categories:
//
// 1. total patterns that can appear everywhere.
// 2. optional patterns that appear only in conditionals and for-in loops.
// 3. partial patterns that can appear only in conditionals and switch stmts.
//
// Note that partial patterns has a non-trivial translation process, which
// renders the translation of pattern uncompositional. Thus we will not use
// a traditional visitor pattern here.
//
// ## Total Patterns
// Total patterns are used to introduce new variables. At AST level, they are
// always one or more variable declrations. There is a special limitation:
// for variables introduced in one pattern, they must share the same access
// level (``let`` or ``var``). This limitation allows a simple translation
// to ECMAScript.
//
// ## Optional Pattern

#ifndef SWEETJS_TRANSLATOR_PATTERN_H
#define SWEETJS_TRANSLATOR_PATTERN_H

#include "swift/AST/AST.h"
#include "swift/AST/ASTVisitor.h"
#include "Translator.h"

namespace swift {
    struct TranslatePattern {
        raw_ostream& out;
        raw_ostream& err;
        unsigned indent;
        
        TranslatePattern(raw_ostream &out, raw_ostream &err, unsigned indent)
        : out(out), err(err), indent(indent) {}
        
        void translateTotalPatternBindingDecl(PatternBindingDecl *pattern,
                                              std::vector<VarDecl*> varDecls);
        std::string translateTotalPattern(Pattern *pattern);
    };
}

#endif
