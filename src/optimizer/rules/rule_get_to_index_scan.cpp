#include "rule_get_to_index_scan.h"
#include "../../operators/operators.h"

extern "C" {
#include "access/relation.h"
#include "access/table.h"
#include "catalog/index.h"
#include "utils/rel.h"
}

namespace pg_carbon {

// Pattern: LogicalFilter -> LogicalGet -> PhysicalIndexScan
RuleGetToIndexScan::RuleGetToIndexScan() : Rule(RULE_GET_TO_INDEX_SCAN) {
    // Pattern: LogicalFilter -> LogicalGet
    auto input_get = new Expression(new LogicalGet(0, 0));
    pattern_ = new Expression(new LogicalFilter(nullptr), {input_get});
}

bool RuleGetToIndexScan::Matches(Expression *expr) const {
    // Top is Filter
    if (!expr->GetOperator()->IsLogical() || !dynamic_cast<LogicalFilter *>(expr->GetOperator())) {
        return false;
    }
    // Child is Get
    if (expr->GetChildren().empty())
        return false;
    auto child = expr->GetChildren()[0];
    return child->GetOperator()->IsLogical() && dynamic_cast<LogicalGet *>(child->GetOperator());
}

PgVector<Expression *> RuleGetToIndexScan::Transform(Expression *expr) const {
    auto logical_filter = dynamic_cast<LogicalFilter *>(expr->GetOperator());
    auto logical_get = dynamic_cast<LogicalGet *>(expr->GetChildren()[0]->GetOperator());
    PgVector<Expression *> result;

    // Check if index exists for the table
    // We need to open the relation to get the index list
    Relation rel = table_open(logical_get->GetTableOid(), AccessShareLock);

    // Get list of index OIDs
    List *index_list = RelationGetIndexList(rel);

    if (index_list != NIL) {
        // Simple logic: Pick first index.
        // In reality, check if filter fields match index columns.
        Oid index_oid = linitial_oid(index_list);

        // Create PhysicalIndexScan WITH QUALS from Filter
        // We pass the filter's qual to the IndexScan
        // Note: Ideally we split quals into IndexCond and Filter.
        // For this demo, we assume all quals are indexable if we pick an index.
        auto physical_scan =
            new PhysicalIndexScan(logical_get->GetTableOid(), logical_get->GetRtIndex(), index_oid,
                                  logical_filter->GetQual());

        // Transform to PhysicalIndexScan (Leaf replacement of Filter)
        auto result_expr = new Expression(physical_scan, {});
        result.push_back(result_expr);
    }

    list_free(index_list);
    table_close(rel, AccessShareLock);

    return result;
}

} // namespace pg_carbon
