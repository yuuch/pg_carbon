#ifndef PG_CARBON_OPERATORS_H
#define PG_CARBON_OPERATORS_H

#include "../common/memory.h"
#include <string>
#include <vector>

extern "C" {
#include "nodes/parsenodes.h"
#include "nodes/pg_list.h"
#include "postgres.h"
}

#include <iostream>

namespace pg_carbon {
class Group;
class Memo;
class LogicalProperties;

class Operator : public PgObject {
public:
  virtual ~Operator() = default;
  virtual bool IsLogical() const = 0;
  virtual bool IsPhysical() const = 0;
  virtual std::string ToString() const = 0;

  void AddInput(Operator *input) { inputs_.push_back(input); }
  const PgVector<Operator *> &GetInputs() const { return inputs_; }

  virtual void Print(std::ostream &os, int level = 0) const {
    for (int i = 0; i < level; ++i) {
      os << "  ";
    }
    if (level > 0) {
      os << "-> ";
    }
    os << ToString() << "\n";
    for (const auto *input : inputs_) {
      input->Print(os, level + 1);
    }
  }

private:
  PgVector<Operator *> inputs_;
};

class LogicalOperator : public Operator {
public:
  bool IsLogical() const override { return true; }
  bool IsPhysical() const override { return false; }

  // Key method for Logical Property Derivation
  // Key method for Logical Property Derivation
  virtual LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const = 0;
};

class PhysicalOperator : public Operator {
public:
  bool IsLogical() const override { return false; }
  bool IsPhysical() const override { return true; }
};

// --- Logical Operators ---

class LogicalGet : public LogicalOperator {
public:
  LogicalGet(Oid table_oid, Index rtindex)
      : table_oid_(table_oid), rtindex_(rtindex) {}

  std::string ToString() const override {
    return "LogicalGet(" + std::to_string(table_oid_) + ")";
  }
  Oid GetTableOid() const { return table_oid_; }
  Index GetRtIndex() const { return rtindex_; }

  LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const override;

private:
  Oid table_oid_;
  Index rtindex_;
};

class LogicalInnerJoin : public LogicalOperator {
public:
  LogicalInnerJoin() = default;

  std::string ToString() const override { return "LogicalInnerJoin"; }

  LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const override;
};

class LogicalFilter : public LogicalOperator {
public:
  explicit LogicalFilter(Node *qual) : qual_(qual) {}

  std::string ToString() const override { return "LogicalFilter"; }
  Node *GetQual() const { return qual_; }

  LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const override;

private:
  Node *qual_;
};

class LogicalProjection : public LogicalOperator {
public:
  explicit LogicalProjection(List *target_list) : target_list_(target_list) {}

  std::string ToString() const override { return "LogicalProjection"; }
  List *GetTargetList() const { return target_list_; }

  LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const override;

private:
  List *target_list_;
};

class LogicalSort : public LogicalOperator {
public:
  explicit LogicalSort(List *sort_clause) : sort_clause_(sort_clause) {}

  std::string ToString() const override { return "LogicalSort"; }
  List *GetSortClause() const { return sort_clause_; }

  LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const override;

private:
  List *sort_clause_;
};

class LogicalLimit : public LogicalOperator {
public:
  LogicalLimit(Node *limit_offset, Node *limit_count)
      : limit_offset_(limit_offset), limit_count_(limit_count) {}

  std::string ToString() const override { return "LogicalLimit"; }
  Node *GetLimitOffset() const { return limit_offset_; }
  Node *GetLimitCount() const { return limit_count_; }

  LogicalProperties *
  DeriveLogicalProps(Memo *memo,
                     const PgVector<Group *> &input_groups) const override;

private:
  Node *limit_offset_;
  Node *limit_count_;
};

// --- Physical Operators ---

class PhysicalTableScan : public PhysicalOperator {
public:
  PhysicalTableScan(Oid table_oid, Index rtindex)
      : table_oid_(table_oid), rtindex_(rtindex) {}

  std::string ToString() const override {
    return "PhysicalTableScan(" + std::to_string(table_oid_) + ")";
  }
  Oid GetTableOid() const { return table_oid_; }
  Index GetRtIndex() const { return rtindex_; }

private:
  Oid table_oid_;
  Index rtindex_;
};

class PhysicalNestedLoopJoin : public PhysicalOperator {
public:
  PhysicalNestedLoopJoin() = default;

  std::string ToString() const override { return "PhysicalNestedLoopJoin"; }
};

class PhysicalFilter : public PhysicalOperator {
public:
  explicit PhysicalFilter(Node *qual) : qual_(qual) {}

  std::string ToString() const override { return "PhysicalFilter"; }
  Node *GetQual() const { return qual_; }

private:
  Node *qual_;
};

class PhysicalProjection : public PhysicalOperator {
public:
  explicit PhysicalProjection(List *target_list) : target_list_(target_list) {}

  std::string ToString() const override { return "PhysicalProjection"; }
  List *GetTargetList() const { return target_list_; }

private:
  List *target_list_;
};

class PhysicalSort : public PhysicalOperator {
public:
  explicit PhysicalSort(List *sort_clause) : sort_clause_(sort_clause) {}

  std::string ToString() const override { return "PhysicalSort"; }
  List *GetSortClause() const { return sort_clause_; }

private:
  List *sort_clause_;
};

class PhysicalAggregate : public PhysicalOperator {
public:
  PhysicalAggregate(List *group_clause, Node *having_qual)
      : group_clause_(group_clause), having_qual_(having_qual) {}

  std::string ToString() const override { return "PhysicalAggregate"; }
  List *GetGroupClause() const { return group_clause_; }
  Node *GetHavingQual() const { return having_qual_; }

private:
  List *group_clause_;
  Node *having_qual_;
};

class PhysicalLimit : public PhysicalOperator {
public:
  PhysicalLimit(Node *limit_offset, Node *limit_count)
      : limit_offset_(limit_offset), limit_count_(limit_count) {}

  std::string ToString() const override { return "PhysicalLimit"; }
  Node *GetLimitOffset() const { return limit_offset_; }
  Node *GetLimitCount() const { return limit_count_; }

private:
  Node *limit_offset_;
  Node *limit_count_;
};

} // namespace pg_carbon

#endif // PG_CARBON_OPERATORS_H
