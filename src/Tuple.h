#ifndef SWEETJS_TUPLE_H
#define SWEETJS_TUPLE_H

#include <string>
#include <vector>
#include "swift/AST/AST.h"

bool isArrayRepTuple(llvm::ArrayRef<swift::Identifier> labels);
bool isArrayRepTuple(std::vector<swift::Identifier> labels);
bool isArrayRepTuple(std::vector<std::string> labels);

void genericRenameNames
(std::vector<std::string>& names, std::function<std::string(int)>&& rename);

std::vector<std::string>
completeObjRepTupleLabels(llvm::ArrayRef<swift::Identifier> labels);

#endif
