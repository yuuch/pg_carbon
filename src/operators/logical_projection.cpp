#include "logical_projection.h"
#include "../optimizer/memo/memo.h"

namespace pg_carbon {

LogicalProperties *
LogicalProjection::DeriveLogicalProps(Memo *memo, const PgVector<Group *> &input_groups) const {
    // Projection: Defines new output columns based on TargetList.
    ColSet output_columns;

    if (target_list_) {
        ListCell *lc;
        foreach (lc, target_list_) {
            TargetEntry *tle = (TargetEntry *)lfirst(lc);
            // Create ExprColumn for the expression in TLE
            auto *col = new ExprColumn((Node *)tle->expr);
            int col_id = memo->AddColumn(col);
            output_columns.Add(col_id);
        }
    }

    // Projection preserves cardinality
    double cardinality = 0.0;

    if (!input_groups.empty() && input_groups[0]->GetLogicalProperties()) {
        cardinality = input_groups[0]->GetLogicalProperties()->GetCardinality();
    }

    return new LogicalProperties(std::move(output_columns), cardinality);
}

} // namespace pg_carbon
