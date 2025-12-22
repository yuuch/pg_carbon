#ifndef PG_CARBON_MEMO_H
#define PG_CARBON_MEMO_H

// clang-format off
extern "C" {
#include "postgres.h"
}
// clang-format on

#include "../common/memory.h"
#include "../operators/operators.h"
#include "column.h"
#include <algorithm> // For std::min
#include <cstddef>
#include <cstdint>

namespace pg_carbon {

// Wrapper around PgVector<bool> to serve as ColSet (Decoupled from PG
// Bitmapset)
class ColSet : public PgObject {
public:
  ColSet() = default;

  void Add(int col_id) {
    if (col_id < 0)
      return;
    if (static_cast<size_t>(col_id) >= bits_.size()) {
      bits_.resize(col_id + 1, false);
    }
    bits_[col_id] = true;
  }

  void Union(const ColSet &other) {
    if (other.bits_.size() > bits_.size()) {
      bits_.resize(other.bits_.size(), false);
    }
    for (size_t i = 0; i < other.bits_.size(); ++i) {
      if (other.bits_[i]) {
        bits_[i] = true;
      }
    }
  }

  bool IsSubset(const ColSet &other) const {
    // return true if *this* is a subset of *other*
    if (bits_.size() > other.bits_.size()) {
      // If we have bits beyond other's size, check if any are set
      for (size_t i = other.bits_.size(); i < bits_.size(); ++i) {
        if (bits_[i])
          return false;
      }
    }
    size_t check_len = std::min(bits_.size(), other.bits_.size());
    for (size_t i = 0; i < check_len; ++i) {
      if (bits_[i] && !other.bits_[i])
        return false;
    }
    return true;
  }

  static ColSet MakeSingleton(int col_id) {
    ColSet s;
    s.Add(col_id);
    return s;
  }

  // Helper to iterate (if needed by implementation)
  // For now internal representation is simple vector

private:
  PgVector<bool> bits_;
};

class Group;
class Memo;

class GroupExpression : public PgObject {
public:
  GroupExpression(Operator *op, PgVector<Group *> children)
      : op_(op), children_(children), group_(nullptr) {}

  void SetGroup(Group *group) { group_ = group; }
  Group *GetGroup() const { return group_; }
  Operator *GetOperator() const { return op_; }
  const PgVector<Group *> &GetChildren() const { return children_; }

private:
  Operator *op_;
  PgVector<Group *> children_;
  Group *group_; // Back pointer to the group this expression belongs to
};

class LogicalProperties : public PgObject {
public:
  LogicalProperties(ColSet output_columns, double cardinality)
      : output_columns_(std::move(output_columns)), cardinality_(cardinality) {}

  const ColSet &GetOutputColumns() const { return output_columns_; }
  double GetCardinality() const { return cardinality_; }

private:
  ColSet output_columns_; // Schema (ColSet)
  double cardinality_;    // Statistics
};

class Group : public PgObject {
public:
  void AddExpression(GroupExpression *expr);
  const PgVector<GroupExpression *> &GetLogicalExpressions() const {
    return logical_exprs_;
  }
  const PgVector<GroupExpression *> &GetPhysicalExpressions() const {
    return physical_exprs_;
  }

  void SetExplored(bool explored) { explored_ = explored; }
  bool IsExplored() const { return explored_; }

  void SetImplemented(bool implemented) { implemented_ = implemented; }
  bool IsImplemented() const { return implemented_; }

  // For simplicity in this skeleton, we just store the best plan directly.
  // In a real optimizer, this would be a map of RequiredProperties -> Best
  // Plan.
  void SetBestExpression(GroupExpression *expr) { best_expression_ = expr; }
  GroupExpression *GetBestExpression() const { return best_expression_; }

  void SetLogicalProperties(LogicalProperties *props) {
    logical_properties_ = props;
  }
  LogicalProperties *GetLogicalProperties() const {
    return logical_properties_;
  }

private:
  PgVector<GroupExpression *> logical_exprs_;
  PgVector<GroupExpression *> physical_exprs_;
  bool explored_ = false;
  bool implemented_ = false;
  GroupExpression *best_expression_ = nullptr;
  LogicalProperties *logical_properties_ = nullptr;
};

class Memo : public PgObject {
public:
  Group *InsertExpression(GroupExpression *expr);
  Group *InitMemo(Operator *root_op);
  Group *NewGroup(LogicalProperties *props = nullptr);
  const PgVector<Group *> &GetGroups() const { return groups_; }

  int AddColumn(CarbonColumn *col) {
    col->SetId(columns_.size());
    columns_.push_back(col);
    return col->GetId();
  }

  CarbonColumn *GetColumn(int id) const {
    if (id >= 0 && static_cast<size_t>(id) < columns_.size()) {
      return columns_[id];
    }
    return nullptr;
  }

private:
  PgVector<Group *> groups_;
  PgVector<CarbonColumn *> columns_;
  // In a real implementation, we would have a hash map to detect duplicate
  // expressions.
};

} // namespace pg_carbon

#endif // PG_CARBON_MEMO_H
