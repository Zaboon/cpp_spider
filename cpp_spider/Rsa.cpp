//
// Created by rustam_t on 11/2/15.
//

#include "Rsa.h"

Rsa::Rsa()
{
    const int kBits = 2048;
    const int kExp = 65537;
    int keylen;
    char *pem_key;

    if ((this->_rsa = RSA_generate_key(kBits, kExp, 0, 0)) == NULL)
        throw BBException("Failed to generate RSA");
    this->_rsa_size = RSA_size(this->_rsa);
    /* To get the C-string PEM form: */
    BIO *bio;
    if ((bio = BIO_new(BIO_s_mem())) == NULL)
        throw BBException("Failed to init ssl");

    //get private key
    PEM_write_bio_RSAPrivateKey(bio, this->_rsa, NULL, NULL, 0, NULL, NULL);
    keylen = BIO_pending(bio);
    if ((pem_key = reinterpret_cast<char *>(calloc(keylen + 1, 1))) == NULL)
        throw BBException("Malloc failed");
    BIO_read(bio, pem_key, keylen);

    for (unsigned int i = 0; i < static_cast<unsigned int>(keylen); i++)
        this->_priv_key.push_back(pem_key[i]);
    free(pem_key);

    PEM_write_bio_RSAPublicKey(bio, this->_rsa);
    keylen = BIO_pending(bio);
    if ((pem_key = reinterpret_cast<char *>(calloc(keylen + 1, 1))) == NULL)
        throw BBException("Malloc failed");
    BIO_read(bio, pem_key, keylen);

    for (unsigned int i = 0; i < static_cast<unsigned int>(keylen); i++)
        this->_pub_key.push_back(pem_key[i]);
    BIO_free_all(bio);
    free(pem_key);
}

Rsa::Rsa(std::vector<unsigned char> &public_key)
{
    const int kBits = 2048;
    const int kExp = 65537;
    BIO *keybio;
    unsigned char *ptr;

    if ((this->_rsa = RSA_generate_key(kBits, kExp, 0, 0)) == NULL)
        throw BBException("Failed to generate RSA");
    if ((keybio = BIO_new_mem_buf(&public_key[0], public_key.size())) == NULL)
        throw BBException("Failed to init ssl");
    if ((PEM_read_bio_RSAPublicKey(keybio, &(this->_rsa), NULL, NULL)) == NULL)
        throw BBException("Failed to generated RSA");
    this->_rsa_size = RSA_size(this->_rsa);
    BIO_free_all(keybio);
}

Rsa::~Rsa()
{
    RSA_free(this->_rsa);
}

std::vector<unsigned char>
Rsa::encrypt(std::vector<unsigned char> data)
{
    std::vector<unsigned char> result;
    int res;
    int initial_length;
    int current_length;
    int flen;
    unsigned char *tmp;

    if (this->_rsa == NULL)
        return (result);
    initial_length = static_cast<int>(data.size());
    current_length = 0;
    tmp = static_cast<unsigned char *>(malloc(this->_rsa_size));
    while (initial_length > 0) {

        flen = (initial_length < this->_rsa_size - 11) ? initial_length : this->_rsa_size - 41;
        if ((res = RSA_public_encrypt(flen, &data[current_length], tmp, this->_rsa, RSA_PKCS1_PADDING)) == -1)
            return (result);
        for (unsigned int i = 0; i < static_cast<unsigned int>(res); i++)
            result.push_back(tmp[i]);
        initial_length -= res;
        current_length += res;
    }
    free(tmp);
    return (result);
}

std::vector<unsigned char>
Rsa::decrypt(std::vector<unsigned char> data)
{
    std::vector<unsigned char> result;
    int res;
    int initial_length;
    int current_length;
    int flen;
    unsigned char *tmp;

    if (this->_rsa == NULL)
        return (result);
    initial_length = static_cast<int>(data.size());
    current_length = 0;
    while (initial_length > 0) {

        tmp = static_cast<unsigned char *>(malloc(this->_rsa_size));
        if ((res = RSA_private_decrypt(this->_rsa_size, &data[current_length], tmp, this->_rsa, RSA_PKCS1_PADDING)) == -1)
            return (result);
        for (unsigned int i = 0; i < static_cast<unsigned int>(res); i++)
            result.push_back(tmp[i]);
        initial_length -= this->_rsa_size;
        current_length += this->_rsa_size;
        free(tmp);
    }
    return (result);
}

std::vector<unsigned char> &
Rsa::getPublicKey()
{
    return (this->_pub_key);
}