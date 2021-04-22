#ifndef __READ_CERT_H__
#define __READ_CERT_H__

#define MAX_NAME_LEN                128
#define MAX_LINE_LEN                124
typedef struct certDetails {
    char commonName[MAX_NAME_LEN*2];
    char countryName[MAX_NAME_LEN*2];
    char stateName[MAX_NAME_LEN*2];
    char orgName[MAX_NAME_LEN*2];
    char orgUnitName[MAX_NAME_LEN*2];
    char email[MAX_NAME_LEN*2];

    char iCommonName[MAX_NAME_LEN*2];
    char iCountryName[MAX_NAME_LEN*2];
    char iStateName[MAX_NAME_LEN*2];
    char iOrgName[MAX_NAME_LEN*2];
    char iOrgUnitName[MAX_NAME_LEN*2];

    char serial[MAX_LINE_LEN];
    char fingerprint[MAX_LINE_LEN];
    char expiration[MAX_NAME_LEN];
    char notBefore[MAX_NAME_LEN];       /*cert is not valid until this date*/
} CertDetails;

#endif /* __READ_CERT_H__ */
