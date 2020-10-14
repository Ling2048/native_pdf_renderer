//#include "repository.h";
#include "../document/page.cpp";
#include "../utility/uuid.hpp";

namespace {
    class PageRepository : public Repository<Page>
    {
        private:
            /* data */
        public:
            PageRepository(/* args */);
            ~PageRepository();
            Page registerEx(string documentId, fz_page *renderer);
    };
    
    PageRepository::PageRepository(/* args */)
    {
    }
    
    PageRepository::~PageRepository()
    {
    }

    Page PageRepository::registerEx(string documentId, fz_page *renderer) {
        std::string id = utility::uuid::generate();
        Page page = Page(id, documentId, renderer);
        set(id, page);
        return page;
    }
    
}