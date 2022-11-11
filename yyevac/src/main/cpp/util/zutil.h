//
// Created by zengjiale on 2022/8/26.
//

#ifndef YYEVA_ZUTIL_H
#define YYEVA_ZUTIL_H
#include <zlib.h>
#include <string>

#define DEBUGPRINT(tag, msg) __android_log_print(ANDROID_LOG_DEBUG, tag, "%s", msg)

using namespace std;
class ZUtil {
public:
    static string zUncompress(string str) {
        z_stream zs;
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = Z_NULL;
        memset(&zs, 0, sizeof(zs));
        inflateInit(&zs);
        zs.next_in = (Bytef*)str.data();
        zs.avail_in = str.size();
        int ret;
        char outbuffer[65534];
        string outString = "";
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
            ret = inflate(&zs, Z_NO_FLUSH);
            outString.append(outbuffer);
        } while (ret == Z_STREAM_END);
        inflateEnd(&zs);

        return outString;
    }

    static string zUncompress(vector<char> str) {
        z_stream zs;
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = Z_NULL;
        memset(&zs, 0, sizeof(zs));
        inflateInit(&zs);
        zs.next_in = (Bytef*)str.data();
        zs.avail_in = str.size();
        int ret;
//        char outbuffer[32768];
        char outbuffer[65534];
        string outString = "";
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
            ret = inflate(&zs, Z_NO_FLUSH);
            outString.append(outbuffer);
        } while (ret == Z_OK);
        inflateEnd(&zs);
        str.clear();
        return outString;
    }
};


#endif //YYEVA_ZUTIL_H
