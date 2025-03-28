#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

char *sha1_hash(char *input_url, char *hashed_url){
    unsigned char hashed_160bits[20];   // always 20byte SHA1
    char hashed_hex[41];                // 20 byte * 2
    int i;

    // convert value, size of convert value, save value converted
    SHA1(input_url, strlen(input_url), hashed_160bits);

    //save hashed url in hashed_hex converted to hex char
    for(int i = 0; i < sizeof(hashed_160bits); i++){
        sprintf(hashed_hex + i*2, "%02x", hashed_160bits[i]);
    }
    hashed_hex[40] = '\0';
    strcpy(hashed_url, hashed_hex);

    return hashed_url;
}