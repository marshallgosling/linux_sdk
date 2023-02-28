
## 目录结构概述

**example**目录包括了Agora Linux Gateway 的示例代码，编译脚本，编译工具等。你可以快速地编译得到若干个可执行文件，运行它们并体验agoraSDK的音视频码流传输功能。而其中的示例代码是你学习agoraSDK使用的绝佳范例，强烈建议你仔细阅读其中的相关sample，可以帮你快速上手如何正确调用agora的api接口实现相关的功能。具体如何编译、运行并阅读学习相关的内容，你可以阅读该目录下的ReadMe文档，指引你一步步继续。

**agora_sdk**目录包括了Agora RTSA_Pro的so库文件以及相关的头文件。相关的头文件中附有大量的注释，可以帮助你快速理解每个API的功能及所需参数。你可以结合 https://docs.agora.io/en/server-gateway/overview/product-overview?platform=linux-cpp  上的API文档，学习掌握相关的API。

**third_party**目录包括了一些有用的第三方库，可以认为这部分与SDK本身无关，你无需过分关注这一目录。

**CENTOS7**如果使用CENTOS7，需要修改CMAKELISTS.txt，增加 -std=c++11， 使用c++ 11标准
```
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++11 -fno-omit-frame-pointer -Wno-unused-command-line-argument ${sanitizer}")

```
**上述** 文件夹中均有更加详细的ReadMe说明，强烈建议你仔细阅读每个ReadMe文档，可以帮助你更顺利的使用Agora Linux Gateway！