#ifndef SWEETJS_TRANSLATOR_H
#define SWEETJS_TRANSLATOR_H

#include <string>
#include <vector>
#include <initializer_list>
#include <functional>

template <typename F, typename X,
typename Y = typename std::result_of<F(X)>::type>
std::vector<Y> fmap(F&& f, const std::vector<X>& xs) {
    std::vector<Y> ys(xs.size());
    std::transform(xs.begin(), xs.end(), ys.begin(), f);
    return ys;
}

template <typename F, typename X>
std::vector<X> filter(F&& check, const std::vector<X>& xs) {
    std::vector<X> xs_;
    for (const auto& x : xs) {
        if (check(x))
            xs_.push_back(x);
    }
    return xs_;
}

template <typename X, typename Y,
typename Z = std::pair<X, Y>>
std::vector<Z> zip(const std::vector<X>& xs, const std::vector<Y>& ys) {
    std::vector<Z> zs(xs.size());
    std::transform(xs.begin(), xs.end(), ys.begin(), zs.begin(),
              [](const X& x, const Y& y){ return std::make_pair(x, y); });
    return zs;
}

template <typename F, typename X, typename Y,
typename Z = typename std::result_of<F(X, Y)>::type>
std::vector<Z> unzip(F&& op, const std::vector<std::pair<X, Y>>& xys) {
    return fmap([&](std::pair<X, Y> pair) {
        return op(pair.first, pair.second);
    }, xys);
}

template <typename ListT>
std::string sepBy(const std::string& sep, const ListT& list) {
    if (list.size() == 0)
        return "";
    
    size_t bufSize = 0;
    for (const auto &x : list)
        bufSize += x.size();
    bufSize += (list.size() - 1) * sep.size();
    
    std::string sum;
    sum.reserve(bufSize);
    
    for (auto i = list.begin(); true; ) {
        sum.append(*i);
        ++i;
        if (i == list.end())
            return sum;
        sum.append(sep);
    }
    return "placeholder";
}

std::string fillTemplate(const std::string& template_,
                         std::initializer_list<std::string> list);

#include "TranslateDecl.h"
#include "TranslateExpr.h"
#include "TranslatePattern.h"
#include "TranslateStmt.h"

#endif
