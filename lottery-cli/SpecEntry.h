#ifndef LOTTERY_CLI_SPECENTRY_H
#define LOTTERY_CLI_SPECENTRY_H

#include <auto/tl/tonlib_api.h>
#include <crypto/vm/cells/CellSlice.h>

namespace tonlib_api = ton::tonlib_api;

class TvmSpecEntry;

/*class TlSpecInEntry
{
public:
    virtual tonlib_api::tvm_StackEntry process(std::string in) const = 0;
};*/

class TlSpecEntry
{
public:
    explicit TlSpecEntry(std::string name) : _name{std::move(name)}
    {}

    virtual ~TlSpecEntry() = default;

    std::string name() const
    {
        return _name;
    }

    virtual std::string process(tonlib_api::tvm_StackEntry& stack_entry) const = 0;

protected:
    std::string _name;
};

class TlSpecEntry_Number : public TlSpecEntry
{
public:
    explicit TlSpecEntry_Number(std::string name) : TlSpecEntry{std::move(name)}
    {}

    std::string process(tonlib_api::tvm_StackEntry& stack_entry) const override;
};

class TlSpecEntry_List : public TlSpecEntry
{
public:
    explicit TlSpecEntry_List(std::string name) : TlSpecEntry{std::move(name)}
    {}

    std::string process(tonlib_api::tvm_StackEntry& stack_entry) const override;
};

class TlSpecEntry_Dict : public TlSpecEntry
{
public:
    TlSpecEntry_Dict(std::string name, TvmSpecEntry* key, TvmSpecEntry* value)
            : TlSpecEntry{std::move(name)}, _key{key}, _value{value}
    {}

    std::string process(tonlib_api::tvm_StackEntry& stack_entry) const override;

private:
    TvmSpecEntry* _key;
    TvmSpecEntry* _value;
};

class TvmSpecEntry
{
public:
    explicit TvmSpecEntry(std::string name) : _name{std::move(name)}
    {}

    virtual ~TvmSpecEntry() = default;

    virtual uint64_t bits() const = 0;

    virtual std::string process(vm::CellSlice& cs) const = 0;

protected:
    std::string _name;
};

class TvmSpecEntry_Int : public TvmSpecEntry
{
public:
    TvmSpecEntry_Int(std::string name, uint64_t bits) : TvmSpecEntry{std::move(name)}, _bits{bits}
    {}

    uint64_t bits() const override
    {
        return _bits;
    }

    std::string process(vm::CellSlice& cs) const override;

private:
    uint64_t _bits;
};

class TvmSpecEntry_UInt : public TvmSpecEntry
{
public:
    TvmSpecEntry_UInt(std::string name, uint64_t bits) : TvmSpecEntry{std::move(name)}, _bits{bits}
    {}

    uint64_t bits() const override
    {
        return _bits;
    }

    std::string process(vm::CellSlice& cs) const override;

private:
    uint64_t _bits;
};

class TvmSpecEntry_Int256 : public TvmSpecEntry
{
public:
    TvmSpecEntry_Int256(std::string name, uint64_t bits, bool sgn = false)
            : TvmSpecEntry{std::move(name)}, _bits{bits}, _sgn{sgn}
    {}

    uint64_t bits() const override
    {
        return _bits;
    }

    std::string process(vm::CellSlice& cs) const override;

private:
    uint64_t _bits;
    bool _sgn;
};

class TvmSpecEntry_User : public TvmSpecEntry
{
public:
    TvmSpecEntry_User(std::string name, std::vector<TvmSpecEntry*> fields)
            : TvmSpecEntry{std::move(name)}, _fields{std::move(fields)}
    {}

    uint64_t bits() const override;

    std::string process(vm::CellSlice& cs) const override;

private:
    std::vector<TvmSpecEntry*> _fields;
};

#endif //LOTTERY_CLI_SPECENTRY_H
