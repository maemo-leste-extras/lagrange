/** @file the_Foundation/tlsrequest.c  TLS request over a Socket (using OpenSSL)

@authors Copyright (c) 2020 Jaakko Keränen <jaakko.keranen@iki.fi>

@par License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

<small>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</small>
*/

#include "the_Foundation/tlsrequest.h"
#include "the_Foundation/buffer.h"
#include "the_Foundation/socket.h"
#include "the_Foundation/stringhash.h"
#include "the_Foundation/thread.h"
#include "the_Foundation/time.h"

#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <time.h>

iDeclareType(Context)

#define DEFAULT_BUF_SIZE 8192

static iContext *context_;
static iBool isPrngSeeded_;

static void initContext_(void);
static iTlsCertificate *newX509Chain_TlsCertificate_(X509 *cert, STACK_OF(X509) *chain);
static void certificateVerifyFailed_TlsRequest_(iTlsRequest *, const iTlsCertificate *cert);

static iBool readAllFromBIO_(BIO *bio, iBlock *out) {
    char buf[DEFAULT_BUF_SIZE];
    int n;
    do {
        n = BIO_read(bio, buf, sizeof(buf));
        if (n > 0) {
            appendData_Block(out, buf, n);
        }
        else if (!BIO_should_retry(bio)) {
            return iFalse;
        }
    } while (n > 0);
    return iTrue;
}

/*----------------------------------------------------------------------------------------------*/

iDeclareClass(CachedSession)
    
static const int maxSessionAge_CachedSession_ = 10 * 60; /* seconds */
    
struct Impl_CachedSession {
    iObject          object;
    iBlock           pemSession;
    iTime            timestamp;
    iTlsCertificate *cert; /* not sent if session reused */
    iBlock           clientHash;
};

static void init_CachedSession(iCachedSession *d, SSL_SESSION *sess, const iTlsCertificate *cert) {
    BIO *buf = BIO_new(BIO_s_mem());
    PEM_write_bio_SSL_SESSION(buf, sess);
    init_Block(&d->pemSession, 0);
    readAllFromBIO_(buf, &d->pemSession);
    BIO_free(buf);
    initCurrent_Time(&d->timestamp);
    d->cert = copy_TlsCertificate(cert);
    init_Block(&d->clientHash, 0);
}

static void deinit_CachedSession(iCachedSession *d) {
    deinit_Block(&d->clientHash);
    deinit_Block(&d->pemSession);
    delete_TlsCertificate(d->cert);
}

static void setClientCertificate_CachedSession_(iCachedSession *d, const iTlsCertificate *clientCert) {
    iBlock *fp = fingerprint_TlsCertificate(clientCert);
    set_Block(&d->clientHash, fp);
    delete_Block(fp);
}

iDefineClass(CachedSession)
iDefineObjectConstructionArgs(CachedSession,
                              (SSL_SESSION *sess, const iTlsCertificate *cert),
                              sess, cert)

static void reuse_CachedSession(const iCachedSession *d, SSL *ssl) {
    BIO *buf = BIO_new_mem_buf(constData_Block(&d->pemSession), (int) size_Block(&d->pemSession));
    SSL_SESSION *sess = NULL;
    PEM_read_bio_SSL_SESSION(buf, &sess, NULL, NULL);
    if (sess) {
        SSL_SESSION_up_ref(sess);
        SSL_set_session(ssl, sess);
        SSL_SESSION_free(sess);
    }
    BIO_free(buf);
}

struct Impl_Context {
    iString               libraryName;
    SSL_CTX *             ctx;
    X509_STORE *          certStore;
    iTlsRequestVerifyFunc userVerifyFunc;
    tss_t                 tssKeyCurrentRequest;
    iMutex                cacheMutex;
    iStringHash *         cache; /* key is "address:port"; these could be saved persistently */
};

static iString *cacheKey_(const iString *host, uint16_t port) {
    iString *key = copy_String(host);
    appendFormat_String(key, ":%u", port);
    return key;
}

static iBool isExpired_CachedSession_(const iCachedSession *d) {
    if (!d) return iTrue;
    return elapsedSeconds_Time(&d->timestamp) > maxSessionAge_CachedSession_;
}

static iTlsCertificate *maybeReuseSession_Context_(iContext *d, SSL *ssl, const iString *host,
                                                   uint16_t port, const iTlsCertificate *clientCert) {
    iTlsCertificate *cert = NULL;
    iBlock *clientHash = (clientCert ? fingerprint_TlsCertificate(clientCert) : new_Block(0));
    iString *key = cacheKey_(host, port);
    lock_Mutex(&d->cacheMutex);
    /* Remove old entries from the session cache. */
    iForEach(StringHash, i, d->cache) {
        iCachedSession *cs = i.value->object;
        if (isExpired_CachedSession_(cs)) {
            iDebug("[TlsRequest] session for `%s` has expired\n", cstr_Block(&i.value->keyBlock));
            remove_StringHashIterator(&i);
        }
    }
    iCachedSession *cs = value_StringHash(d->cache, key);
    if (cs && (size_Block(&cs->clientHash) == size_Block(clientHash) &&
               cmp_Block(&cs->clientHash, clientHash) == 0)) {
        reuse_CachedSession(cs, ssl);
        cert = copy_TlsCertificate(cs->cert);
        iDebug("[TlsRequest] reusing session for `%s`\n", cstr_String(key));
    }
    unlock_Mutex(&d->cacheMutex);
    delete_String(key);
    delete_Block(clientHash);
    return cert; /* caller gets ownership */
}

static void saveSession_Context_(iContext *d, const iString *host, uint16_t port,
                                 SSL_SESSION *sess, const iTlsCertificate *serverCert,
                                 const iTlsCertificate *clientCert) {
    if (sess && serverCert) {
        iString *key = cacheKey_(host, port);
        lock_Mutex(&d->cacheMutex);
        iCachedSession *cs = new_CachedSession(sess, serverCert);
        if (clientCert) {
            setClientCertificate_CachedSession_(cs, clientCert);
        }
        insert_StringHash(d->cache, key, cs);
        unlock_Mutex(&d->cacheMutex);
        iDebug("[TlsRequest] saved session for `%s`\n", cstr_String(key));
        delete_String(key);
    }
}

static iTlsRequest *currentRequestForThread_Context_(iContext *d) {
    return tss_get(context_->tssKeyCurrentRequest);
}

static void setCurrentRequestForThread_Context_(iContext *d, iTlsRequest *request) {
    tss_set(context_->tssKeyCurrentRequest, request);
}

static int verifyCallback_Context_(int preverifyOk, X509_STORE_CTX *storeCtx) {
    if (preverifyOk) {
        return 1; /* OpenSSL says it's OK */
    }
    iAssert(context_); /* must've been initialized by now */
    iContext *d     = context_;
    const int depth = X509_STORE_CTX_get_error_depth(storeCtx);
    X509 *    x509  = X509_STORE_CTX_get_current_cert(storeCtx);
    X509_up_ref(x509); /* keep a reference */
    iTlsCertificate *cert = newX509Chain_TlsCertificate_(x509, NULL);
#if 0
    /* Debugging. */ {
        X509_NAME *name = X509_get_subject_name(x509);
        printf("[TlsRequest] verifyCallback_Context: depth=%d\n", depth);
        X509_NAME_print_ex_fp(stdout, name, 0, 0);
        printf("\n");
        iBlock *fp = publicKeyFingerprint_TlsCertificate(cert);
        printf("key fp: %s\n", cstrCollect_String(hexEncode_Block(fp)));
        delete_Block(fp);
    }
#endif
    int result = 1; /* accept everything by default */
    if (d->userVerifyFunc) {
        iTlsRequest *request = currentRequestForThread_Context_(d);
        iAssert(request != NULL);
        result = d->userVerifyFunc(request, cert, depth) ? 1 : 0;
        if (!result) {
            certificateVerifyFailed_TlsRequest_(request, cert);
        }
    }
    delete_TlsCertificate(cert); /* free the reference */
    return result;
}

void init_Context(iContext *d) {
    init_String(&d->libraryName);
#if defined (LIBRESSL_VERSION_TEXT)
    setCStr_String(&d->libraryName, "LibreSSL");
#else
    setCStr_String(&d->libraryName, "OpenSSL");
#endif
    d->tssKeyCurrentRequest = 0;
    tss_create(&d->tssKeyCurrentRequest, NULL);
#if OPENSSL_API_COMPAT >= 0x10100000L
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
    OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS, NULL);
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
#else
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ERR_load_crypto_strings();
#endif
    ERR_load_BIO_strings();
    d->ctx = SSL_CTX_new(TLS_client_method());
    if (!d->ctx) {
        iDebug("[TlsRequest] failed to initialize OpenSSL\n");
        iAssert(d->ctx);
    }
    d->certStore = NULL;
    d->userVerifyFunc = NULL;
    SSL_CTX_set_verify(d->ctx, SSL_VERIFY_PEER, verifyCallback_Context_);
    /* Bug workarounds: https://www.openssl.org/docs/manmaster/man3/SSL_CTX_set_options.html */
    SSL_CTX_set_options(d->ctx, SSL_OP_ALL);
    init_Mutex(&d->cacheMutex);
    d->cache = new_StringHash();
    SSL_CTX_set_session_cache_mode(d->ctx, SSL_SESS_CACHE_CLIENT | SSL_SESS_CACHE_NO_INTERNAL_STORE);
}

void deinit_Context(iContext *d) {
    iRelease(d->cache);
    deinit_Mutex(&d->cacheMutex);
    SSL_CTX_free(d->ctx);
    tss_delete(d->tssKeyCurrentRequest);
    deinit_String(&d->libraryName);
}

iBool isValid_Context(iContext *d) {
    return d->ctx != NULL;
}

/*----------------------------------------------------------------------------------------------*/

void setCACertificates_TlsRequest(const iString *caFile, const iString *caPath) {
    initContext_();
    iContext *d = context_;
    if (d->certStore) {
        X509_STORE_free(d->certStore);
    }
    d->certStore = X509_STORE_new();
    if (!X509_STORE_load_locations(d->certStore,
                                   isEmpty_String(caFile) ? NULL : cstr_String(caFile),
                                   isEmpty_String(caPath) ? NULL : cstr_String(caPath))) {
        iWarning("[TlsRequest] OpenSSL failed to load CA certificates\n");
    }
}

void setVerifyFunc_TlsRequest(iTlsRequestVerifyFunc verifyFunc) {
    initContext_();
    iContext *d = context_;
    d->userVerifyFunc = verifyFunc;
}

iDefineTypeConstruction(Context)

static void globalCleanup_TlsRequest_(void) {
    if (context_) {
        delete_Context(context_);
    }
}

static void initContext_(void) {
    if (!context_) {
        context_ = new_Context();
        atexit(globalCleanup_TlsRequest_);
    }
}

/*----------------------------------------------------------------------------------------------*/

struct Impl_TlsCertificate {
    X509 *cert;
    STACK_OF(X509) *chain;
    EVP_PKEY *pkey;
    enum iTlsCertificateVerifyStatus *cachedVerifyStatus; /* TODO: include domain/IP check, too? */
};

iDefineTypeConstruction(TlsCertificate)

void init_TlsCertificate(iTlsCertificate *d) {
    initContext_();
    d->cert  = NULL;
    d->chain = NULL;
    d->pkey  = NULL;
    d->cachedVerifyStatus = malloc(sizeof(*d->cachedVerifyStatus));
    *d->cachedVerifyStatus = unknown_TlsCertificateVerifyStatus;
}

static void freeX509Chain_(STACK_OF(X509) *chain) {
    for (int i = 0; i < sk_X509_num(chain); i++) {
        X509_free(sk_X509_value(chain, i));
    }
    sk_X509_free(chain);
}

void deinit_TlsCertificate(iTlsCertificate *d) {
    free(d->cachedVerifyStatus);
    if (d->cert) {
        X509_free(d->cert);
    }
    if (d->chain) {
        freeX509Chain_(d->chain);
    }
    if (d->pkey) {
        EVP_PKEY_free(d->pkey);
    }
}

static iTlsCertificate *newX509Chain_TlsCertificate_(X509 *cert, STACK_OF(X509) *chain) {
    /* Note that `chain` includes `cert`; a bit redundant. However, the chain is only
       used when verifying the `cert` via OpenSSL's API. */
    iTlsCertificate *d = new_TlsCertificate();
    d->cert  = cert;
    d->chain = chain;
    return d;
}

iTlsCertificate *newPem_TlsCertificate(const iString *pem) {
    iTlsCertificate *d = new_TlsCertificate();
    BIO *buf = BIO_new_mem_buf(cstr_String(pem), (int) size_String(pem));
    PEM_read_bio_X509(buf, &d->cert, NULL /* no passphrase callback */, "" /* empty passphrase */);
    BIO_free(buf);
    return d;
}

iTlsCertificate *newPemKey_TlsCertificate(const iString *certPem, const iString *keyPem) {
    iTlsCertificate *d = newPem_TlsCertificate(certPem);
    BIO *buf = BIO_new_mem_buf(cstr_String(keyPem), (int) size_String(keyPem));
    PEM_read_bio_PrivateKey(buf, &d->pkey, NULL, "");
    BIO_free(buf);
    return d;
}

static const iString *findName_(const iTlsCertificateName *names, enum iTlsCertificateNameType type) {
    for (; names->type; names++) {
        if (names->type == type) {
            return names->text;
        }
    }
    return NULL;
}

static void add_X509Name_(X509_NAME *name, const char *id, enum iTlsCertificateNameType type,
                          const iTlsCertificateName *names) {
    const iString *str = findName_(names, type);
    if (str) {
        X509_NAME_add_entry_by_txt(
            name, id, MBSTRING_UTF8, constData_Block(&str->chars), (int) size_String(str), -1, 0);
    }
}

static void addDomain_X509Name_(X509_NAME *name, enum iTlsCertificateNameType type,
                                const iTlsCertificateName *names) {
    const iString *domain = findName_(names, type);
    if (domain) {
        const iRangecc range = range_String(domain);
        iRangecc comp = iNullRange;
        while (nextSplit_Rangecc(range, ".", &comp)) {
            X509_NAME_add_entry_by_txt(
                name, "DC", MBSTRING_UTF8, (const void *) comp.start, (int) size_Range(&comp), -1, 0);
        }
    }
}

static X509_NAME *makeX509Name_(int kindBit, const iTlsCertificateName *names) {
    X509_NAME *name = X509_NAME_new();
    add_X509Name_(name, "CN",           kindBit | commonName_TlsCertificateNameType, names);
    add_X509Name_(name, "emailAddress", kindBit | emailAddress_TlsCertificateNameType, names);
    add_X509Name_(name, "UID",          kindBit | userId_TlsCertificateNameType, names);
    addDomain_X509Name_(name,           kindBit | domain_TlsCertificateNameType, names);
    add_X509Name_(name, "OU",           kindBit | organizationalUnit_TlsCertificateNameType, names);
    add_X509Name_(name, "O",            kindBit | organization_TlsCertificateNameType, names);
    add_X509Name_(name, "C",            kindBit | country_TlsCertificateNameType, names);
    return name;
}

static void checkErrors_(void) {
    for (unsigned long err = ERR_get_error(); err; err = ERR_get_error()) {
        iDebug("[OpenSSL] %s: %s: %s\n",
               ERR_lib_error_string(err),
               ERR_func_error_string(err),
               ERR_reason_error_string(err));
    }
}

iTlsCertificate *newSelfSignedRSA_TlsCertificate(
    int rsaBits, iDate validUntil, const iTlsCertificateName *names) {
    initContext_();
    /* Seed the random number generator. */
    if (!isPrngSeeded_) {
        iTime now;
        initCurrent_Time(&now);
        RAND_seed(&now.ts.tv_nsec, sizeof(now.ts.tv_nsec));
        isPrngSeeded_ = iTrue;
    }
    RSA *rsa = RSA_new();
    BIGNUM *exponent = NULL;
    BN_asc2bn(&exponent, "65537");
    RSA_generate_key_ex(rsa, rsaBits, exponent, NULL);
    BN_free(exponent);
    iTlsCertificate *d = new_TlsCertificate();
    d->cert = X509_new();
    d->pkey = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(d->pkey, rsa);
    X509_set_pubkey(d->cert, d->pkey);
#if !defined (LIBRESSL_VERSION_NUMBER)
    /* Random serial number. */ {
        BIGNUM *big = BN_new();
        if (BN_rand(big, 64, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY)) {
            ASN1_INTEGER *num = ASN1_INTEGER_new();
            if (num && BN_to_ASN1_INTEGER(big, num)) {
                X509_set_serialNumber(d->cert, num);
            }
            ASN1_INTEGER_free(num);
        }
        BN_free(big);
    }
#endif
    /* Set names. */ {
        X509_NAME *issuer = makeX509Name_(issuerBit_TlsCertificateNameType, names);
        X509_set_issuer_name(d->cert, issuer);
        X509_NAME_free(issuer);
        X509_NAME *subject = makeX509Name_(subjectBit_TlsCertificateNameType, names);
        X509_set_subject_name(d->cert, subject);
        X509_NAME_free(subject);
    }
    /* Valid from. */ {
        ASN1_UTCTIME *notBefore = ASN1_UTCTIME_new();
        ASN1_UTCTIME_set(notBefore, time(NULL));
        X509_set1_notBefore(d->cert, notBefore);
        ASN1_UTCTIME_free(notBefore);
    }
    /* Valid until. */ {
        ASN1_TIME *notAfter = ASN1_TIME_new();
        ASN1_TIME_set(notAfter, sinceEpoch_Date(&validUntil));
        X509_set1_notAfter(d->cert, notAfter);
        ASN1_TIME_free(notAfter);
    }
    X509_sign(d->cert, d->pkey, EVP_sha256());
    checkErrors_();
    return d;
}

iTlsCertificate *copy_TlsCertificate(const iTlsCertificate *d) {
    iTlsCertificate *copy = new_TlsCertificate();
    if (d->cert) {
        X509_up_ref(d->cert);
        copy->cert = d->cert;
    }
    copy->chain = d->chain ? X509_chain_up_ref(d->chain) : NULL;
    if (d->pkey) {
        EVP_PKEY_up_ref(d->pkey);
        copy->pkey = d->pkey;
    }
    *copy->cachedVerifyStatus = *d->cachedVerifyStatus;
    return copy;
}

iBool isEmpty_TlsCertificate(const iTlsCertificate *d) {
    return d->cert == NULL;
}

iBool hasPrivateKey_TlsCertificate(const iTlsCertificate *d) {
    return d->pkey != NULL;
}

iString *subject_TlsCertificate(const iTlsCertificate *d) {
    iString *sub = new_String();
    if (d->cert) {
        BIO *buf = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(
            buf, X509_get_subject_name(d->cert), 0, XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
        readAllFromBIO_(buf, &sub->chars);
        BIO_free(buf);
    }
    return sub;
}

iString *issuer_TlsCertificate(const iTlsCertificate *d) {
    iString *sub = new_String();
    if (d->cert) {
        BIO *buf = BIO_new(BIO_s_mem());
        X509_NAME_print_ex(
            buf, X509_get_issuer_name(d->cert), 0, XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB);
        readAllFromBIO_(buf, &sub->chars);
        BIO_free(buf);
    }
    return sub;
}

void validUntil_TlsCertificate(const iTlsCertificate *d, iDate *untilDate_out) {
    iAssert(untilDate_out);
    iZap(*untilDate_out);
    if (d->cert) {
        struct tm time;
#if defined (LIBRESSL_VERSION_NUMBER)
        const ASN1_TIME *notAfter = X509_get0_notAfter(d->cert);
        ASN1_time_parse((const char *) ASN1_STRING_get0_data(notAfter),
                        ASN1_STRING_length(notAfter),
                        &time,
                        0);
#else
        ASN1_TIME_to_tm(X509_get0_notAfter(d->cert), &time);
#endif
        initStdTime_Date(untilDate_out, &time);
    }
}

iBool isExpired_TlsCertificate(const iTlsCertificate *d) {
    if (!d->cert) return iTrue;
    return X509_cmp_current_time(X509_get0_notAfter(d->cert)) < 0;
}

enum iTlsCertificateVerifyStatus verify_TlsCertificate(const iTlsCertificate *d) {
    if (*d->cachedVerifyStatus != unknown_TlsCertificateVerifyStatus) {
        return *d->cachedVerifyStatus;
    }
    enum iTlsCertificateVerifyStatus status = unverified_TlsCertificateVerifyStatus;
    if (!d->cert) {
        return status;
    }
    X509_STORE_CTX *store = X509_STORE_CTX_new();
    X509_STORE_CTX_init(store, context_->certStore, d->cert, d->chain);
    const int result = X509_verify_cert(store);
    const int err    = X509_STORE_CTX_get_error(store);
    if (err == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) {
        status = selfSigned_TlsCertificateVerifyStatus;
    }
    else if (result) {
        status = authority_TlsCertificateVerifyStatus;
    }
    iDebug("[TlsCertificate] result:%d %s (error:%d)\n",
           result,
           X509_verify_cert_error_string(err),
           err);
    X509_STORE_CTX_free(store);
    *d->cachedVerifyStatus = status;
    return status;
}

iBool verifyDomain_TlsCertificate(const iTlsCertificate *d, iRangecc domain) {
    if (!d->cert) return iFalse;
    /* Check the common name first manually. X509_check_host() seems to prioritize SAN,
       so if it doesn't match the CN is ignored. */ {
        X509_NAME_ENTRY *e;
        X509_NAME *subject = X509_get_subject_name(d->cert);
        for (int lastpos = -1;;) {
            lastpos = X509_NAME_get_index_by_NID(subject, NID_commonName, lastpos);
            if (lastpos == -1) {
                break;
            }
            e = X509_NAME_get_entry(subject, lastpos);
            const char *name = (const char *) ASN1_STRING_get0_data(X509_NAME_ENTRY_get_data(e));
            if (equalCase_Rangecc(domain, name)) {
                return iTrue;
            }
            if (startsWith_CStr(name, "*.")) { /* Wildcard. */
                if (endsWithCase_Rangecc(domain, name + 1)) {
                    return iTrue;
                }
            }
        }
    }
    /* OpenSSL's check does wildcards, too. */
    return X509_check_host(d->cert, domain.start, size_Range(&domain), 0, NULL) > 0;
}

iBool verifyIp_TlsCertificate(const iTlsCertificate *d, const iString *ipAddress) {
    if (!d->cert || isEmpty_String(ipAddress)) return iFalse;
    int rc = X509_check_ip_asc(d->cert, cstr_String(ipAddress), 0);
    iAssert(rc != -2 /* bad input */);
    return rc > 0;
}

iBool equal_TlsCertificate(const iTlsCertificate *d, const iTlsCertificate *other) {
    if (d->cert == NULL && other->cert == NULL) {
        return iTrue;
    }
    if (d->cert == NULL || other->cert == NULL) {
        return iFalse;
    }
    return X509_cmp(d->cert, other->cert) == 0;
}

static void calcSHA256_(BIO *src, iBlock *dst) {
    iBlock der;
    init_Block(&der, 0);
    readAllFromBIO_(src, &der);
    SHA256(constData_Block(&der), size_Block(&der), data_Block(dst));
    deinit_Block(&der);
}

iBlock *fingerprint_TlsCertificate(const iTlsCertificate *d) {
    iBlock *sha = new_Block(SHA256_DIGEST_LENGTH);
    if (d->cert) {
        /* Get the DER serialization of the certificate. */
        BIO *buf = BIO_new(BIO_s_mem());
        i2d_X509_bio(buf, d->cert);
        calcSHA256_(buf, sha);
        BIO_free(buf);
    }
    return sha;
}

iBlock *publicKeyFingerprint_TlsCertificate(const iTlsCertificate *d) {
    iBlock *sha = new_Block(SHA256_DIGEST_LENGTH);
    if (!d->cert) {
        return sha;
    }
    EVP_PKEY *pub = X509_get_pubkey(d->cert);
    if (pub) {
        /* Get the DER serialization of the public key. */
        BIO *buf = BIO_new(BIO_s_mem());
        i2d_PUBKEY_bio(buf, pub);
        calcSHA256_(buf, sha);
        BIO_free(buf);
        EVP_PKEY_free(pub);
    }
    return sha;
}

iBlock *privateKeyFingerprint_TlsCertificate(const iTlsCertificate *d) {
    iBlock *sha = new_Block(SHA256_DIGEST_LENGTH);
    if (d->pkey) {
        /* Get the DER serialization of the private key. */
        BIO *buf = BIO_new(BIO_s_mem());
        i2d_PrivateKey_bio(buf, d->pkey);
        calcSHA256_(buf, sha);
        BIO_free(buf);
    }
    return sha;
}

iString *pem_TlsCertificate(const iTlsCertificate *d) {
    iString *pem = new_String();
    if (d->cert) {
        BIO *buf = BIO_new(BIO_s_mem());
        PEM_write_bio_X509(buf, d->cert);
        readAllFromBIO_(buf, &pem->chars);
        BIO_free(buf);
    }
    return pem;
}

iString *privateKeyPem_TlsCertificate(const iTlsCertificate *d) {
    iString *pem = new_String();
    if (d->pkey) {
        BIO *buf = BIO_new(BIO_s_mem());
        PEM_write_bio_PrivateKey(buf, d->pkey, NULL, NULL, 0, NULL, NULL);
        readAllFromBIO_(buf, &pem->chars);
        BIO_free(buf);
    }
    return pem;
}

/*----------------------------------------------------------------------------------------------*/

struct Impl_TlsRequest {
    iObject          object;
    iMutex           mtx;
    /* Connection. */
    iString *        hostName;
    uint16_t         port;
    iSocket *        socket;
    const iTlsCertificate *clientCert;
    /* Payload and result. */
    iBlock           content;
    iBuffer *        result;
    iTlsCertificate *cert; /* server certificate */
    iBool            certVerifyFailed;
    /* Internal state. */
    volatile enum iTlsRequestStatus status;
    iString *        errorMsg;
    iThread *        thread;
    iBool            sessionCacheEnabled;
    iBool            notifyReady;
    size_t           totalBytesToSend;
    size_t           totalBytesSent;
    iBlock  *        incoming;
    iCondition       gotIncoming;
    iMutex           incomingMtx;
    iCondition       requestDone;
    iAudience *      readyRead;
    iAudience *      sent;
    iAudience *      finished;
    /* OpenSSL state. */
    SSL *            ssl;
    BIO *            rbio; /* we insert incoming encrypted bytes here for SSL to read */
    BIO *            wbio; /* SSL sends encrypted bytes to socket */
    iBlock           sending;
};

iDefineObjectConstruction(TlsRequest)
iDefineAudienceGetter(TlsRequest, readyRead)
iDefineAudienceGetter(TlsRequest, sent)
iDefineAudienceGetter(TlsRequest, finished)

static void setError_TlsRequest_(iTlsRequest *d, const char *msg);

enum iSSLResult { ok_SSLResult, wantIO_SSLResult, closed_SSLResult, fail_SSLResult };

static enum iSSLResult sslResult_TlsRequest_(iTlsRequest *d, int code) {
    switch (SSL_get_error(d->ssl, code)) {
        case SSL_ERROR_NONE:
            return ok_SSLResult;
        case SSL_ERROR_WANT_WRITE:
        case SSL_ERROR_WANT_READ:
            return wantIO_SSLResult;
        case SSL_ERROR_ZERO_RETURN:
            return closed_SSLResult;
        case SSL_ERROR_SYSCALL:
        default:
//            fprintf(stderr, "[TlsRequest] SSL_get_error returns %d (code:%d)\n",
//                    SSL_get_error(d->ssl, code), code);
            ERR_print_errors_fp(stderr);
            return fail_SSLResult;
    }
}

static void setStatus_TlsRequest_(iTlsRequest *d, enum iTlsRequestStatus st) {
    lock_Mutex(&d->mtx);
    if (d->status != st) {
        d->status = st;
        if (st == finished_TlsRequestStatus || st == error_TlsRequestStatus) {
            signalAll_Condition(&d->requestDone);
        }
        unlock_Mutex(&d->mtx);
        lock_Mutex(&d->incomingMtx);
        signal_Condition(&d->gotIncoming); /* wake up if sleeping */
        unlock_Mutex(&d->incomingMtx);
    }
    else {
        unlock_Mutex(&d->mtx);
    }
}

static void flushToSocket_TlsRequest_(iTlsRequest *d) {
    char buf[DEFAULT_BUF_SIZE];
    int n;
    do {
        n = BIO_read(d->wbio, buf, sizeof(buf));
        if (n > 0) {
            d->totalBytesToSend += n;
            writeData_Socket(d->socket, buf, n);
        }
        else if (!BIO_should_retry(d->wbio)) {
            iDebug("[TlsRequest] output error (BIO_read)\n");
            setStatus_TlsRequest_(d, error_TlsRequestStatus);
            return;
        }
    } while (n > 0);
}

static enum iSSLResult doHandshake_TlsRequest_(iTlsRequest *d) {
    int n = SSL_do_handshake(d->ssl);
    enum iSSLResult result = sslResult_TlsRequest_(d, n);
    if (result == wantIO_SSLResult) {
        flushToSocket_TlsRequest_(d);
    }
    return result;
}

static iBool encrypt_TlsRequest_(iTlsRequest *d) {
    if (!SSL_is_init_finished(d->ssl)) {
        return iFalse;
    }
    while (!isEmpty_Block(&d->sending)) {
        int n = SSL_write(d->ssl, constData_Block(&d->sending), (int) size_Block(&d->sending));
        enum iSSLResult status = sslResult_TlsRequest_(d, n);
        if (n > 0) {
            remove_Block(&d->sending, 0, n);
            flushToSocket_TlsRequest_(d);
        }
        if (status == fail_SSLResult) {
            iDebug("[TlsRequest] failure to encrypt (SSL_write)\n");
            setError_TlsRequest_(d, "failure to encrypt data");
            return iTrue;
        }
        if (n == 0) {
            break;
        }
    }
    return iTrue;
}

void setCiphers_TlsRequest(const char *cipherList) {
    initContext_();
    SSL_CTX_set_cipher_list(context_->ctx, cipherList);
}

const char *libraryName_TlsRequest(void) {
    initContext_();
    return cstr_String(&context_->libraryName);
}

void init_TlsRequest(iTlsRequest *d) {
    initContext_();
    init_Mutex(&d->mtx);
    d->hostName = new_String();
    d->port = 0;
    d->socket = NULL;
    d->clientCert = NULL;
    init_Block(&d->content, 0);
    d->result = new_Buffer();
    openEmpty_Buffer(d->result);
    d->cert = NULL;
    d->errorMsg = new_String();
    d->status = initialized_TlsRequestStatus;
    d->sessionCacheEnabled = iTrue;
    d->thread = NULL;
    d->notifyReady = iFalse;
    d->totalBytesToSend = 0;
    d->totalBytesSent = 0;
    d->incoming = new_Block(0);
    init_Mutex(&d->incomingMtx);
    init_Condition(&d->gotIncoming);
    init_Condition(&d->requestDone);
    d->readyRead = NULL;
    d->sent = NULL;
    d->finished = NULL;
    d->ssl = SSL_new(context_->ctx);
    /* We could also try BIO_s_socket() but all BSD socket related code should be encapsulated
       into the Socket class. */
    d->rbio = BIO_new(BIO_s_mem());
    d->wbio = BIO_new(BIO_s_mem());
    SSL_set_connect_state(d->ssl);
    SSL_set_bio(d->ssl, d->rbio, d->wbio);
    init_Block(&d->sending, 0);
}

void deinit_TlsRequest(iTlsRequest *d) {
    iGuardMutex(&d->incomingMtx, signal_Condition(&d->gotIncoming));
    iGuardMutex(&d->mtx, d->status = finished_TlsRequestStatus);
    if (d->thread) {
        join_Thread(d->thread);
        iRelease(d->thread);
    }
    deinit_Block(&d->sending);
    SSL_free(d->ssl);
    deinit_Condition(&d->requestDone);
    deinit_Condition(&d->gotIncoming);
    deinit_Mutex(&d->incomingMtx);
    delete_Block(d->incoming);
    delete_Audience(d->finished);
    delete_Audience(d->sent);
    delete_Audience(d->readyRead);
    delete_String(d->errorMsg);
    delete_TlsCertificate(d->cert);
    iRelease(d->result);
    deinit_Block(&d->content);
    iRelease(d->socket);
    delete_String(d->hostName);
    deinit_Mutex(&d->mtx);
}

void setHost_TlsRequest(iTlsRequest *d, const iString *hostName, uint16_t port) {
    set_String(d->hostName, hostName);
    d->port = port;
}

void setContent_TlsRequest(iTlsRequest *d, const iBlock *content) {
    set_Block(&d->content, content);
}

void setCertificate_TlsRequest(iTlsRequest *d, const iTlsCertificate *cert) {
    d->clientCert = cert;
}

void setSessionCacheEnabled_TlsRequest(iTlsRequest *d, iBool enabled) {
    d->sessionCacheEnabled = enabled;
}

static void appendReceived_TlsRequest_(iTlsRequest *d, const char *buf, size_t len) {
    if (len > 0) {
        iGuardMutex(&d->mtx, {
            writeData_Buffer(d->result, buf, len);
        });
        d->notifyReady = iTrue;
    }
}

static int processIncoming_TlsRequest_(iTlsRequest *d, const char *src, size_t len) {
    /* Note: Runs in the socket thread. */
    char buf[DEFAULT_BUF_SIZE];
    enum iSSLResult status;
    int n;
    do {
        if (len > 0) {
            n = BIO_write(d->rbio, src, (int) len);
            if (n <= 0) {
                return -1; /* assume bio write failure is unrecoverable */
            }
            src += n;
            len -= n;
        }
        if (!SSL_is_init_finished(d->ssl)) {
            if (doHandshake_TlsRequest_(d) == fail_SSLResult) {
                iDebug("[TlsRequest] handshake failure\n");
                setError_TlsRequest_(d, "TLS/SSL handshake failed");
                return -1;
            }
            if (!SSL_is_init_finished(d->ssl)) {
                return 0; /* continue later */
            }
        }
        if (!d->cert) {
            STACK_OF(X509) *chain = SSL_get_peer_cert_chain(d->ssl);
            d->cert = newX509Chain_TlsCertificate_(SSL_get_peer_certificate(d->ssl),
                                                   X509_chain_up_ref(chain));
        }
        /* The encrypted data is now in the input bio so now we can perform actual
           read of unencrypted data. */
        do {
            n = SSL_read(d->ssl, buf, sizeof(buf));
            if (n > 0) {
                appendReceived_TlsRequest_(d, buf, (size_t) n);
            }
        } while (n > 0);

        status = sslResult_TlsRequest_(d, n);
        /* Did SSL request to write bytes? This can happen if peer has requested SSL
           renegotiation. */
        if (status == wantIO_SSLResult) {
            flushToSocket_TlsRequest_(d);
        }
        if (status == fail_SSLResult) {
            setError_TlsRequest_(d, "error while decrypting incoming data");
            return -1;
        }
        if (status == closed_SSLResult && len == 0) {
            setStatus_TlsRequest_(d, finished_TlsRequestStatus); /* even if socket remains open */
        }
    } while (len > 0);
    return 0;
}

static void checkReadyRead_TlsRequest_(iTlsRequest *d) {
    /* All notifications are done from the TlsRequest thread. */
    if (d->notifyReady) {
        d->notifyReady = iFalse;
        iNotifyAudience(d, readyRead, TlsRequestReadyRead);
    }
}

static void gotIncoming_TlsRequest_(iTlsRequest *d, iSocket *socket) {
    iUnused(socket);
    iBlock *data = readAll_Socket(socket);
    lock_Mutex(&d->incomingMtx);
    appendData_Block(d->incoming, constData_Block(data), size_Block(data));
    signal_Condition(&d->gotIncoming);
    unlock_Mutex(&d->incomingMtx);
    delete_Block(data);
}

static iBool readIncoming_TlsRequest_(iTlsRequest *d) {
    lock_Mutex(&d->incomingMtx);
    const iBool didRead = !isEmpty_Block(d->incoming);
    processIncoming_TlsRequest_(d, data_Block(d->incoming), size_Block(d->incoming));
    clear_Block(d->incoming);
    unlock_Mutex(&d->incomingMtx);
    checkReadyRead_TlsRequest_(d);
    return didRead;
}

static iThreadResult run_TlsRequest_(iThread *thread) {
    iTlsRequest *d = userData_Thread(thread);
    /* Thread-local pointer to the current request so it can be accessed in the 
       verify callback. */
    iDebug("[TlsRequest] run_TlsRequest_: %zu bytes to send\n", size_Block(&d->sending));
    setCurrentRequestForThread_Context_(context_, d);
    doHandshake_TlsRequest_(d);
    for (;;) {
        encrypt_TlsRequest_(d);
        if (!readIncoming_TlsRequest_(d)) {
            lock_Mutex(&d->mtx);
            if (d->status == submitted_TlsRequestStatus) {
                unlock_Mutex(&d->mtx);
                /* Wait for incoming data if there isn't some available. */
                lock_Mutex(&d->incomingMtx);
                if (isEmpty_Block(d->incoming)) {
                    wait_Condition(&d->gotIncoming, &d->incomingMtx);
                }
                unlock_Mutex(&d->incomingMtx);
            }
            else {
                //fprintf(stderr, "[TlsRequest] run loop exiting, status %d\n", d->status);
                unlock_Mutex(&d->mtx);
                break;
            }
        }
    }
    if (!SSL_session_reused(d->ssl) && d->status != error_TlsRequestStatus) {
        saveSession_Context_(
            context_, d->hostName, d->port, SSL_get0_session(d->ssl), d->cert, d->clientCert);
    }
    readIncoming_TlsRequest_(d);
    iNotifyAudience(d, finished, TlsRequestFinished);
    iDebug("[TlsRequest] finished\n");
    return 0;
}

static void connected_TlsRequest_(iTlsRequest *d, iSocket *sock) {
    /* The socket has been connected. During this notification the socket remains locked
       so we must start a different thread for carrying out the I/O. */
    iUnused(sock);
    iAssert(!d->thread);
    d->thread = new_Thread(run_TlsRequest_);
    setName_Thread(d->thread, "TlsRequest");
    setUserData_Thread(d->thread, d);
    start_Thread(d->thread);
}

static void disconnected_TlsRequest_(iTlsRequest *d, iSocket *sock) {
    iUnused(sock);
    setStatus_TlsRequest_(d, finished_TlsRequestStatus);
}

static void bytesWritten_TlsRequest_(iTlsRequest *d, iSocket *sock, size_t num) {
    d->totalBytesSent += num;
    iNotifyAudienceArgs(d, sent, TlsRequestSent, d->totalBytesSent, d->totalBytesToSend);
}

static void setError_TlsRequest_(iTlsRequest *d, const char *msg) {
    setCStr_String(d->errorMsg, msg);
    setStatus_TlsRequest_(d, error_TlsRequestStatus);
}

static void handleError_TlsRequest_(iTlsRequest *d, iSocket *sock, int error, const char *msg) {
    iUnused(sock, error);
    setError_TlsRequest_(d, msg);
    if (!d->thread) {
        iNotifyAudience(d, finished, TlsRequestFinished);
    }
}

void submit_TlsRequest(iTlsRequest *d) {
    if (d->status == submitted_TlsRequestStatus) {
        iDebug("[TlsRequest] request already ongoing\n");
        return;
    }
    clear_Buffer(d->result);
    clear_String(d->errorMsg);
    set_Block(&d->sending, &d->content);
    iRelease(d->socket);
    d->certVerifyFailed = iFalse;
    SSL_set1_host(d->ssl, cstr_String(d->hostName));
    /* Server Name Indication for the handshake. */
    if (!contains_String(d->hostName, ':')) { /* Domain names only (not literal IPv6 addresses). */
        SSL_set_tlsext_host_name(d->ssl, cstr_String(d->hostName));
    }
    /* The client certificate. */
    if (d->clientCert) {
        SSL_use_certificate(d->ssl, d->clientCert->cert);
        SSL_use_PrivateKey(d->ssl, d->clientCert->pkey);
    }
    if (d->sessionCacheEnabled) {
        d->cert = maybeReuseSession_Context_(context_, d->ssl, d->hostName, d->port, d->clientCert);
    }
    d->socket = new_Socket(cstr_String(d->hostName), d->port);
    iConnect(Socket, d->socket, connected, d, connected_TlsRequest_);
    iConnect(Socket, d->socket, disconnected, d, disconnected_TlsRequest_);
    iConnect(Socket, d->socket, readyRead, d, gotIncoming_TlsRequest_);
    iConnect(Socket, d->socket, bytesWritten, d, bytesWritten_TlsRequest_);
    iConnect(Socket, d->socket, error, d, handleError_TlsRequest_);
    d->status = submitted_TlsRequestStatus;
    if (!open_Socket(d->socket)) {
        d->status = error_TlsRequestStatus;
    }
}

void cancel_TlsRequest(iTlsRequest *d) {
    lock_Mutex(&d->mtx);
    if (d->status == submitted_TlsRequestStatus) {
        d->status = error_TlsRequestStatus;
        unlock_Mutex(&d->mtx);
        close_Socket(d->socket);
    }
    else {
        unlock_Mutex(&d->mtx);
    }
    signal_Condition(&d->gotIncoming);
    join_Thread(d->thread);
    iReleasePtr(&d->thread);
}

void waitForFinished_TlsRequest(iTlsRequest *d) {
    lock_Mutex(&d->mtx);
    if (d->status == submitted_TlsRequestStatus) {
        wait_Condition(&d->requestDone, &d->mtx);
    }
    unlock_Mutex(&d->mtx);
}

const iAddress *address_TlsRequest(const iTlsRequest *d) {
    return d->socket ? address_Socket(d->socket) : NULL;
}

enum iTlsRequestStatus status_TlsRequest(const iTlsRequest *d) {
    return d->status;
}

const iString *errorMessage_TlsRequest(const iTlsRequest *d) {
    return d->errorMsg;
}

static void certificateVerifyFailed_TlsRequest_(iTlsRequest *d, const iTlsCertificate *cert) {
    iAssert(d->cert == NULL);
    d->cert = copy_TlsCertificate(cert);
    d->certVerifyFailed = iTrue;
}

iBool isVerified_TlsRequest(const iTlsRequest *d) {
    return !d->certVerifyFailed;
}

const iTlsCertificate *serverCertificate_TlsRequest(const iTlsRequest *d) {
    return d->cert;
}

iBlock *readAll_TlsRequest(iTlsRequest *d) {
    iBlock *rd;
    iGuardMutex(&d->mtx, rd = consumeAll_Buffer(d->result));
    return rd;
}

size_t receivedBytes_TlsRequest(const iTlsRequest *d) {
    size_t len;
    iGuardMutex(&d->mtx, len = size_Buffer(d->result));
    return len;
}

iDefineClass(TlsRequest)
