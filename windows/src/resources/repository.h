#include <string>
#include <map>

using std::string;

// enum RepositoryError {
//     ItemNotFound
// };

namespace {
    template<class T>
    class Repository
    {
        private:
            /* data */
            std::map<string, T> items;
        public:
            Repository(/* args */);
            ~Repository();
            T get(string id);
            void set(string id, T item);
            bool exist(string id);
            void close(string id);
    };
}