#include "logical_inner_join.h"
#include "../optimizer/memo/memo.h"

namespace pg_carbon {

LogicalProperties *
LogicalInnerJoin::DeriveLogicalProps(Memo *memo, const PgVector<Group *> &input_groups) const {
    // Join: Union of child output columns.
    ColSet output_columns;
    double cardinality = 1000.0;

    for (Group *child_group : input_groups) {
        if (!child_group)
            continue;

        LogicalProperties *child_props = child_group->GetLogicalProperties();
        if (child_props) {
            output_columns.Union(child_props->GetOutputColumns());
        }
    }

    return new LogicalProperties(std::move(output_columns), cardinality);
}

} // namespace pg_carbon
