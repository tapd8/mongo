#define MONGO_LOG_DEFAULT_COMPONENT ::mongo::logger::LogComponent::kStorage

#include "mongo/platform/basic.h"

#include "mongo/db/fts/fts_chinese_tokenizer.h"

#include "mongo/db/fts/fts_query_impl.h"
#include "mongo/db/fts/fts_spec.h"
#include "mongo/db/fts/stemmer.h"
#include "mongo/db/fts/stop_words.h"
#include "mongo/db/fts/tokenizer.h"
#include "mongo/db/server_options.h"
#include "mongo/stdx/memory.h"
#include "mongo/util/mongoutils/str.h"
#include "mongo/util/stringutils.h"
#include "mongo/util/log.h"

namespace mongo {
namespace fts {

ChineseFTSTokenizer::ChineseFTSTokenizer(const FTSLanguage* language)
	: _language(language) {}

void ChineseFTSTokenizer::reset(StringData document, Options options) {
	(void)options;
	_document = document.toString();
	_words = split(_document);
	_stem = "";
}

std::list<std::u16string> ChineseFTSTokenizer::split(const StringData& doc) {
	auto func = []() -> std::unique_ptr<MMSeg> {
            auto p = std::make_unique<MMSeg>();
	    std::string wordpath = serverGlobalParams.ftsDictDir + "/words.dic";
	    std::string charpath = serverGlobalParams.ftsDictDir + "/chars.dic";
	    int ret = p->load(wordpath, charpath);
            if (ret != 0) {
	        LOG(0) << "Error: load cn fts dict failed";
                return nullptr;
            }
            return std::move(p);
	};
        static std::unique_ptr<MMSeg> mmseg = func();
        if (mmseg == nullptr) {
            mmseg = func();
        }
	std::list<std::u16string> result;
        if (mmseg == nullptr) {
	    LOG(0) << "Warning: no dict available, not tokenized";
            return result;
        }
	auto s = MMSeg::from_utf8(doc.toString());
	LOG(0) << "StringData:" << doc.toString();
	for (const auto& v : mmseg->segment(s)) {
		const auto& tmp = MMSeg::to_utf8(v);
		LOG(0) << "ChineseFTSTokenizer:" << tmp;
		if (tmp == "\n" || tmp == "\t" || tmp == "\r" || tmp == "\f" || tmp == "\v" || tmp == " ") {
			continue;
		}
		result.push_back(v);
	}
	return result;
}

bool ChineseFTSTokenizer::moveNext() {
	if (_words.size() == 0) {
		_stem = "";
		return false;
	}
	_stem = MMSeg::to_utf8(*(_words.begin()));
	_words.pop_front();
	return true;
}

StringData ChineseFTSTokenizer::get() const {
	return _stem;
}

}  // namespace fts
}  // namespace mongo

