#include "rule_filter_to_physical.h"
#include "../../operators/operators.h"

namespace pg_carbon {

// Pattern: LogicalFilter -> Any
RuleFilterToPhysical::RuleFilterToPhysical() : Rule(RULE_FILTER_TO_PHYSICAL) {
    auto *child = new Expression(new PatternLeaf());
    pattern_ = new Expression(new LogicalFilter(nullptr), {child});
}

bool RuleFilterToPhysical::Matches(Expression *expr) const {
    return expr->GetOperator()->IsLogical() && dynamic_cast<LogicalFilter *>(expr->GetOperator());
}

PgVector<Expression *> RuleFilterToPhysical::Transform(Expression *expr) const {
    auto logical = dynamic_cast<LogicalFilter *>(expr->GetOperator());
    auto physical = new PhysicalFilter(logical->GetQual());
    auto result_expr = new Expression(physical, expr->GetChildren());

    PgVector<Expression *> result;
    result.push_back(result_expr);
    return result;
}

} // namespace pg_carbon
