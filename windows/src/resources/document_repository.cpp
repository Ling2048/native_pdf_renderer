//#include "repository.h";
#include "../document/document.cpp";
#include "../utility/uuid.hpp";

namespace {
    class DocumentRepository : public Repository<Document>
    {
        private:
            /* data */
        public:
            DocumentRepository(/* args */);
            ~DocumentRepository();
            Document registerEx(fz_document *renderer);
            Document registerEx(fz_document *renderer, fz_stream *stream, const unsigned char *cdata);
    };
    
    DocumentRepository::DocumentRepository(/* args */)
    {
    }
    
    DocumentRepository::~DocumentRepository()
    {
    }
    
    Document DocumentRepository::registerEx(fz_document *renderer) 
    {
        std::string id = utility::uuid::generate();
        Document doc = Document(id, renderer);
        set(id, doc);
        return doc;
    }

    Document DocumentRepository::registerEx(fz_document *renderer, fz_stream *stream, const unsigned char *cdata) 
    {
        std::string id = utility::uuid::generate();
        Document doc = Document(id, renderer, stream, cdata);
        set(id, doc);
        return doc;
    }
}