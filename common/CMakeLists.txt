# 查找当前目录下的所有源文件，并将名称保存到 DIR_SRC_FILES 变量中
aux_source_directory(. DIR_SRC_FILES)

# 添加编译选项
add_compile_options(-fPIC)

# 生成静态库
add_library(common ${DIR_SRC_FILES})