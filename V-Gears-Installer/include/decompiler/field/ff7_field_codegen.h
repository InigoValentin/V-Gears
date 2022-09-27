#pragma  once

#include "decompiler/decompiler_codegen.h"
#include <boost/algorithm/string.hpp>
#include <deque>
#include <unordered_map>
#include "common/make_unique.h"
#include "decompiler/sudm.h"

namespace FF7
{
    class FunctionMetaData
    {
    public:
        // Meta data format can be:
        // start_end_entityname
        // start_entityname
        // end_entity_name
        FunctionMetaData(std::string metaData)
        {
            Parse(metaData);
        }

        bool IsStart() const
        {
            return mStart;
        }

        bool IsEnd() const
        {
            return mEnd;
        }

        std::string EntityName()
        {
            return mEntityName;
        }

        int CharacterId()
        {
            return mCharacterId;
        }

    private:
        void Parse(std::string str)
        {
            std::deque<std::string> strs;
            boost::split(strs, str, boost::is_any_of("_"), boost::token_compress_on);
            if (!strs.empty())
            {
                auto tmp = strs.front();
                strs.pop_front();
                ParseStart(tmp, strs);
            }
        }

        void ParseStart(const std::string& item, std::deque<std::string>& strs)
        {
            if (item == "start")
            {
                mStart = true;
                if (!strs.empty())
                {
                    auto tmp = strs.front();
                    strs.pop_front();
                    ParseEnd(tmp, strs);
                }
            }
            else
            {
                ParseEnd(item, strs);
            }
        }

        void ParseEnd(const std::string& item, std::deque<std::string>& strs)
        {
            if (item == "end")
            {
                mEnd = true;
                if (!strs.empty())
                {
                    auto tmp = strs.front();
                    strs.pop_front();
                    ParseCharId(tmp, strs);
                }
            }
            else
            {
                ParseCharId(item, strs);
            }
        }

        void ParseCharId(const std::string& item, std::deque<std::string>& strs)
        {
            if (!item.empty())
            {
                mCharacterId = std::stoi(item);
            }
            if (!strs.empty())
            {
                auto tmp = strs.front();
                strs.pop_front();
                ParseEntity(tmp, strs);
            }
        }

        void ParseEntity(const std::string& item, std::deque<std::string>& strs)
        {
            mEntityName = item;
            for (auto& part : strs)
            {
                if (!part.empty())
                {
                    mEntityName += "_" + part;
                }
            }
        }

        bool mEnd = false;
        bool mStart = false;
        std::string mEntityName;
        int mCharacterId = -1;
    };

    class FF7CodeGenerator : public CodeGenerator
    {
    public:
        FF7CodeGenerator(Engine *engine, const InstVec& insts, std::ostream &output)
            : CodeGenerator(engine, output, kFIFOArgOrder, kLIFOArgOrder),
              mInsts(insts)
        {
            mTargetLang = std::make_unique<LuaTargetLanguage>();
        }
    protected:
        virtual std::string constructFuncSignature(const Function &func) override;
        virtual void onEndFunction(const Function& func) override;
        virtual void onBeforeStartFunction(const Function& func) override;
        virtual void onStartFunction(const Function& func) override;
        virtual bool OutputOnlyRequiredLabels() const override { return true; }
    private:
        const InstVec& mInsts;
    };


    class FF7SimpleCodeGenerator : public CodeGenerator
    {
    public:
        FF7SimpleCodeGenerator(Engine *engine, const InstVec& insts, std::ostream &output, SUDM::IScriptFormatter& formatter)
            : CodeGenerator(engine, output, kFIFOArgOrder, kLIFOArgOrder),
            mInsts(insts), mFormatter(formatter)
        {
            mTargetLang = std::make_unique<LuaTargetLanguage>();
        }
        virtual void generate(InstVec& insts, const Graph &g) override;
        virtual void addOutputLine(std::string s, bool unindentBefore = false, bool indentAfter = false) override;
        float ScaleFactor() const;
    protected:
        virtual std::string constructFuncSignature(const Function &func) override;
        virtual void onEndFunction(const Function& func) override;
        virtual void onBeforeStartFunction(const Function& func) override;
        virtual void onStartFunction(const Function& func) override;
        virtual bool OutputOnlyRequiredLabels() const override { return true; }
    private:
        const InstVec& mInsts;
        std::vector<CodeLine> mLines;
    public:
        SUDM::IScriptFormatter& mFormatter;
    };

    namespace FF7CodeGeneratorHelpers
    {
        enum struct ValueType : int
        {
            Integer = 0,
            Float = 1
        };

        const std::string FormatInstructionNotImplemented(const std::string& entity, uint32 address, uint32 opcode);
        const std::string FormatInstructionNotImplemented(const std::string& entity, uint32 address, const Instruction& instruction);
        const std::string FormatBool(uint32 value);
        const std::string FormatInvertedBool(uint32 value);
 
        template<typename TValue>

        /**
         * Formats a data access for a map script.
         *
         * If possible, it will look for friendly names for variables.
         *
         * @param formatter[in] Formatter to look up variable friendly names.
         * @param bank[in] The memory bank to use.
         * @param value_or_address[in] The value or memory address to use.
         * When bank is 0, it will be considered as a value. When bank is non
         * 0, it will be considered an address of the bank.
         * @param value_type[in] Data type to use. Used only when getting
         * a value, not a bank address.
         * @param scale[in] Scale to scale values to. Used only when using
         * float values, unused when type is integer or when using a bank
         * address.
         * @return String with the friendly variable name, value, or bank
         * address.
         */
        const std::string FormatValueOrVariable(
          SUDM::IScriptFormatter& formatter, uint32 bank, TValue valueOrAddress,
          ValueType valueType = ValueType::Integer, float scale = 1.0f
        ){
            switch (bank){
                case 0:
                    switch (valueType){
                        // TODO: check for zero
                        case ValueType::Float: return std::to_string(valueOrAddress / scale);
                        case ValueType::Integer: return std::to_string(valueOrAddress);
                        default: return std::to_string(valueOrAddress);
                    }
                case 1:
                case 2:
                case 3:
                case 13:
                case 15:
                    {
                        const auto address = static_cast<uint32>(valueOrAddress) & 0xFF;
                        const auto friendly_name = formatter.VarName(bank, valueOrAddress);
                        if (friendly_name.empty())
                            return (boost::format("FFVII.Banks[%1%][%2%]") % bank % address).str();
                        return (boost::format("FFVII.Data.%1%") % friendly_name).str();
                    }
                case 5:
                case 6:
                    {
                        const auto address = static_cast<uint32>(valueOrAddress)& 0xFF;
                        const  auto friendly_name = formatter.VarName(bank, address);
                        if (friendly_name.empty())
                            return (boost::format("FFVII.Banks[%1%][%2%]") % bank % address).str();
                        return "FFVII.Data." + friendly_name;
                    }
                default:
                    //throw UnknownBankException();
                    return (boost::format(
                      "FFVII.Banks[%1%][%2%]") % bank % (static_cast<uint32>(valueOrAddress) & 0xFF)
                    ).str();
            }
        }
    }
}
