#include "Translator.h"

std::string fillTemplate(const std::string& template_,
                         std::initializer_list<std::string> list) {
    size_t bufSize = template_.size();
    for (const auto &x : list)
        bufSize += x.size() - 1;
    std::string sum;
    sum.reserve(bufSize);
    
    auto i = list.begin();
    for (auto ch : template_) {
        if (ch != '@') {
            const char ch_[2] = {ch, 0};
            sum.append(ch_);
            continue;
        }
        if (i == list.end())
            assert(false && "size of list < num of holes in template_");
        sum.append(*i);
        ++i;
    }
    if (i != list.end())
        assert(false && "size of list > num of holes in template_");
    
    return sum;
}
