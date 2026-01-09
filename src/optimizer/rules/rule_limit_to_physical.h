#ifndef PG_CARBON_RULE_LIMIT_TO_PHYSICAL_H
#define PG_CARBON_RULE_LIMIT_TO_PHYSICAL_H

#include "rule.h"

namespace pg_carbon {

class RuleLimitToPhysical : public Rule {
public:
    RuleLimitToPhysical();
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleLimitToPhysical"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_RULE_LIMIT_TO_PHYSICAL_H
