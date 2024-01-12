#include <iostream>
#include <pthread.h>
#include <string.h>
#include <bitset>
#include <cmath>
#include <map>

//This struct is for n child threads
struct vault
{
    std::string c;
    int dec;
    std::string cMSG;
    int nbits;
    int pos;
    pthread_mutex_t* bsem;
    pthread_cond_t* waitTurn;
    int* turn;
};

//This struct is for m child threads
struct deVault
{
    std::string cMSG;
    std::string decode;
    int nbits;
    int pos;
    std::map<std::string,std::string>mp;  //This is for map
    pthread_mutex_t* bsem2;
    pthread_cond_t* waitTurn2;
    int* turn2;
};

//Void function for n child threads
void* child_nThreads(void* arg)
{
    vault* vault_ptr = (vault *) arg;
    //Critical section end
    pthread_mutex_unlock(vault_ptr->bsem);

    std::bitset<8>a(vault_ptr->dec); //Convert decimal numbers to binary code in bits size
    std::string code = a.to_string(); //Store each binary codes in struct
    code = code.substr(8-vault_ptr->nbits);  //Reduce the binary code by bits size
    
    //Find the # of frequency of each binary number
    int freq;
    for (int j = 0; j < vault_ptr->cMSG.length(); j+=code.length())
    {
        std::string currentChar = vault_ptr->cMSG.substr(j, vault_ptr->nbits);
        for (int k = 0; k < currentChar.length(); k++)
        {
            if(currentChar == code)
            {
                currentChar = code + currentChar;
                freq++;  
            }
        }
    }
    //Print the output for character, Code, and Frequency
    std::cout << "Character: " << vault_ptr->c << ", " << "Code: " << code << ", " << "Frequency: " << freq << std::endl;
    
    //Critical section begin
    pthread_mutex_lock(vault_ptr->bsem);
    *(vault_ptr->turn) = *(vault_ptr->turn)+1;
    //Broadcasting
    pthread_cond_broadcast(vault_ptr->waitTurn);
    //Critical section end
    pthread_mutex_unlock(vault_ptr->bsem);

    return NULL;
}

//Void function for m child threads
void* child_mThreads(void* arg)
{
    deVault* deVault_ptr = (deVault *)arg;
    //Critical section end
    pthread_mutex_unlock(deVault_ptr->bsem2);
    
    //Slice the compressed message by the number of bits
    std::string binMSG = deVault_ptr->cMSG.substr(deVault_ptr->pos*deVault_ptr->nbits, deVault_ptr->nbits);
    //Map each binary codes with characters
    deVault_ptr->decode = deVault_ptr->mp[binMSG];
    
    //Critical section begin
    pthread_mutex_lock(deVault_ptr->bsem2);
    *(deVault_ptr->turn2) = *(deVault_ptr->turn2)+1;
    //Broadcasting
    pthread_cond_broadcast(deVault_ptr->waitTurn2);
    //Critical section end
    pthread_mutex_unlock(deVault_ptr->bsem2);
    return NULL;
}

int main()
{
    static pthread_mutex_t bsem;      //Create this mutex for n child threads
    static pthread_cond_t waitTurn;   //Create this condition wait for n child threads
    
    int nSymbols;
    std::cin >> nSymbols;            //Get user input for # the of symbols
    std::string character[nSymbols]; //A place to store character values
    int decimal[nSymbols];           //A place to store decimal values
    
    std::string text;
    std::getline(std::cin, text);
    //Store each characters and decimal values in array
    for (int i = 0; i < nSymbols; i++)
    {
        std::getline(std::cin, text);
        character[i] = text.substr(0,1);
        decimal[i] = stoi(text.substr(2));
    }
    
    //Get the compressed MSG from the user
    std::string cMSG;
    std::getline(std::cin, text);
    cMSG = text;

    //Finding the greatest Base 10 code
    int gstBase10 = 0;
    for (int i = 0; i < nSymbols; i++)
    {
        if(decimal[i] > gstBase10)
        {
            gstBase10 = decimal[i];
        }
    }
    int nbits = ceil(log2(gstBase10+1));     //Calculate the # of bits
    int numOfBitsMSG = cMSG.length();        //The size of compressed message
    int numOfCharMSG = numOfBitsMSG/nbits+1; //The number of characters in compressed message by bits size

    //Set all variables into 1 Arg Struct
    int turn = 0;
    vault nChildArg;
    nChildArg.bsem = &bsem;
    nChildArg.waitTurn = &waitTurn;
    nChildArg.nbits = nbits;
    nChildArg.cMSG = cMSG;
    nChildArg.turn = &turn;

    std::cout << "Alphabet:" << std::endl;
    //Create n child threads
    pthread_t th[nSymbols]; //The number of threads from user input
    for(int i = 0; i < nSymbols; i++)
    {
        //Critical section begin
        pthread_mutex_lock(&bsem);
        //Threads waits patiently
        while(i != turn)
        {
            pthread_cond_wait(&waitTurn, &bsem);
        }
        nChildArg.pos = i;          //Initial position
        nChildArg.c = character[i]; //Send characters to 1 arg struct
        nChildArg.dec = decimal[i]; //Send decimals to 1 arg struct
        if(pthread_create(&th[i], NULL, child_nThreads, &nChildArg))
        {
            perror("Failed to create thread");
        }
    }
    //Join n child threads
    for(int i = 0; i < nSymbols; i++)
    {
        if(pthread_join(th[i], NULL))
        {
            perror("Failed to join thread");
        }
    }

    //Decompressing part begins

    //Binary codes and letters into map 
    std::map<std::string, std::string> mapping;
    std::string binCode;
    for(int i = 0; i < nSymbols; i++)
    {
        std::bitset<8>a(decimal[i]); 
        std::string binCode = a.to_string(); 
        binCode = binCode.substr(8-nbits);
        //Map each binary codes with characters
        mapping.insert(std::pair<std::string,std::string>(binCode, character[i]));
    }

    static pthread_mutex_t bsem2;     //Create this mutex for m child threads
    static pthread_cond_t waitTurn2;  //Create this condition wait for m child threads

    //Set all variables into 1 Arg Struct
    deVault mChildArg;
    int turn2 = 0;
    mChildArg.bsem2 = &bsem2;
    mChildArg.waitTurn2 = &waitTurn2;
    mChildArg.nbits = nbits;
    mChildArg.cMSG = cMSG;
    mChildArg.mp = mapping;
    mChildArg.turn2 = &turn2;
    mChildArg.decode;

    std::cout << "" << std::endl;
    std::cout << "Decompressed message: ";
    //Create m child threads for decompressing
    pthread_t th2[numOfCharMSG];    //The number of threads from the # of slices in compressesd message
    for(int i = 0; i < numOfCharMSG; i++)
    {
        //Critical section begin
        pthread_mutex_lock(&bsem2);
        //Threads waits patiently
        while(i != turn2)
        {
            pthread_cond_wait(&waitTurn2, &bsem2);
        }
        mChildArg.pos = i;  //Initial position
        if(pthread_create(&th2[i], NULL, &child_mThreads, &mChildArg))
        {
            perror("Failed to create thread");
        }
        std::cout << mChildArg.decode; //Print the output for decompressed message
    }

    //Join m child threads
    for (int i = 0; i < numOfCharMSG; i++)
    {
        if(pthread_join(th2[i], NULL))
        {
            perror("Failed to join thread");
        }
    }

    return 0;
}