#ifndef PG_CARBON_OPERATOR_H
#define PG_CARBON_OPERATOR_H

#include "../common/memory.h"
#include <iostream>
#include <string>

extern "C" {
#include "nodes/parsenodes.h"
#include "nodes/pg_list.h"
#include "postgres.h"
}

namespace pg_carbon {

class Group;
class Memo;
class LogicalProperties;
class GroupExpression;

class Operator : public PgObject {
public:
    virtual ~Operator() = default;
    virtual bool IsLogical() const = 0;
    virtual bool IsPhysical() const = 0;
    virtual std::string ToString() const = 0;

    // Calculate cost based on children costs + local cost.
    virtual double ComputeCost(Memo * /*memo*/, const PgVector<Group *> & /*input_groups*/,
                               GroupExpression * /*expr*/ = nullptr) const {
        return 0.0;
    }

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
    virtual LogicalProperties *DeriveLogicalProps(Memo *memo,
                                                  const PgVector<Group *> &input_groups) const = 0;
};

class PhysicalOperator : public Operator {
public:
    bool IsLogical() const override { return false; }
    bool IsPhysical() const override { return true; }
};

class PatternLeaf : public Operator {
public:
    bool IsLogical() const override { return false; } // Pattern is neither? Or logical?
    bool IsPhysical() const override { return false; }
    std::string ToString() const override { return "PatternLeaf"; }
};

} // namespace pg_carbon

#endif // PG_CARBON_OPERATOR_H
