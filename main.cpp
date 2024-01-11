#include <string.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <cmath>
#include <bitset>
#include <pthread.h>
#include <map>

struct vault
{
    std::string c;
    int dec;
    std::string cMSG;
    std::string code;
    int nbits;
    int freq;
};

struct deVault
{
    std::string cMSG;
    std::string decode;
    int nbits;
    int data;
    std::map<std::string,std::string>mp;  //This is for map
};

void* functionChT (void* arg)
{
    struct vault *vault_ptr = (vault *)arg;
    {
        std::bitset<8>a(vault_ptr->dec); //Convert decimal numbers to binary code in # of bits
        vault_ptr->code = a.to_string(); //Store each binary code in struct
        vault_ptr->code = vault_ptr->code.substr(8-vault_ptr->nbits);  //Reduce the binary code by # of bits

        //Find the # of frequency of each binary number
        for (int j = 0; j < vault_ptr->cMSG.length(); j+=vault_ptr->code.length())
        {
            std::string currentChar = vault_ptr->cMSG.substr(j, vault_ptr->nbits);
            for (int k = 0; k < currentChar.length(); k++)
            {
                if(currentChar == vault_ptr->code)
                {
                    currentChar = vault_ptr->code + currentChar;
                    vault_ptr->freq++;    
                }
            }
        }          
    }  
    return NULL;
}

void* decodeMSG (void* arg)
{
    struct deVault *deVault_ptr =  (deVault *)arg;
    {
        std::string binMSG = deVault_ptr->cMSG.substr(deVault_ptr->data*deVault_ptr->nbits, deVault_ptr->nbits);
        deVault_ptr->decode = deVault_ptr->mp[binMSG];
    }
    return NULL;
}

int main(int argc, char const *argv[])
{
    int nSymbols;
    std::cin >> nSymbols; //Input for the # of symbols in the alphabet
    vault *alpha = new vault[nSymbols];
    pthread_t th[nSymbols];
    std::string text;
    std::getline(std::cin, text);
    //Store each characters and decimal values in struct
    for (int i = 0; i < nSymbols; i++)
    {
        std::getline(std::cin, text);
        alpha[i].c = text.substr(0,1);
        alpha[i].dec = stoi(text.substr(2));
    }

    std::string cMSG;
    std::getline(std::cin, text);
    cMSG = text;

    int gstBase10 = 0;
    //Finding the greatest Base 10 code
    for (int i = 0; i < nSymbols; i++)
    {
        if(alpha[i].dec > gstBase10)
        {
            gstBase10 = alpha[i].dec;
        }
    }
    int nbits = ceil(log2(gstBase10+1));  //Calculate the # of bits
    int numOfBitsMSG = cMSG.length();
    int numOfBitsCode = nbits;
    int numOfCharMSG = numOfBitsMSG/numOfBitsCode;

    //Create Thread
    for(int i = 0; i < nSymbols; i++)
    {
        alpha[i].c;
        alpha[i].dec;
        alpha[i].nbits = nbits;
        alpha[i].cMSG = cMSG;
        alpha[i].freq;
        if (pthread_create(&th[i], NULL, &functionChT, &alpha[i]) != 0)
        {
            perror("Failed to create thread");
        }
    }

    // Join threads
    for ( int i = 0; i < nSymbols; i++)
    {
        if (pthread_join(th[i], nullptr) != 0)
        {
            perror("Failed to join thread");
        }
    }

    deVault *beta = new deVault[numOfCharMSG];
    pthread_t th2[numOfCharMSG];

    //Binary codes and letters into map 
    std::map<std::string, std::string> mapping;
    for(int i = 0; i < nSymbols; i++)
    {
        mapping.insert(std::pair<std::string,std::string>(alpha[i].code,alpha[i].c));
    }
    
    //Create Thread for Decompressing
    for(int i = 0; i < numOfCharMSG; i++)
    {
        beta[i].data = i;
        beta[i].nbits = nbits;
        beta[i].cMSG = cMSG;
        beta[i].decode;
        beta[i].mp = mapping;
        if (pthread_create(&th2[i], NULL, &decodeMSG, &beta[i]) != 0)
        {
            perror("Failed to create thread");
        }
    }

    // Join threads
    for ( int i = 0; i < numOfCharMSG; i++)
    {
        if (pthread_join(th2[i], nullptr) != 0)
        {
            perror("Failed to join thread");
        }
    }
    
    //Print the output
    std::cout << "Alphabet: " << std::endl;   
    for (int i = 0; i < nSymbols; i++)
    {
        std::cout << "Character: " << alpha[i].c << ", " << "Code: " << alpha[i].code << ", " << "Frequency: " << alpha[i].freq << std::endl;
    }
    
    std::cout << "" << std::endl;
    std::cout << "Decompressed Message: ";
    for(int i = 0; i < numOfCharMSG; i++)
    {
        std::cout << beta[i].decode;
    }
    
    delete [] alpha;
    delete [] beta;
    return 0;
}