#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>


///////////////////////////////////////////////////////////////////////
// sha1_hash                                                         //
///////////////////////////////////////////////////////////////////////
// Input:                                                            //
//   - input_url: the original string to be hashed                   //
//   - hashed_url: buffer to store the resulting SHA-1 hex string    //
// Output:                                                           //
//   - Returns a pointer to the hashed_url buffer                    //
// Purpose:                                                          //
//   - Computes the SHA-1 hash of the input_url                      //
//   - Converts the 20-byte binary hash to a 40-character hex string //
//   - Stores the result in hashed_url                               //
///////////////////////////////////////////////////////////////////////
char *sha1_hash(char *input_url, char *hashed_url){
    unsigned char hashed_160bits[20];   // always 20byte SHA1
    char hashed_hex[41];                // 20 byte * 2
    int i;

    // convert value, size of convert value, save value converted
    SHA1((const unsigned char *)input_url, strlen(input_url), hashed_160bits);

    //save hashed url in hashed_hex converted to hex char
    for(int i = 0; i < sizeof(hashed_160bits); i++){
        sprintf(hashed_hex + i*2, "%02x", hashed_160bits[i]);
    }
    hashed_hex[40] = '\0';
    strcpy(hashed_url, hashed_hex);

    return hashed_url;
}

char *getName_hashed(char *hashed_url, int offset, int length){
    
}