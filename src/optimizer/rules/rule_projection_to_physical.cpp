#include "rule_projection_to_physical.h"
#include "../../operators/operators.h"

namespace pg_carbon {

// Pattern: LogicalProjection -> Any
RuleProjectionToPhysical::RuleProjectionToPhysical() : Rule(RULE_PROJECTION_TO_PHYSICAL) {
    auto *child = new Expression(new PatternLeaf());
    pattern_ = new Expression(new LogicalProjection(nullptr), {child});
}

bool RuleProjectionToPhysical::Matches(Expression *expr) const {
    return expr->GetOperator()->IsLogical() &&
           dynamic_cast<LogicalProjection *>(expr->GetOperator());
}

PgVector<Expression *> RuleProjectionToPhysical::Transform(Expression *expr) const {
    auto logical = dynamic_cast<LogicalProjection *>(expr->GetOperator());
    auto physical = new PhysicalProjection(logical->GetTargetList());
    auto result_expr = new Expression(physical, expr->GetChildren());

    PgVector<Expression *> result;
    result.push_back(result_expr);
    return result;
}

} // namespace pg_carbon
