#include <string>
#include <map>
#include <flutter/standard_method_codec.h>

using std::string;
using std::map;

#include "../../mupdf/fitz.h"
#include "../utility/global.h";
using namespace global;

namespace {
    class Document
    {
        private:
            /* data */
            string id;
            fz_document *renderer;
            fz_stream *stream;
            const unsigned char *cdata;
        public:
            Document();
            Document(string id, fz_document *renderer);
            Document(string id, fz_document *renderer, fz_stream *stream, const unsigned char *cdata);
            ~Document();
            int pageCount();
            flutter::EncodableMap infoMap();
            fz_page *openPage(const int pageNumber);
    };

    int Document::pageCount() {
        int pagecount = fz_count_pages(ctx, renderer);
        return pagecount;
    }

    flutter::EncodableMap Document::infoMap() {
        flutter::EncodableMap map;
        
        map.insert(std::pair<string, string>("id", id));
        map.insert(std::pair<string, int>("pagesCount", pageCount()));

        return map;
    }

    fz_page *Document::openPage(const int pageNumber) {
        fz_try(ctx) {
            fz_page *page = fz_load_page(ctx, renderer, pageNumber - 1);
            return page;
        }
        fz_catch(ctx) {
        }

        return NULL;
    }

    Document::Document(){}
    
    Document::Document(string id, fz_document *renderer)
    {
        this->id = id;
        this->renderer = renderer;
    }

    Document::Document(string id, fz_document *renderer, fz_stream *stream, const unsigned char *cdata)
    {
        this->id = id;
        this->renderer = renderer;
        this->stream = stream;
        this->cdata = cdata;
    }
    
    Document::~Document()
    {
    }
}