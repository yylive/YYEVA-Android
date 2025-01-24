# YY-EVA <sup>Android</sup>

简体中文 | [English](./README_en.md)

> 轻量级 高性能 跨平台 MP4 礼物播放器

## 支持本项目

请支持我们的项目，点击[⭐⭐⭐](https://github.com/yylive/YYEVA), 让更多的人看到该项目


## 案例演示

<img src="./resource/out_3.gif" width = "399" height = "428" alt="图片名称" align=center /> 

## 介绍

+ YYEVAPlayer 是一个轻量的动画渲染库。通过[这里](https://github.com/yylive/YYEVA/blob/main/YYEVA%E8%AE%BE%E8%AE%A1%E8%A7%84%E8%8C%83.md)导出动画文件
+ 通过[这里](https://github.com/yylive/YYEVA/tree/main/AEP/demo_aep)可以获取设计的测试资源文件   
+ YYEVA-Android 使用Native Opengles 渲染视频，为你提供高性能、低开销的动画体验。

## 平台支持
+ 支持 [Android](https://github.com/yylive/YYEVA-Android)、[IOS](https://github.com/yylive/YYEVA-iOS)、[Web](https://github.com/yylive/YYEVA-Web)   点击了解详细接入   
+ 资源制作的AE插件使用规范 [详情](https://github.com/yylive/YYEVA/tree/main/AEP)
+ 数据结构定义 [详情](https://github.com/yylive/YYEVA/blob/main/%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84.md)
+ 项目相关文章、设计规范等 [详情](https://github.com/yylive/YYEVA)

## 用法

我们在这里介绍 YYEVA-Android 的用法。想要知道如何导出动画，点击[这里](https://github.com/yylive/YYEVA/blob/main/YYEVA%E8%AE%BE%E8%AE%A1%E8%A7%84%E8%8C%83.md)。

### 使用Gradle安装依赖
build.gradle
```groovy
allprojects {
	repositories {
		maven { url 'https://jitpack.io' }
	}
}
```

```groovy
dependencies {
	implementation 'com.github.yylive.YYEVA-Android:yyeva:1.0.17'
}
// 2.0.0-beta版本
dependencies {
    implementation 'com.github.yylive.YYEVA-Android:yyeva:2.0.0-beta'
}
```

### 放置混合 mp4 文件 在Assets中
如需要使用SurfaceView可以使用EvaAnimView，需要使用TextureView可以使用EvaAnimViewV3，demo使用EvaAnimViewV3

替换元素接口
```kotlin
interface IEvaFetchResource {
    // 获取图片 (暂时不支持Bitmap.Config.ALPHA_8 主要是因为一些机型opengl兼容问题)
    fun setImage(resource: EvaResource, result:(Bitmap?) -> Unit)

    // 获取文字
    fun setText(resource: EvaResource, result:(String?) -> Unit)

    // 资源释放通知
    fun releaseSrc(resources: List<EvaResource>)

    //speed为倍速
    fun setVideoFps(fps: Int, speed: Float = 1.0f)
    //设置资源颜色区域和透明区域的对齐方式
    /*
    const val VIDEO_MODE_SPLIT_HORIZONTAL = 1 // 视频左右对齐（alpha左\rgb右）
    const val VIDEO_MODE_SPLIT_VERTICAL = 2 // 视频上下对齐（alpha上\rgb下）
    const val VIDEO_MODE_SPLIT_HORIZONTAL_REVERSE = 3 // 视频左右对齐（rgb左\alpha右）
    const val VIDEO_MODE_SPLIT_VERTICAL_REVERSE = 4 // 视频上下对齐（rgb上\alpha下）
    const val YYEVAColorRegion_AlphaMP4_alphaHalfRightTop = 5 // 视频上下对齐（rgb上\alpha下）1/4 elpha
     */
    fun setVideoMode(mode: Int)
    //设置起始播放位置 毫秒
    //硬解某些机型会有跳帧前几帧解析异常的问题，不建议使用。
    fun setStartPoint(startPoint: Long)
    //设置静音
    fun setMute(isMute: Boolean)
    //是否不透明度mp4
    fun setNormalMp4(isNormalMp4: Boolean)
    //是否停留在最后一帧
    fun setLastFrame(isSetLastFrame: Boolean)
    //设置日志打印
    fun setLog(log: IELog)
}
```
具体实现可以参照EvaDemoActivity的代码实验，替换自身mp4中的元素。

播放使用IEvaAnimView接口
```kotlin
interface IEvaAnimView {
    ...
    //播放文件
    fun startPlay(file: File)
    //播放本地文件
    fun startPlay(assetManager: AssetManager, assetsPath: String)
    //停止播放
    fun stopPlay()
    //是否正在运行
    fun isRunning(): Boolean
    //循环播放
    fun setLoop(playLoop: Int)
    //设置背景图
    fun setBgImage(bg: Bitmap)
    ...
}
```
 

2.0.0-beta播放使用OptionParams类配置
```kotlin
class OptionParams {
    var frameRate = 30
    var playCount = 1
    var isMute = false
    var isRemoteService = true  //使用多进程
    var mp4Address = ""
    var scaleType = 1   // 1=>裁剪居中， 2=>全屏拉伸  3=>原资源大小
    var filterType = "" //高清算法 hermite lagrange
}
```

### 代码

## QQ交流群
![qqgroup](https://github.com/yylive/YYEVA/blob/main/img/qqgroup.png)

## 鸣谢 
+ 感谢 [vap](https://github.com/Tencent/vap) 优秀的混合渲染方案、项目Render混合部分重用了 vap的方案

## Dev Team
<table>
  <tbody>
    <tr>
      <td align="center" valign="top">
        <img style="border-radius:8px" width="80" height="80" src="https://avatars.githubusercontent.com/u/10419150?v=4">
        <br>
        <a href="https://github.com/cangwang">Cangwang</a>
      </td>
     </tr>
  </tbody>
</table>


 
