#!/bin/bash
# 校园导游咨询系统编译脚本

echo "开始编译校园导游咨询系统..."

# 创建build目录
mkdir -p build
cd build

# 检查qmake是否可用
if command -v qmake &> /dev/null; then
    echo "使用qmake编译..."
    qmake ../CampusGuide.pro
    if [ $? -eq 0 ]; then
        make
        if [ $? -eq 0 ]; then
            echo "编译成功！可执行文件: build/CampusGuide"
            echo "运行方式: cd build && ./CampusGuide"
        else
            echo "make失败"
            exit 1
        fi
    else
        echo "qmake失败"
        exit 1
    fi
elif command -v qmake6 &> /dev/null; then
    echo "使用qmake6编译..."
    qmake6 ../CampusGuide.pro
    if [ $? -eq 0 ]; then
        make
        if [ $? -eq 0 ]; then
            echo "编译成功！可执行文件: build/CampusGuide"
            echo "运行方式: cd build && ./CampusGuide"
        else
            echo "make失败"
            exit 1
        fi
    else
        echo "qmake6失败"
        exit 1
    fi
else
    echo "错误: 未找到qmake或qmake6"
    echo "尝试使用CMake..."
    cmake ..
    if [ $? -eq 0 ]; then
        make
        if [ $? -eq 0 ]; then
            echo "编译成功！可执行文件: build/CampusGuide"
            echo "运行方式: cd build && ./CampusGuide"
        else
            echo "make失败"
            exit 1
        fi
    else
        echo "CMake也失败，请检查Qt安装"
        exit 1
    fi
fi

