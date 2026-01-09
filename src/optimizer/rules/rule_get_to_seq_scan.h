#ifndef PG_CARBON_RULE_GET_TO_SEQ_SCAN_H
#define PG_CARBON_RULE_GET_TO_SEQ_SCAN_H

#include "rule.h"

namespace pg_carbon {

class RuleGetToSeqScan : public Rule {
public:
    RuleGetToSeqScan();
    bool Matches(Expression *expr) const override;
    PgVector<Expression *> Transform(Expression *expr) const override;
    std::string ToString() const override { return "RuleGetToSeqScan"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_RULE_GET_TO_SEQ_SCAN_H
