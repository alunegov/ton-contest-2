#include "SpecEntry.h"

#include <fmt/format.h>

#include "crypto/vm/boc.h"
#include "crypto/vm/dict.h"

std::string TlSpecEntry_Number::process(tonlib_api::tvm_StackEntry& stack_entry) const
{
    auto& entry = static_cast<tonlib_api::tvm_stackEntryNumber&>(stack_entry);

    return fmt::format(R"("{0}":{1})", _name, entry.number_->number_);
}

std::string TlSpecEntry_List::process(tonlib_api::tvm_StackEntry& stack_entry) const
{
    auto& entry = static_cast<tonlib_api::tvm_stackEntryList&>(stack_entry);

    std::stringstream ss;
    ss << fmt::format(R"("{}":[)", _name);
    bool first{true};
    for (const auto& el : entry.list_->elements_) {
        if (!first) {
            ss << ',';
        } else {
            first = false;
        }

        // TODO: element type as template
        auto& elv = static_cast<tonlib_api::tvm_stackEntryNumber&>(*el);
        ss << elv.number_->number_;
    }
    ss << ']';
    auto res = ss.str();

    return res;
}

std::string TlSpecEntry_Dict::process(tonlib_api::tvm_StackEntry& stack_entry) const
{
    auto& entry = static_cast<tonlib_api::tvm_stackEntryCell&>(stack_entry);

    auto key_bits{_key->bits()};

    auto cell = vm::std_boc_deserialize(entry.cell_->bytes_);
    auto dict = vm::Dictionary{cell.move_as_ok(), static_cast<int>(key_bits)};

    std::stringstream ss;
    ss << fmt::format(R"("{}":[)", _name);
    bool first{true};
    for (auto it : dict) {
        if (!first) {
            ss << ',';
        } else {
            first = false;
        }

        ss << '{';

        vm::CellBuilder cb;
        cb.store_bits(it.first, key_bits);
        auto key = cb.as_cellslice();
        ss << _key->process(key);

        ss << ',';

        auto value = it.second->clone();
        ss << _value->process(value);

        ss << '}';
    }
    ss << ']';
    auto res = ss.str();

    return res;
}

std::string TvmSpecEntry_Int::process(vm::CellSlice& cs) const
{
    auto n = cs.fetch_long(_bits);
    return fmt::format(R"("{0}":{1})", _name, n);
}

std::string TvmSpecEntry_UInt::process(vm::CellSlice& cs) const
{
    auto n = cs.fetch_ulong(_bits);
    return fmt::format(R"("{0}":{1})", _name, n);
}

std::string TvmSpecEntry_Int256::process(vm::CellSlice& cs) const
{
    auto n = cs.fetch_int256(_bits, _sgn);
    return fmt::format(R"("{0}":"{1}")", _name, n->to_hex_string());
}

uint64_t TvmSpecEntry_User::bits() const
{
    uint64_t res{0};
    for (const auto* field : _fields) {
        res += field->bits();
    }
    return res;
}

std::string TvmSpecEntry_User::process(vm::CellSlice& cs) const
{
    std::stringstream ss;
    ss << fmt::format(R"("{}":{{)", _name);
    bool first{true};
    for (const TvmSpecEntry* it : _fields) {
        if (!first) {
            ss << ',';
        } else {
            first = false;
        }

        ss << it->process(cs);
    }
    ss << '}';
    auto res = ss.str();

    return res;
}
