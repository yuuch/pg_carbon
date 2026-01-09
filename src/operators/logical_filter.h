#ifndef PG_CARBON_LOGICAL_FILTER_H
#define PG_CARBON_LOGICAL_FILTER_H

#include "operator.h"

namespace pg_carbon {

class LogicalFilter : public LogicalOperator {
public:
    explicit LogicalFilter(Node *qual) : qual_(qual) {}

    std::string ToString() const override { return "LogicalFilter"; }
    Node *GetQual() const { return qual_; }

    LogicalProperties *DeriveLogicalProps(Memo *memo,
                                          const PgVector<Group *> &input_groups) const override;

private:
    Node *qual_;
};

} // namespace pg_carbon

#endif // PG_CARBON_LOGICAL_FILTER_H
