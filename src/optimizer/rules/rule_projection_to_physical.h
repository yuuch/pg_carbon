#ifndef PG_CARBON_RULE_PROJECTION_TO_PHYSICAL_H
#define PG_CARBON_RULE_PROJECTION_TO_PHYSICAL_H

#include "rule.h"

namespace pg_carbon {

class RuleProjectionToPhysical : public Rule {
public:
    RuleProjectionToPhysical();
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleProjectionToPhysical"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_RULE_PROJECTION_TO_PHYSICAL_H
