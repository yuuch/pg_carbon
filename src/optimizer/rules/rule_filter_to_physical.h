#ifndef PG_CARBON_RULE_FILTER_TO_PHYSICAL_H
#define PG_CARBON_RULE_FILTER_TO_PHYSICAL_H

#include "rule.h"

namespace pg_carbon {

class RuleFilterToPhysical : public Rule {
public:
    RuleFilterToPhysical();
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleFilterToPhysical"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_RULE_FILTER_TO_PHYSICAL_H
