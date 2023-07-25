### 项目介绍
参考mit http://web.mit.edu/~yandros/doc/craft-text-editing/ 完成的vim项目\
`text_display.h` 封装所有控制台显示逻辑，使用windows console API直接对控制台缓冲区进行操作\
`sub_editor.h` 封装所以编辑器逻辑，包含文本，光标，操作栈等数据结构和相应的操作\
`MyText.cpp` 程序入口，包含vim-like的指令逻辑以及对上述两者类的创建与调用

### 使用指南
`:q` 回车退出程序\
`:o[pen]+空格+filename` 打开文件\
如果filename输错直接按回车，再重新从`:o[pen]`开始输入，`:w` 同理保存\
文件名输错下方会显示`Error File`\
`h`, `j`, `k`, `l` 分别为光标左下上右移动\
`/` 搜索文件\
`u` 为撤销，`r` 为重做\
`i` 进入编辑模式\
`x` 删除字母\
下方状态栏依次显示：当前模式 当前指令（仅回显指令）当前文件
