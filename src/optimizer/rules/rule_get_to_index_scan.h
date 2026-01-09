#ifndef PG_CARBON_RULE_GET_TO_INDEX_SCAN_H
#define PG_CARBON_RULE_GET_TO_INDEX_SCAN_H

#include "rule.h"

namespace pg_carbon {

class RuleGetToIndexScan : public Rule {
public:
    RuleGetToIndexScan();
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleGetToIndexScan"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_RULE_GET_TO_INDEX_SCAN_H
