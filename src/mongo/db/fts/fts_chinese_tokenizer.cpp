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
        const auto& tmp = split(_document);
        for (const auto& v : tmp) {
            _words.emplace_back(v);
        }
	_stem = "";
}

std::list<std::string> ChineseFTSTokenizer::split(const StringData& doc) {
	auto func = []() -> std::unique_ptr<cppjieba::Jieba> {
            static const std::string dict_path = serverGlobalParams.ftsDictDir + "/jieba.dict.utf8";
            static const std::string hmm_path = serverGlobalParams.ftsDictDir + "/hmm_model.utf8";
            static const std::string user_dict_path = serverGlobalParams.ftsDictDir + "/user.dict.utf8";
            static const std::string idf_path = serverGlobalParams.ftsDictDir + "/idf.utf8";
            static const std::string stop_word_path = serverGlobalParams.ftsDictDir + "/stop_words.utf8";
            auto p = std::make_unique<cppjieba::Jieba>(dict_path,
                                                       hmm_path,
                                                       user_dict_path,
                                                       idf_path,
                                                       stop_word_path);
            return std::move(p);
	};
        static std::unique_ptr<cppjieba::Jieba> seg = func();
        if (seg == nullptr) {
            seg = func();
        }
	std::vector<std::string> tmp;
        std::list<std::string> result;
        if (seg == nullptr) {
	    LOG(0) << "Warning: no dict available, not tokenized";
            return result;
        }

	LOG(0) << "StringData:" << doc.toString();
        seg->CutForSearch(doc.toString(), tmp);
	for (const auto& v : tmp) {
		LOG(0) << "ChineseFTSTokenizer:" << v;
		if (v == "\n" || v == "\t" || v == "\r" || v == "\f" || v == "\v" || v == " ") {
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
	_stem = *(_words.begin());
	_words.pop_front();
	return true;
}

StringData ChineseFTSTokenizer::get() const {
	return _stem;
}

}  // namespace fts
}  // namespace mongo

