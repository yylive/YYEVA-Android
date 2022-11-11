// IRemoteCallback.aidl
package com.yy.yyevav2.service;

// Declare any non-default types here with import statements
//解码进程回调
interface IRemoteCallback {
   oneway void onVideoRender(int frameIndex);
   oneway void onVideoComplete();
   oneway void onVideoDestroy();
   oneway void onFailed(int errorType, String errorMsg);
}