#include "physical_projection.h"
#include "../cost_model/cost_model.h"
#include "../metadata/metadata.h"
#include "../optimizer/memo/memo.h"

namespace pg_carbon {

double PhysicalProjection::ComputeCost(Memo * /*memo*/, const PgVector<Group *> &input_groups,
                                       GroupExpression * /*expr*/) const {
    double input_cost = 0.0;
    double rows = 0.0;

    if (!input_groups.empty()) {
        Group *child = input_groups[0];
        if (child && child->GetBestExpression())
            input_cost += child->GetBestExpression()->GetCost();
        if (child && child->GetLogicalProperties())
            rows = child->GetLogicalProperties()->GetCardinality();
    }

    pg_carbon::MetadataAccessor metadata;
    pg_carbon::CostModel cost_model(&metadata);
    return cost_model.CostProjection(rows, input_cost);
}

} // namespace pg_carbon
