// IServiceInterface.aidl
package com.yy.yyevav2.service;

import android.view.Surface;
import android.graphics.Bitmap;
import com.yy.yyevav2.service.IRemoteCallback;
// Declare any non-default types here with import statements

interface IServiceInterface {
    /**
     * Demonstrates some basic types that you can use as parameters
     * and return values in AIDL.
     */

    int remoteCreatePlayer(String tag, String optionParams);
    int remotePlay(String optionParams);
    boolean remoteIsRunning(int nativePlayer);
    int remoteStop(int nativePlayer);
    int remoteRelease(int nativePlayer);
    int remoteDestroySurface(int nativePlayer);
    int remoteSetSurface(int nativePlayer, in Surface surface);
    void remoteSetDecodeSurface(int nativePlayer, in Surface surface);
    void remoteUpdateViewPoint(int nativePlayer, int width, int height);
    void remoteSetSourceParams(String params);
    void remoteAddEffect(String name, in Bitmap bitmap, String scaleMode);
    void remoteSetEffectParams(String params);
    void remoteRenderData();
    void remoteSetPlayParams(String params);
    void remoteTouchPoint(int x, int y);
    //解码进程回调
    oneway void registerCallback(in IRemoteCallback callback);
}