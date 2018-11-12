#pragma once

#include "mongo/base/disallow_copying.h"
#include "mongo/base/string_data.h"
#include "mongo/db/fts/fts_tokenizer.h"
#include "mongo/db/fts/stemmer.h"
#include "mongo/db/fts/tokenizer.h"
#include "mongo/db/fts/unicode/string.h"
#include "mongo/db/fts/jieba/Jieba.hpp"

namespace mongo {
namespace fts {

class FTSLanguage;
class StopWords;

class ChineseFTSTokenizer final : public FTSTokenizer {
    MONGO_DISALLOW_COPYING(ChineseFTSTokenizer);

public:
    ChineseFTSTokenizer(const FTSLanguage* language);

    void reset(StringData document, Options options) override;

    bool moveNext() override;

    StringData get() const override;

private:
    std::list<std::string> split(const StringData& doc);
    const FTSLanguage* const _language;

    // const StopWords* const _stopWords;
    // const unicode::DelimiterListLanguage _delimListLanguage;
    // const unicode::CaseFoldMode _caseFoldMode;

    std::string _document;
    std::string _stem;
    std::list<std::string> _words;
};

}  // namespace fts
}  // namespace mongo
