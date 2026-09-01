#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_128_KEY_SIZE 16
#define AES_BLOCK_SIZE 16

/*
 * Note: pay attention to auto padding or not.
 * 20250320
 */
// Function to handle errors
void handleErrors(void) {
    fprintf(stderr, "An error occurred\n");
    exit(EXIT_FAILURE);
}

// Function to encrypt data using AES-128-CBC
int encrypt(unsigned char *plaintext, int plaintext_len,
            unsigned char *key, unsigned char *iv,
            unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();
    
    //padding by myself, or else we need to pass the correct ciphertext_len for decrypt,
    //or else EVP_DecryptFinal_ex() will always fail
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    
    // Initialize encryption operation for AES-128-CBC
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1)
        handleErrors();

    // Encrypt the plaintext
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1)
        handleErrors();
    ciphertext_len = len;

    // Finalize encryption (adds padding if needed)
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1)
        handleErrors();
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

// Function to decrypt data using AES-128-CBC
int decrypt(unsigned char *ciphertext, int ciphertext_len,
            unsigned char *key, unsigned char *iv,
            unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    // Create and initialize the context
    if (!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    EVP_CIPHER_CTX_set_padding(ctx, 0);
 
    // Initialize decryption operation for AES-128-CBC
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv) != 1)
        handleErrors();

    // Decrypt the ciphertext
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1)
        handleErrors();
    plaintext_len = len;

    // Finalize decryption (removes padding if needed)
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1)
        handleErrors();
    plaintext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return plaintext_len;
}

int main(void) {
    // 16 bytes = 128 bits key size
    unsigned char key[AES_128_KEY_SIZE];
    unsigned char iv[AES_BLOCK_SIZE];

    // Generate a random key and IV
    if (RAND_bytes(key, sizeof(key)) != 1 || RAND_bytes(iv, sizeof(iv)) != 1) {
        fprintf(stderr, "Failed to generate key or IV\n");
        return 1;
    }

    // Input plaintext
    unsigned char plaintext[80] = "hello world,fffffffffffffffffjjjjjjjjjjj";
    unsigned char ciphertext[128];
    unsigned char decryptedtext[128];

    printf("Plaintext: %s\n", plaintext);

    // Encrypt the plaintext
    int ciphertext_len = encrypt(plaintext, sizeof(plaintext), key, iv, ciphertext);

    printf("Ciphertext (hex): ciphertext_len = %d\n", ciphertext_len);
    for (int i = 0; i < ciphertext_len; i++)
        printf("%02x", ciphertext[i]);
    printf("\n");

    // Decrypt the ciphertext
    int decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);

    // Null-terminate the decrypted string
    decryptedtext[decryptedtext_len] = '\0';

    printf("Decrypted text: %s\n", decryptedtext);

    return 0;
}

