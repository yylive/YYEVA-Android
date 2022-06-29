# YY-EVA <sup>Android</sup>
> 轻量级 高性能 跨平台 MP4 礼物播放器

## 支持本项目

请支持我们的项目，点击 GitHub Star, 让更多的人看到该项目

## 介绍

+ YYEVAPlayer 是一个轻量的动画渲染库。你可以使用工具从 Adobe After Effects 中导出动画文件，然后使用 YYEVAPlayer 在移动设备上渲染并播放。

+ YYEVA-Android 使用Native Opengles 渲染视频，为你提供高性能、低开销的动画体验。

## 平台支持
+ 支持 [Android](https://github.com/YY-LIVE/YYEVA-Android)、[IOS](https://github.com/YY-LIVE/YYEVA-iOS)、[Web](https://github.com/YY-LIVE/YYEVA-Web)、百度小程序  点击了解详细接入
+ 素材制作 AE插件 [详情](https://github.com/YY-LIVE/YYEVA-AE-Plugin)
+ 数据结构定义 [详情](https://github.com/YY-LIVE/YYEVA-format)
+ 项目相关文章、设计规范等 [详情](https://github.com/YY-LIVE/YYEVA-docs)

## 用法

我们在这里介绍 YYEVA-Android 的用法。想要知道如何导出动画，点击[这里](https://github.com/YY-LIVE/YYEVA-docs/blob/main/docs/YYEVA%E8%AE%BE%E8%AE%A1%E8%A7%84%E8%8C%83.md)。

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
	implementation 'com.github.YY-LIVE.YYEVA-Android:yyeva:1.0.0_alpha_0.0.4'
}
```

### 放置混合 mp4 文件 在Assets中
具体实现可以参照EvaDemoActivity的代码实验，替换自身mp4中的元素。
### 代码

## QQ交流群
![qqgroup](https://github.com/YY-LIVE/YYEVA-Web/blob/main/docs/assets/qqgroup.png)

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


 
