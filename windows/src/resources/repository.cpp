#include "repository.h"

#include <string>
#include <map>

using std::string;

// enum RepositoryError {
//     ItemNotFound
// };

namespace {
    // template<class T>
    // class Repository
    // {
    //     private:
    //         /* data */
    //         std::map<string, T> items;
    //     public:
    //         Repository(/* args */);
    //         ~Repository();
    //         T get(string id);
    //         void set(string id, T item);
    //         bool exist(string id);
    //         void close(string id);
    // };
    
    template <class T>
    Repository<T>::Repository(/* args */)
    {
    }
    
    template <class T>
    Repository<T>::~Repository()
    {
    }

    template <class T>
    T Repository<T>::get(string id) 
    {
        // if !Repository<T>::exist(id) {

        // }
        return items[id];
    }

    template <class T>
    void Repository<T>::set(string id, T item) 
    {
        items[id] = item;
    }

    template <class T>
    bool Repository<T>::exist(string id) 
    {
        return items.count(id) == 0 ? false : true;
    }

    template <class T>
    void Repository<T>::close(string id) 
    {
        items.erase(id);
    }
    
}