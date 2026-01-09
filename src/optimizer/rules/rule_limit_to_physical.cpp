#include "rule_limit_to_physical.h"
#include "../../operators/operators.h"

namespace pg_carbon {

// Pattern: LogicalLimit -> Any
RuleLimitToPhysical::RuleLimitToPhysical() : Rule(RULE_LIMIT_TO_PHYSICAL) {
    auto *child = new Expression(new PatternLeaf());
    pattern_ = new Expression(new LogicalLimit(nullptr, nullptr), {child});
}

bool RuleLimitToPhysical::Matches(Expression *expr) const {
    return expr->GetOperator()->IsLogical() && dynamic_cast<LogicalLimit *>(expr->GetOperator());
}

PgVector<Expression *> RuleLimitToPhysical::Transform(Expression *expr) const {
    auto logical = dynamic_cast<LogicalLimit *>(expr->GetOperator());
    auto physical = new PhysicalLimit(logical->GetLimitOffset(), logical->GetLimitCount());
    auto result_expr = new Expression(physical, expr->GetChildren());

    PgVector<Expression *> result;
    result.push_back(result_expr);
    return result;
}

} // namespace pg_carbon
