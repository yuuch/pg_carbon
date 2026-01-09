#include "rule_get_to_seq_scan.h"
#include "../../operators/operators.h"

namespace pg_carbon {

// Pattern: LogicalGet
RuleGetToSeqScan::RuleGetToSeqScan() : Rule(RULE_GET_TO_SEQ_SCAN) {
    // Pattern: LogicalGet (Leaf)
    pattern_ = new Expression(new LogicalGet(0, 0));
}

bool RuleGetToSeqScan::Matches(Expression *expr) const {
    return expr->GetOperator()->IsLogical() && dynamic_cast<LogicalGet *>(expr->GetOperator());
}

PgVector<Expression *> RuleGetToSeqScan::Transform(Expression *expr) const {
    auto logical_get = dynamic_cast<LogicalGet *>(expr->GetOperator());
    auto physical_scan =
        new PhysicalTableScan(logical_get->GetTableOid(), logical_get->GetRtIndex());
    // Transform to PhysicalTableScan (Leaf)
    auto result_expr = new Expression(physical_scan, {});

    PgVector<Expression *> result;
    result.push_back(result_expr);
    return result;
}

} // namespace pg_carbon
