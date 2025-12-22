#include "rules.h"
#include "../../operators/operators.h"

namespace pg_carbon {

// --- RuleGetToScan ---
// Pattern: LogicalGet
Expression *RuleGetToScan::GetPattern() const {
    if (pattern_ == nullptr) {
        // Pattern: LogicalGet (Leaf)
        pattern_ = new Expression(new LogicalGet(0, 0));
    }
    return pattern_;
}

bool RuleGetToScan::Matches(Expression *expr) const {
    return expr->GetOperator()->IsLogical() && dynamic_cast<LogicalGet *>(expr->GetOperator());
}

PgVector<Expression *> RuleGetToScan::Transform(Expression *expr) const {
    auto logical_get = dynamic_cast<LogicalGet *>(expr->GetOperator());
    auto physical_scan =
        new PhysicalTableScan(logical_get->GetTableOid(), logical_get->GetRtIndex());
    // Transform to PhysicalTableScan (Leaf)
    auto result_expr = new Expression(physical_scan, {});

    PgVector<Expression *> result;
    result.push_back(result_expr);
    return result;
}

// --- RuleFilterToPhysical ---
// Pattern: LogicalFilter -> Any
Expression *RuleFilterToPhysical::GetPattern() const {
    if (pattern_ == nullptr) {
        auto *child = new Expression(new PatternLeaf());
        pattern_ = new Expression(new LogicalFilter(nullptr), {child});
    }
    return pattern_;
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

// --- RuleProjectionToPhysical ---
// Pattern: LogicalProjection -> Any
Expression *RuleProjectionToPhysical::GetPattern() const {
    if (pattern_ == nullptr) {
        auto *child = new Expression(new PatternLeaf());
        pattern_ = new Expression(new LogicalProjection(nullptr), {child});
    }
    return pattern_;
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

// --- RuleLimitToPhysical ---
// Pattern: LogicalLimit -> Any
Expression *RuleLimitToPhysical::GetPattern() const {
    if (pattern_ == nullptr) {
        auto *child = new Expression(new PatternLeaf());
        pattern_ = new Expression(new LogicalLimit(nullptr, nullptr), {child});
    }
    return pattern_;
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
