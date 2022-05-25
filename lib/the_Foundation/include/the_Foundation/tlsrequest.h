#pragma once

/** @file the_Foundation/tlsrequest.h  TLS request over a Socket (using OpenSSL)

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

#include "address.h"
#include "audience.h"
#include "object.h"
#include "string.h"

iBeginPublic

iDeclareType(TlsCertificateName)

enum iTlsCertificateNameType {
    none_TlsCertificateNameType, /* array terminator */
    commonName_TlsCertificateNameType,
    emailAddress_TlsCertificateNameType,
    userId_TlsCertificateNameType,
    domain_TlsCertificateNameType,
    organization_TlsCertificateNameType,
    organizationalUnit_TlsCertificateNameType,
    country_TlsCertificateNameType,
    issuerBit_TlsCertificateNameType  = 0x10,
    subjectBit_TlsCertificateNameType = 0x20,
    /* name component IDs */
    issuerCommonName_TlsCertificateNameType = issuerBit_TlsCertificateNameType |
                                              commonName_TlsCertificateNameType,
    issuerEmailAddress_TlsCertificateNameType,
    issuerUserId_TlsCertificateNameType,
    issuerDomain_TlsCertificateNameType,
    issuerOrganization_TlsCertificateNameType,
    issuerOrganizationalUnit_TlsCertificateNameType,
    issuerCountry_TlsCertificateNameType,
    subjectCommonName_TlsCertificateNameType = subjectBit_TlsCertificateNameType |
                                               commonName_TlsCertificateNameType,
    subjectEmailAddress_TlsCertificateNameType,
    subjectUserId_TlsCertificateNameType,
    subjectDomain_TlsCertificateNameType,
    subjectOrganization_TlsCertificateNameType,
    subjectOrganizationalUnit_TlsCertificateNameType,
    subjectCountry_TlsCertificateNameType,
};

struct Impl_TlsCertificateName {
    enum iTlsCertificateNameType type;
    const iString *text;
};

enum iTlsCertificateVerifyStatus {
    unknown_TlsCertificateVerifyStatus = -1,
    unverified_TlsCertificateVerifyStatus = 0,
    selfSigned_TlsCertificateVerifyStatus,
    authority_TlsCertificateVerifyStatus,
};

iDeclareType(TlsCertificate)
iDeclareTypeConstruction(TlsCertificate)

iTlsCertificate *   newPem_TlsCertificate       (const iString *pem);
iTlsCertificate *   newPemKey_TlsCertificate    (const iString *certPem, const iString *keyPem);

iTlsCertificate *   newSelfSignedRSA_TlsCertificate(int rsaBits, iDate validUntil,
                                                    const iTlsCertificateName *namesNullTerminatedArray);
iTlsCertificate *   copy_TlsCertificate         (const iTlsCertificate *);

iBool               isEmpty_TlsCertificate      (const iTlsCertificate *);
iBool               hasPrivateKey_TlsCertificate(const iTlsCertificate *);
iString *           subject_TlsCertificate      (const iTlsCertificate *);
iString *           issuer_TlsCertificate       (const iTlsCertificate *);
void                validUntil_TlsCertificate   (const iTlsCertificate *, iDate *untilDate_out);
iBool               isExpired_TlsCertificate    (const iTlsCertificate *);
enum iTlsCertificateVerifyStatus
                    verify_TlsCertificate       (const iTlsCertificate *);
iBool               verifyDomain_TlsCertificate (const iTlsCertificate *, iRangecc domain); /* supports wildcards */
iBool               verifyIp_TlsCertificate     (const iTlsCertificate *, const iString *ipAddress);
iBool               equal_TlsCertificate        (const iTlsCertificate *, const iTlsCertificate *);
iString *           pem_TlsCertificate          (const iTlsCertificate *);
iString *           privateKeyPem_TlsCertificate(const iTlsCertificate *);

iBlock *            fingerprint_TlsCertificate          (const iTlsCertificate *);
iBlock *            publicKeyFingerprint_TlsCertificate (const iTlsCertificate *);
iBlock *            privateKeyFingerprint_TlsCertificate(const iTlsCertificate *);

/*----------------------------------------------------------------------------------------------*/

iDeclareClass(TlsRequest)
iDeclareObjectConstruction(TlsRequest)

iDeclareNotifyFunc    (TlsRequest, ReadyRead)
iDeclareNotifyFuncArgs(TlsRequest, Sent, size_t sent, size_t toSend)
iDeclareNotifyFunc    (TlsRequest, Finished)

iDeclareAudienceGetter(TlsRequest, readyRead)
iDeclareAudienceGetter(TlsRequest, sent)
iDeclareAudienceGetter(TlsRequest, finished)

enum iTlsRequestStatus {
    initialized_TlsRequestStatus,
    submitted_TlsRequestStatus,
    finished_TlsRequestStatus,
    error_TlsRequestStatus,
};

void        setHost_TlsRequest          (iTlsRequest *, const iString *hostName, uint16_t port);
void        setContent_TlsRequest       (iTlsRequest *, const iBlock *content);
void        setCertificate_TlsRequest   (iTlsRequest *, const iTlsCertificate *cert);
void        setSessionCacheEnabled_TlsRequest       (iTlsRequest *, iBool enabled);

void        submit_TlsRequest           (iTlsRequest *);
void        cancel_TlsRequest           (iTlsRequest *);
void        waitForFinished_TlsRequest  (iTlsRequest *);
iBlock *    readAll_TlsRequest          (iTlsRequest *);

const iAddress *        address_TlsRequest          (const iTlsRequest *);
size_t                  receivedBytes_TlsRequest    (const iTlsRequest *);
enum iTlsRequestStatus  status_TlsRequest           (const iTlsRequest *);
const iString *         errorMessage_TlsRequest     (const iTlsRequest *);
const iTlsCertificate * serverCertificate_TlsRequest(const iTlsRequest *);
iBool                   isVerified_TlsRequest       (const iTlsRequest *);

typedef iBool (*iTlsRequestVerifyFunc)(iTlsRequest *, const iTlsCertificate *, int depth);

void        setCACertificates_TlsRequest(const iString *caFile, const iString *caPath);
void        setCiphers_TlsRequest       (const char *cipherList);
void        setVerifyFunc_TlsRequest    (iTlsRequestVerifyFunc verifyFunc);

iEndPublic
