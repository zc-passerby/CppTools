# 查找当前目录下的所有源文件，并将名称保存到 DIR_SRC_FILES 变量中
aux_source_directory(fsEslSrc DIR_SRC_FILES)

include_directories("${PROJECT_SOURCE_DIR}/fsEslHelper/fsEslSrc/include")

# 生成链接库
add_library(fsesl SHARED ${DIR_SRC_FILES} ../readConfig/readconfig.h)