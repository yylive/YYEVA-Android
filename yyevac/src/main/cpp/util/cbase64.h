//
// Created by zengjiale on 2022/8/26.
//

#ifndef YYEVA_CBASE64_H
#define YYEVA_CBASE64_H

#include <iostream>
#include <string>
#include <string.h>
#include <vector>

using namespace std;
class CBase64 {
public:
    static string encodeBase64(unsigned char *input , int input_len) {
        const char * code="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        unsigned char input_char[3];
        char output_char[5];
        int output_num;
        string output_str="";
        int near=input_len%3;

        for (int i=0;i<input_len;i+=3) {
            memset(input_char ,0,3);
            memset(output_char ,61,5);
            if (i+3 <= input_len) {
                memcpy(input_char,input+i,3);
            } else {
                // 不够凑成3个byte
                memcpy(input_char,input+i,input_len-i);
                output_num=((int)input_char[0]<<16)+((int)input_char[1]<<8)+(int)input_char[2];

                if (near==1) {
                    output_char[0]=code[((output_num>>18) & 0x3f)];
                    output_char[1]=code[((output_num>>12) & 0x3f)];
                    output_char[2]='=';
                    output_char[3]='=';
                    output_char[4]='\0';
                }

                if (near==2) {
                    output_char[0]=code[((output_num>>18) & 0x3f)];
                    output_char[1]=code[((output_num>>12) & 0x3f)];
                    output_char[2]=code[((output_num>>6) & 0x3f)];;
                    output_char[3]='=';
                    output_char[4]='\0';
                }

                output_str.append(output_char);
                break;
            }

            output_num=((int)input_char[0]<<16)+((int)input_char[1]<<8)+(int)input_char[2];
            output_char[0]=code[((output_num>>18) & 0x3f)];
            output_char[1]=code[((output_num>>12) & 0x3f)];
            output_char[2]=code[((output_num>>6) & 0x3f)];
            output_char[3]=code[((output_num) & 0x3f)];
            output_char[4]='\0';
            output_str.append(output_char);
        }

        return output_str;
    }

    static string decodeBase64(string input) {
        unsigned char input_char[4];
        unsigned char output_char[4];
        int output_num=0;
        int k=0;
        string output_str="";

        for (unsigned int i=0;i<input.size();i++) {
            input_char[k]=indexOfCode(input[i]);
            k++;
            if (k==4) {
                output_num= ((int)input_char[0]<<18)+((int)input_char[1]<<12)+((int)input_char[2]<<6)+((int)input_char[3]);
                output_char[0]=(unsigned char)((output_num & 0x00FF0000 )>> 16) ;
                output_char[1]=(unsigned char)((output_num & 0x0000FF00 ) >> 8) ;
                output_char[2]=(unsigned char)(output_num & 0x000000FF) ;
                output_char[3]='\0';
                output_str.append((char *)output_char);
                k=0;
            }
        }

        return output_str;
    }

    static int indexOfCode(const char c) {
        const char * code="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (unsigned int i=0;i<64;i++) {
            if (code[i]==c)
                return i;
        }
        return 0;
    }
    constexpr static const unsigned char from_base64[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                                 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255,  62, 255,  63,
                                                 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255, 255, 255, 255, 255,
                                                 255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
                                                 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255,  63,
                                                 255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
                                                 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255};


    static std::vector<char> base64_decode(std::string encoded_string) {
        size_t encoded_size = encoded_string.size();
        std::vector<char> ret;
        ret.reserve(3*encoded_size/4);

        for (size_t i=0; i<encoded_size; i += 4)
        {
            unsigned char b4[4];
            b4[0] = from_base64[encoded_string[i+0]];
            b4[1] = from_base64[encoded_string[i+1]];
            b4[2] = from_base64[encoded_string[i+2]];
            b4[3] = from_base64[encoded_string[i+3]];

            unsigned char b3[3];
            b3[0] = ((b4[0] & 0x3f) << 2) + ((b4[1] & 0x30) >> 4);
            b3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2);
            b3[2] = ((b4[2] & 0x03) << 6) + ((b4[3] & 0x3f));

            ret.push_back(b3[0]);
            ret.push_back(b3[1]);
            ret.push_back(b3[2]);
        }

        return ret;
    }
};

#endif //YYEVA_CBASE64_H
