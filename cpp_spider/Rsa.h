//
// Created by rustam_t on 11/2/15.
//

#ifndef CPP_BABEL_RSA_H
#define CPP_BABEL_RSA_H

#ifdef _WIN32
# include <WinSock2.h>
#endif

#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <iostream>
#include <string>
#include <vector>
#include "BBException.hpp"

class Rsa
{
public:
    //new Rsa
    Rsa();
    //from public key
    Rsa(std::vector<unsigned char> &public_key);

    ~Rsa();

    std::vector<unsigned char> encrypt(std::vector<unsigned char> data);
    std::vector<unsigned char> decrypt(std::vector<unsigned char> data);

    std::vector<unsigned char> &getPublicKey();
private:

    RSA *_rsa;
    int _rsa_size;

    std::vector<unsigned char> _pub_key;
    std::vector<unsigned char> _priv_key;
};

#endif //CPP_BABEL_RSA_H
