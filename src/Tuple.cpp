#include <functional>
using namespace std;

#include "Tuple.h"
using namespace swift;

#include "Translator.h"

inline string identifierToString (Identifier id) {
    return id.get() ? id.get() : string{};
}

bool isArrayRepTuple(ArrayRef<Identifier> labels) {
    return isArrayRepTuple(labels.vec());
}

bool isArrayRepTuple(vector<Identifier> labels) {
    return isArrayRepTuple(fmap(identifierToString, labels));
}

bool isArrayRepTuple(vector<string> labels) {
    for (auto label : labels) {
        if (!label.empty())
            return false;
    }
    return true;
}

void genericRenameNames
(vector<string>& names, function<string(int)>&& rename) {
    for (size_t i = 0; i < names.size(); ++i) {
        if (names[i].empty())
            names[i] = rename(i);
    }
}

vector<string> completeObjRepTupleLabels(ArrayRef<Identifier> labels) {
    auto labels_ = fmap(identifierToString, labels.vec());
    genericRenameNames(labels_, [](size_t i) { return "$" + to_string(i); });
    return labels_;
}
