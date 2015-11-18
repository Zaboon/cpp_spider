//
// Created by rustam_t on 10/13/15.
//

#ifndef CPP_BABEL_PACKET_H
#define CPP_BABEL_PACKET_H

#if defined(_WIN32) && !defined(WIN32)
# define _WINSOCKAPI_
# include <windows.h>
# define WIN32
#endif

#include <string>
#include <vector>
#include <typeinfo>
#include "Rsa.h"
//#include "Identity.hpp"
//#include "Sound/SoundManager.hpp"

#define _MAGIC_ 0x0101010

class Packet
{
public:

    virtual ~Packet() {};

    enum Type {

        String = 0xb16b00b5,
        IntVector = 0x42424242,
        SSLPublicKey = 0x4ab2321a,
    };

    //object specific constructors
    //Packet(Instruct inst);
    //Packet(Identity& id);
    Packet(std::string &str);
    Packet(std::vector<int> &vec);
    Packet(Rsa &);
    //Packet(SoundPacket &);

    static std::vector<unsigned char>& stringToStream(char *buf, int const size)
    {
        std::vector<unsigned char>* stream = new std::vector<unsigned char>();

        for (int i=0; i < size; ++i)
        {
            stream->push_back(buf[i]);
        }
        return (*stream);
    }

    static void streamToString(std::vector<unsigned char>* stream, unsigned char* data)
    {
        for (unsigned int i=0; i < stream->size(); ++i)
        {
            data[i] = (*stream)[i];
        }
    }

    template <typename T>
    static Packet *pack(T &obj)
    {
        return (new Packet(obj));
    };

	static int extractSizeFromHeader(std::vector<unsigned char> &data)
	{
		unsigned int headerSize = Packet::getHeaderSize();
		if (data.size() <= headerSize)
			return (-1);

		unsigned int *r_magic = reinterpret_cast<unsigned int *>(&data[0]);
		Packet::Type *r_type = reinterpret_cast<Packet::Type *>(&data[sizeof(unsigned int)]);
		unsigned int *r_size = reinterpret_cast<unsigned int *>(&data[sizeof(unsigned int) * 2]);
		unsigned int *r_encrypted = reinterpret_cast<unsigned int *>(&data[sizeof(unsigned int) * 3]);

		if (*r_magic != _MAGIC_ || *r_size + headerSize > data.size() || !(*r_encrypted == 0 || *r_encrypted == 1))
			return (-1);
		return (*r_size);
	}

    //build a bytestream from the packet
    std::vector<unsigned char>              *build(Rsa *rsa = NULL);

    //reconstruct the packet from vector stream which will be consumed if succeeded
    static Packet                           *fromStream(std::vector<unsigned char> &data, Rsa *rsa = NULL);

    //unpack the packet to get your object back
    template <typename T>
    T *unpack(Rsa *rsa = NULL)
    {
        if (rsa != NULL && this->_encrypted) {
            this->_data = rsa->decrypt(this->_data);
            this->_encrypted = false;
        }
        if (typeid(T) == typeid(std::string))
            return (reinterpret_cast<T *>(this->getString()));
        else if (typeid(T) == typeid(std::vector<int>))
            return (reinterpret_cast<T *>(this->getIntVector()));
        else if (typeid(T) == typeid(Rsa))
            return (reinterpret_cast<T *>(this->getRsa()));
        /*else if (typeid(T) == typeid(Identity))
            return (reinterpret_cast<T *>(this->getIdentity()));
        else if (typeid(T) == typeid(SoundPacket))
            return (reinterpret_cast<T *>(this->getSound()));*/
        return (NULL);
    };

    //just 3 * (sizeof(unsigned int))
    static unsigned int                     getHeaderSize();

    Type                                    getType() const;

    bool                                    isEncrypted() const;

    std::vector<unsigned char>              &getData();

protected:

    //properties
    Type _type;
    bool _encrypted;
    std::vector<unsigned char> _data;

    //constructor
    Packet();

    //object getters
    //Identity                                *getIdentity();
    std::string                             *getString();
    //Instruct                                *getInstruction();
    std::vector<int>                        *getIntVector();
    Rsa                                     *getRsa();
    //SoundPacket                             *getSound();
};

#endif //CPP_BABEL_PACKET_H
