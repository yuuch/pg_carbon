#ifndef PG_CARBON_LOGICAL_PROJECTION_H
#define PG_CARBON_LOGICAL_PROJECTION_H

#include "operator.h"

namespace pg_carbon {

class LogicalProjection : public LogicalOperator {
public:
    explicit LogicalProjection(List *target_list) : target_list_(target_list) {}

    std::string ToString() const override { return "LogicalProjection"; }
    List *GetTargetList() const { return target_list_; }

    LogicalProperties *DeriveLogicalProps(Memo *memo,
                                          const PgVector<Group *> &input_groups) const override;

private:
    List *target_list_;
};

} // namespace pg_carbon

#endif // PG_CARBON_LOGICAL_PROJECTION_H
