# bin2txt
# 使用方法
```
-c path/to/txt path/to/bin baseOffset
```
将文本导入bin文件。

例：
txtFile
```
0x6d8	电击匕首      
0x6eb	Mk.2手枪      
0x6fc	Operator          
```

每行偏移和字符串以制表符分隔
字符串将覆写到bin文件baseOffset+偏移的位置。

```
-e path/to/bin
```
从bin文件中提取偏移和字符串表输出到txt文件

例：
输入
```
-e scenerio.bin
```
将在程序目录下输出一个scenerio.bin.txt文件
