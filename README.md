# v8-custom-global-variables
V8 嵌入 C++ 应用（JavaScript runtime）并自定义全局变量的例子


下载后使用 clion 打开编译后，运行以下命令：

```bash
./demo ./app.js
```

定义了全局变量 __version 为字符串 1.0.0，全局函数 getVersion 返回 1.0.0，全局打印 log 函数，全局对象 node ,node.version 返回 1.0.0

