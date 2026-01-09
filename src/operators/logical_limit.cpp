#include "logical_limit.h"
#include "../optimizer/memo/memo.h"

namespace pg_carbon {

LogicalProperties *LogicalLimit::DeriveLogicalProps(Memo *memo,
                                                    const PgVector<Group *> &input_groups) const {
    // Limit: Preserves input columns.
    if (input_groups.empty())
        return new LogicalProperties(ColSet(), 0.0);

    Group *child = input_groups[0];
    if (child && child->GetLogicalProperties()) {
        const auto *child_props = child->GetLogicalProperties();
        // Limit changes cardinality
        double cardinality = 10.0; // Mock limit
        return new LogicalProperties(ColSet(child_props->GetOutputColumns()), cardinality);
    }
    return new LogicalProperties(ColSet(), 0.0);
}

} // namespace pg_carbon
