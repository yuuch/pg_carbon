#ifndef PG_CARBON_PHYSICAL_INDEX_SCAN_H
#define PG_CARBON_PHYSICAL_INDEX_SCAN_H

#include "operator.h"

namespace pg_carbon {

class PhysicalIndexScan : public PhysicalOperator {
public:
    PhysicalIndexScan(Oid table_oid, Index rtindex, Oid index_oid, Node *index_quals = nullptr)
        : table_oid_(table_oid), rtindex_(rtindex), index_oid_(index_oid),
          index_quals_(index_quals) {}

    std::string ToString() const override {
        return "PhysicalIndexScan(" + std::to_string(table_oid_) +
               ", index=" + std::to_string(index_oid_) + ")";
    }
    Oid GetTableOid() const { return table_oid_; }
    Index GetRtIndex() const { return rtindex_; }
    Oid GetIndexOid() const { return index_oid_; }
    Node *GetIndexQuals() const { return index_quals_; }

    // Override ComputeCost to return lower cost than TableScan for demo
    double ComputeCost(Memo *memo, const PgVector<Group *> &input_groups,
                       GroupExpression *expr = nullptr) const override;

private:
    Oid table_oid_;
    Index rtindex_;
    Oid index_oid_;
    Node *index_quals_ = nullptr;
};

} // namespace pg_carbon

#endif // PG_CARBON_PHYSICAL_INDEX_SCAN_H
