//
// Created by rustam_t on 10/6/15.
//

#if defined(_WIN32) && !defined(WIN32)
# define _WINSOCKAPI_
# include <windows.h>
# define WIN32
#endif

#ifndef PROJECT2_MUTEXVAULT_HPP
# define PROJECT2_MUTEXVAULT_HPP

# include <vector>
# include <map>
# include <string>
# include <sstream>
# include "IMutex.hpp"

class MutexVault
{
public:

    static const unsigned long int bad_ptr;

    virtual ~MutexVault();

    template <typename T>
    static std::string toString(T something)
    {
        std::ostringstream ss;

        ss << something;
        return ss.str();
    }

    static MutexVault *getMutexVault();

    static bool isBadPtr(void *ptr);

    IMutex *operator[](unsigned int index);

    IMutex *operator[](const std::string &index);

    void push_back(IMutex *new_element);

    void remove(unsigned int index);

protected:

    MutexVault();

    std::vector<IMutex *> _mutex_vault;
    std::map<std::string, IMutex *> _mutex_vault_map;
};

#endif //PROJECT2_MUTEXVAULT_HPP
