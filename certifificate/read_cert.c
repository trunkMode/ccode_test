#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "seuBase64.h"
#include "read_cert.h"

#define TMP_CCERT_PATH                      "client.crt"
#define CERT_BEGIN_MARK                     "-----BEGIN CERTIFICATE-----"
#define CERT_END_MARK                       "-----END CERTIFICATE-----"
#define CT_LOG_DEBUG(level,format,arg...)      printf(format, ##arg)

#define STRIP_END_CHAR(str,c) {         \
    int __len = strlen((str));          \
    if ((str)[__len - 1] == (c)) {      \
        (str)[__len - 1] = '\0';        \
    }                                   \
}
static char *fingerprintToHex(const void *ptr, int len)
{
	int i;
	char *hex = malloc(len * 3 + 1);
	const char *mem = ptr;
	for (i = 0; i < len; ++i)
	{
		sprintf(hex + (i * 3), "%02X:", mem[i] & 0xFF);
	}
	hex[len * 3 - 1] = '\0';
	return hex;
}

static void
ASN1IntToCString(ASN1_INTEGER *asnInt, char *casnInt, size_t bufsize)
{
	if (asnInt)
	{
		BIO *mbio = BIO_new(BIO_s_mem());
		if (mbio)
		{
			int bytesWritten;
			if ((bytesWritten = i2a_ASN1_INTEGER(mbio, asnInt)) != -1)
			{
				BUF_MEM *out = 0;
				BIO_get_mem_ptr(mbio, &out);
				if (out)
				{
					snprintf(casnInt, bufsize, "%s", out->data);
					if (bytesWritten < (int)bufsize)
						memset(casnInt+bytesWritten, 0, bufsize-bytesWritten);
				}
			}
			BIO_free(mbio);
		}
	}
}

static void NIDToCString(X509_NAME *x509Name, int nid, char *cNid, size_t buflen)
{
	if (x509Name)
	{
		size_t bufRemaining = buflen;
		char *pOut = cNid;

		int lastpos	= X509_NAME_get_index_by_NID(x509Name, nid, -1);

		while (lastpos != -1)
		{
			X509_NAME_ENTRY *xEntry = X509_NAME_get_entry(x509Name, lastpos);

			if (xEntry)
			{
				ASN1_STRING
				*asnStr = X509_NAME_ENTRY_get_data(xEntry);

				unsigned char *asnOut	= 0;
				int asnLen 		= ASN1_STRING_to_UTF8(&asnOut, asnStr);

				if (asnLen != -1)
				{
					size_t bytesWritten = snprintf(pOut, bufRemaining, "%s", asnOut);
					bufRemaining -= bytesWritten;
					pOut += bytesWritten;
					OPENSSL_free(asnOut);
				}
			}
			lastpos = X509_NAME_get_index_by_NID(x509Name, nid, lastpos);
		}
	}
}

static int getCertDetailsFromX509(X509 *x, CertDetails *pCert)
{
	int result = 0;

	bzero(pCert, sizeof(CertDetails));

	//Subject
	X509_NAME *subX509 	= X509_get_subject_name(x);

	NIDToCString(subX509, NID_commonName, pCert->commonName, MAX_NAME_LEN*2);
	NIDToCString(subX509, NID_countryName, pCert->countryName, MAX_NAME_LEN*2);
	NIDToCString(subX509, NID_stateOrProvinceName, pCert->stateName, MAX_NAME_LEN*2);
	NIDToCString(subX509, NID_organizationName, pCert->orgName, MAX_NAME_LEN*2);
	NIDToCString(subX509, NID_organizationalUnitName, pCert->orgUnitName, MAX_NAME_LEN*2);

	// Issuer
	X509_NAME *issuX509 = X509_get_issuer_name(x);

	NIDToCString(issuX509, NID_commonName, pCert->iCommonName, MAX_NAME_LEN*2);
	NIDToCString(issuX509, NID_countryName, pCert->iCountryName, MAX_NAME_LEN*2);
	NIDToCString(issuX509, NID_stateOrProvinceName, pCert->iStateName, MAX_NAME_LEN*2);
	NIDToCString(issuX509, NID_organizationName, pCert->iOrgName, MAX_NAME_LEN*2);
	NIDToCString(issuX509, NID_organizationalUnitName, pCert->iOrgUnitName, MAX_NAME_LEN*2);
    NIDToCString(issuX509, NID_pkcs9_emailAddress, pCert->email, MAX_NAME_LEN*2);

	// Serial number
	ASN1IntToCString(X509_get_serialNumber(x), pCert->serial, MAX_LINE_LEN);

	// Fingerprint
	__uint8_t hBuff[EVP_MAX_MD_SIZE];
	__uint32_t hLen		= 0;
	const EVP_MD *hash 	= EVP_sha1();

	if (X509_digest(x, hash, hBuff, &hLen) == 1)
	{
		const char *hType = OBJ_nid2sn(EVP_MD_type(hash));
		char *hex = fingerprintToHex(hBuff, hLen);
		snprintf(pCert->fingerprint, MAX_LINE_LEN, "%s[%s]", hType ? hType : "?", hex);
	}

	// Validity
	pCert->expiration[0] = '\0';
	pCert->notBefore[0] = '\0';
	BIO *fbio;

	fbio = BIO_new(BIO_s_mem());
	if (!fbio)
		goto err;
	if (!ASN1_TIME_print(fbio, X509_get_notBefore(x))) {
		BIO_free(fbio);
		goto err;
	}
	BIO_read(fbio, pCert->notBefore, MAX_NAME_LEN);
	BIO_free(fbio);

	fbio = BIO_new(BIO_s_mem());
	if (!fbio)
		goto err;
	if (!ASN1_TIME_print(fbio, X509_get_notAfter(x))) {
		BIO_free(fbio);
		goto err;
	}
	BIO_read(fbio, pCert->expiration, MAX_NAME_LEN);
	BIO_free(fbio);

	result = 1;
err:

#if 1//DEBUG
	printf("Cert: commonName:   %s\n", pCert->commonName);
	printf("Cert: countryName:  %s\n", pCert->countryName);
	printf("Cert: stateName:    %s\n", pCert->stateName);
	printf("Cert: orgName:      %s\n", pCert->orgName);
	printf("Cert: orgUnitName:  %s\n", pCert->orgUnitName);
    printf("Cert: email:        %s\n", pCert->email);

	printf("Cert: iCommonName:  %s\n", pCert->iCommonName);
	printf("Cert: iCountryName: %s\n", pCert->iCountryName);
	printf("Cert: iStateName:   %s\n", pCert->iStateName);
	printf("Cert: iOrgName:     %s\n", pCert->iOrgName);
	printf("Cert: iOrgUnitName: %s\n", pCert->iOrgUnitName);

	printf("Cert: serial:       %s\n", pCert->serial);
	printf("Cert: fingerprint:  %s\n", pCert->fingerprint);
	printf("Cert: not before:   %s\n", pCert->notBefore);
	printf("Cert: not after:    %s\n", pCert->expiration);
#endif

	return result;
}

void x509_cert_dump()
{
    FILE *fp = NULL;
    X509 *x509 = NULL;
    CertDetails cert_details;

    do {
        fp = fopen("client.crt", "r");
        if (!fp) {
            printf("Failed to open client.crt\n");
            break;
        }

        x509 = PEM_read_X509(fp, NULL, NULL, NULL);
        if (!x509) {
            printf("Failed to read load x509 cert\n");
            break;
        }
        getCertDetailsFromX509(x509, &cert_details);

        X509_free(x509);
    } while (0);

 
}
int load_ccert()
{
    int ret, len = 0, begin = 0, size = 0, str_len;
    FILE *fp = NULL;
    char buf[1024];
    char *cert = NULL;

    do {
        if (!(fp = fopen(TMP_CCERT_PATH, "r"))) {
            CT_LOG_DEBUG(CT_ERROR, "Failed to open %s.", TMP_CCERT_PATH);
            break;
        }

        if (fseek(fp, 0, SEEK_END) < 0 ||
            (size = ftell(fp)) < 0) {
            CT_LOG_DEBUG(CT_ERROR, "Failed to get file pos.");
            break;
        }
        if (!(cert = malloc(size))) {
            CT_LOG_DEBUG(CT_ERROR, "Failed to allocate mem for ccert.");
            break;
        }


        begin = len = 0;
        fseek(fp, 0, SEEK_SET);
        while (fgets(buf, size, fp)) {
            if (!begin) {
                if (strstr(buf, CERT_BEGIN_MARK)) {
                    begin = 1;
                }
                continue;
            }

            if (!begin && strstr(buf, CERT_BEGIN_MARK))
                continue;

            if (strstr(buf, CERT_END_MARK))
               break;

//            STRIP_END_CHAR(buf, '\n');
            strcat(cert, buf);
            len += strlen(buf);
        }

        printf("cert = %s\n", cert);
        int out_len = 0;
#if 1
        char *base64decode(const char *code_in, int *length, int place_in);
        uint8_t *decode = base64decode(cert, &len, 1);
        out_len = len;
        printf("len = %d\n", len);

#else
        char *decode = b64decode(cert, &out_len, 0);
//        char *decode = b64decode(cert, NULL, 1);
//          char *decode = base64decode(cert);
#endif
        printf("len = %d\n", strlen(cert));
        printf("out_len = %d\n", out_len);
        printf("decode = %s\n", decode);
#if 1
        for (int i = 0; i < out_len; i++) {
            printf("%02x ", (uint8_t)decode[i]);
            if (((i-3) % 16) == 0)
                printf("\n");
        }
#endif

    } while(0);

    if (fp)
        fclose(fp);
    if (cert)
        free(cert);
    return 0;
}

#if 1
static int read_text_file(const char *file_path)
{
    char buffer[1024];
    int buffer_length = sizeof(buffer);
    int i, result = -1;

    if (file_path != NULL) {
        BIO *file_io = BIO_new_file(file_path, "r");

        if (file_io != NULL) {
            result = BIO_read(file_io, buffer, buffer_length);

            /* dump */
            for (i = 0; i < result; i++) {
                printf("%c", buffer[i]);
            }
            BIO_free(file_io);
        }
    }

    return result;
}
#endif

int main()
{
#if 0
    read_text_file("client.crt");
    return;
#endif
#if 0
    x509_cert_dump();
    return 0;
#endif
    load_ccert();
    printf("end main \n");
    return 0;
}
