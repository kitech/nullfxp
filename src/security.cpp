// security.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-10-07 16:11:15 +0800
// Version: $Id$
// 

//---------------------------------------------------------------------------
// #include <vcl.h>
// #pragma hdrstop
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// #include "Common.h"
#include "security.h"
//---------------------------------------------------------------------------
// #pragma package(smart_init)
//---------------------------------------------------------------------------
#define PWALG_SIMPLE_INTERNAL 0x00
#define PWALG_SIMPLE_EXTERNAL 0x01
//---------------------------------------------------------------------------
std::string SimpleEncryptChar(unsigned char Ch)
{
    std::string str;
    const char *hexChars = PWALG_SIMPLE_STRING;
    Ch = (unsigned char)((~Ch) ^ PWALG_SIMPLE_MAGIC);

    // str += hexChars[((Ch & 0xF0) >> 4) + 1];
    // str += hexChars[((Ch & 0x0F) >> 0) + 1];
    str.append(1, hexChars[((Ch & 0xF0) >> 4)]);
    str.append(1, hexChars[((Ch & 0x0F) >> 0)]);

    // fprintf(stderr, "enc: %c -> %s \n", Ch, str.c_str());

    return str;
  // return
  //   PWALG_SIMPLE_STRING.SubString(((Ch & 0xF0) >> 4) + 1, 1) +
  //   PWALG_SIMPLE_STRING.SubString(((Ch & 0x0F) >> 0) + 1, 1);
}
//---------------------------------------------------------------------------
unsigned char SimpleDecryptNextChar(std::string &Str)
{
    // fprintf(stderr, "dec: %s\n", Str.c_str());
    std::string hexChars = PWALG_SIMPLE_STRING;
    if (Str.length() > 0) {
        unsigned char hi, lo;
        hi = hexChars.find(Str.c_str()[0]-1);
        lo = hexChars.find(Str.c_str()[1]-1);
        unsigned char Result = (unsigned char)
            ~((((hexChars.find(Str.c_str()[0])) << 4) + 
               ((hexChars.find(Str.c_str()[1])) << 0)) ^ PWALG_SIMPLE_MAGIC);
            // ~((((PWALG_SIMPLE_STRING.Pos(Str.c_str()[0])-1) << 4) +
            //    ((PWALG_SIMPLE_STRING.Pos(Str.c_str()[1])-1) << 0)) ^ PWALG_SIMPLE_MAGIC);
        // Str.Delete(1, 2);
        Str.erase(0, 2); 
        // fprintf(stderr, "dec: %s, %d-%d\n", Str.c_str(), hi, lo);
        return Result;
    }
    return 0x00;
}
//---------------------------------------------------------------------------
std::string EncryptPassword(std::string Password, std::string Key, int /* Algorithm */)
{
    std::string Result("");
    int Shift, Index;

    // if (!RandSeed) Randomize();
    srand(time(NULL));
    Password = Key + Password;
    Shift = (Password.length() < PWALG_SIMPLE_MAXLEN) ?
        (unsigned char)rand() % (PWALG_SIMPLE_MAXLEN - Password.length()) : 0;
    Result += SimpleEncryptChar((char)PWALG_SIMPLE_FLAG); // Flag
    Result += SimpleEncryptChar((char)PWALG_SIMPLE_INTERNAL); // Dummy
    Result += SimpleEncryptChar((char)Password.length());
    Result += SimpleEncryptChar((char)Shift);
    for (Index = 0; Index < Shift; Index++)
        Result += SimpleEncryptChar((unsigned char)rand()%256);
    for (Index = 0; Index < Password.length(); Index++)
        Result += SimpleEncryptChar(Password.c_str()[Index]);
    while (Result.length() < PWALG_SIMPLE_MAXLEN * 2)
        Result += SimpleEncryptChar((unsigned char)rand()%256);
    return Result;
}
//---------------------------------------------------------------------------
std::string DecryptPassword(std::string Password, std::string Key, int /* Algorithm */)
{
    std::string Result("");
    int Index;
    unsigned char Length, Flag;

    Flag = SimpleDecryptNextChar(Password);
    if (Flag == PWALG_SIMPLE_FLAG) {
        /* Dummy = */ SimpleDecryptNextChar(Password);
        Length = SimpleDecryptNextChar(Password);
    } else {
        Length = Flag;
    }
    // fprintf(stderr, "flag: %d, len: %d\n", Flag, Length);

    // Password.Delete(1, ((int)SimpleDecryptNextChar(Password))*2);
    Index = ((int)SimpleDecryptNextChar(Password));
    // fprintf(stderr, "erase: %d\n", Index);
    Password.erase(0, Index * 2);
    for (Index = 0; Index < Length; Index++)
        Result += (char)SimpleDecryptNextChar(Password);
    if (Flag == PWALG_SIMPLE_FLAG) {
        // if (Result.SubString(1, Key.Length()) != Key) Result = "";
        // else Result.Delete(1, Key.Length());
        if (Result.substr(0, Key.length()) != Key) Result = "";
        else Result.erase(0, Key.length());
    }
    return Result;
}

//---------------------------------------------------------------------------
std::string CharToHex(char Ch, bool UpperCase)
{
    static char UpperDigits[] = "0123456789ABCDEF";
    static char LowerDigits[] = "0123456789abcdef";

    const char * Digits = (UpperCase ? UpperDigits : LowerDigits);
    std::string Result;
    // Result.SetLength(2);
    // Result[1] = Digits[((unsigned char)Ch & 0xF0) >> 4];
    // Result[2] = Digits[ (unsigned char)Ch & 0x0F];
    Result.append(1, Digits[((unsigned char)Ch & 0xF0) >> 4]);
    Result.append(1, Digits[ (unsigned char)Ch & 0x0F]);

    return Result;
}

//---------------------------------------------------------------------------
std::string  StrToHex(const std::string Str, bool UpperCase = true, char Separator = '\0')
{
    std::string Result;
    for (int i = 1; i <= Str.length(); i++) {
        Result += CharToHex(Str[i], UpperCase);
        if ((Separator != '\0') && (i < Str.length())) {
            Result += Separator;
        }
    }

    return Result;
}

//---------------------------------------------------------------------------
std::string HexToStr(const std::string Hex)
{
    static std::string Digits = "0123456789ABCDEF";
    std::string Result;
    int L, P1, P2;
    L = Hex.length();
    if (L % 2 == 0) {
        for (int i = 1; i <= Hex.length(); i += 2) {
            P1 = Digits.find((char)toupper(Hex[i]));
            P2 = Digits.find((char)toupper(Hex[i + 1]));
            if (P1 <= 0 || P2 <= 0) {
                Result = "";
                break;
            } else {
                Result += static_cast<char>((P1 - 1) * 16 + P2 - 1);
            }
        }
    }

    return Result;
}

//---------------------------------------------------------------------------
std::string SetExternalEncryptedPassword(std::string Password)
{
    std::string Result;
    Result += SimpleEncryptChar((char)PWALG_SIMPLE_FLAG);
    Result += SimpleEncryptChar((char)PWALG_SIMPLE_EXTERNAL);
    Result += StrToHex(Password);
    return Result;
}
//---------------------------------------------------------------------------
bool GetExternalEncryptedPassword(std::string Encrypted, std::string & Password)
{
    bool Result =
        (SimpleDecryptNextChar(Encrypted) == PWALG_SIMPLE_FLAG) &&
        (SimpleDecryptNextChar(Encrypted) == PWALG_SIMPLE_EXTERNAL);
    if (Result) {
        Password = HexToStr(Encrypted);
    }
    return Result;
}
//---------------------------------------------------------------------------
