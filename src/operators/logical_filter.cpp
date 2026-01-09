#include "logical_filter.h"
#include "../metadata/metadata.h"
#include "../optimizer/memo/memo.h"

namespace pg_carbon {

LogicalProperties *LogicalFilter::DeriveLogicalProps(Memo *memo,
                                                     const PgVector<Group *> &input_groups) const {
    // Filter: Preserves input columns of the single child.
    if (input_groups.empty())
        return new LogicalProperties(ColSet(), 0.0);

    Group *child = input_groups[0];
    if (child && child->GetLogicalProperties()) {
        const auto *child_props = child->GetLogicalProperties();
        double rows = child_props->GetCardinality();

        double selectivity = 0.5; // Default fallback
        if (memo->GetMetadataAccessor()) {
            // Need child rte index. Currently LogicalGet stores it.
            // LogicalFilter doesn't know it directly unless we inspect child operator or props
            // track it. Simplified: Assume RTE 1 or try to find it. In a real system, we'd look up
            // which relations are in the input. For now, let's hardcode 1 as mostly single table
            // queries in demo. Use 0 or 1? `GetSelectivity` args: (quals, index). Pass 0 to
            // indicate unknown/join selectivity? Or 1.
            selectivity = memo->GetMetadataAccessor()->GetSelectivity(qual_, 1);
        }

        double cardinality = rows * selectivity;
        if (cardinality < 1.0)
            cardinality = 1.0;
        ColSet output_columns(child_props->GetOutputColumns());
        return new LogicalProperties(std::move(output_columns), cardinality);
    }
    return new LogicalProperties(ColSet(), 0.0);
}

} // namespace pg_carbon
