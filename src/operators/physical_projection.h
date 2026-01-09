#ifndef PG_CARBON_PHYSICAL_PROJECTION_H
#define PG_CARBON_PHYSICAL_PROJECTION_H

#include "operator.h"

namespace pg_carbon {

class PhysicalProjection : public PhysicalOperator {
public:
    explicit PhysicalProjection(List *target_list) : target_list_(target_list) {}

    std::string ToString() const override { return "PhysicalProjection"; }
    List *GetTargetList() const { return target_list_; }

    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;

private:
    List *target_list_;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_PROJECTION_H
