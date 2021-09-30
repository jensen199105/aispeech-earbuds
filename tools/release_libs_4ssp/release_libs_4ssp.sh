#!/bin/bash

function usage()
{
    echo "usage:"
    echo "  $0 [options]"
    echo "参数解释:"
    echo "--res  设置资源文件的根目录"
    echo "       默认值为 examples/lite-wakeup/res/wmej_gddh-fbank24-30ms-fsmn-linear-e2e"
    echo "       可在本脚本中手动修改默认值"
    echo "例子1. lite-fesp$ tools/release_libs_4ssp/release_libs_4ssp.sh --res examples/lite-wakeup/res/test"
    echo "例子2. release_libs_4ssp$ ./release_libs_4ssp.sh --res ../../examples/lite-wakeup/res/test"
    exit
}

# 编译 lite-wakeup
# 参数说明
# param1: lite-fesp 代码根目录, 必选
# param2: resouce.c 的绝对路径, 必选
function build_lite_wakeup()
{
    src_root=$1
    target_resource=$2
    echo "lite_fesp src root is "${src_root}
    echo "target_resource: " ${target_resource}

    if [ ! -d "${src_root}" ]; then
        echo -e "\e[1;31;7m 根目录[${src_root}]不存在 \e[0m"
        exit
    fi

    if [ ! -e "${target_resource}" ]; then
        echo -e "\e[1;31;7m 资源文件[${target_resource}]不存在 \e[0m"
        exit
    fi

    # change_source_file
    HISTORY_DIR=`pwd`
    cd ${src_root}
    make clean

    # make clean 清除了软连接，因此这里要重新建立
    rm -f ${src_root}/examples/lite-wakeup/resource.c
    ln -s $target_resource ${src_root}/examples/lite-wakeup/resource.c

    echo `ls -al ${src_root}/examples/lite-wakeup/ | grep resource.c`
    make all

    cd ${HISTORY_DIR}    # 改变了目录之后,要切回去，否则会找不到在当前目录下引用的脚本
}

##############################################################################
SCRIPT_DIR=`dirname \`pwd\`/$0`
LITE_FESP="${SCRIPT_DIR}/../.."

##############################################################################
# 设置默认参数, 可根据需要在本地做修改, 绝对路径
INPUT_RES=${LITE_FESP}/examples/lite-wakeup/res/wmej_gddh-fbank24-30ms-fsmn-linear-e2e

##############################################################################
# 解析脚本参数
while [ $# -gt 0 ]
do
    if [ $# -eq 1 ]; then
        echo -e "\e[1;31;7m 参数必须成对出现, 解析失败 \e[0m"
        usage
        exit
    fi

    case $1 in
    --res)
        INPUT_RES=$2
        shift 2
    ;;
    *)
        echo -e "\e[1;31;7m 未知参数 $1 \e[0m"
        usage
        exit
    ;;
    esac
done

# 检查配置参数的合法性
if [ ! -e ${INPUT_RES} ]; then
     echo -e "\e[1;31;7m INPUT_RES 目录 [${INPUT_RES}] 不存在 \e[0m"
     exit
fi

##############################################################################
TOOLS=${LITE_FESP}/tools
res2src="res2src_linux"     # res2src发布的名字

# 放置发布文件的根目录
RELEASE=${LITE_FESP}/Release/4ssp/`date +"%Y%m%d"`
# RELEASE=${LITE_FESP}/Release/4ssp

if [ -d ${RELEASE} ]; then
    rm ${RELEASE} -rf
fi
mkdir ${RELEASE}
mkdir ${RELEASE}/ssp
mkdir ${RELEASE}/ssp/res
mkdir ${RELEASE}/lite_wakeup

##############################################################################
# 生成可执行文件 res2src
# 若pyinstaller执行失败，请先确认是否已经安装了pyinstaller，并且使用的是python3.x版本
pyinstaller --onefile ${TOOLS}/res2src/res2src.py --name ${res2src} --distpath ${TOOLS}/dist --workpath ${TOOLS}/build
if [ ! -e  ${TOOLS}/dist/${res2src} ]; then
     echo -e "\e[1;31;7m [ ${TOOLS}/dist/${res2src}] 生成失败 \e[0m"
     exit
fi

cp ${TOOLS}/dist/${res2src} ${RELEASE}/ssp

# 删除 pyinstaller 生成的文件
rm -r ${TOOLS}/build
rm -r ${TOOLS}/dist
rm ${res2src}.spec

##############################################################################
# 生成 resource.c
${RELEASE}/ssp/${res2src} --directory ${INPUT_RES} &> ${RELEASE}/lite_wakeup/resource.c

#############################################################################
# 编译, 生成库文件
build_lite_wakeup ${LITE_FESP} ${RELEASE}/lite_wakeup/resource.c
if [ ! -e ${LITE_FESP}/Release/lite-wakeup/liblite-wakeup_export.a ]; then
     echo -e "\e[1;31;7m 库[${LITE_FESP}/Release/lite-wakeup/liblite-wakeup_export.a] 不存在 \e[0m"
     exit
fi

############################################################################
# 给研究发布的文件
cp ${LITE_FESP}/Release/lite-wakeup/liblite-wakeup_export.a ${RELEASE}/ssp
cp ${INPUT_RES}/* ${RELEASE}/ssp/res/ -r
cp ${LITE_FESP}/examples/lite-wakeup/lwakeup.c  ${RELEASE}/ssp
cp ${LITE_FESP}/examples/lite-wakeup/lwakeup.h  ${RELEASE}/ssp
cp ${TOOLS}/release_libs_4ssp/ssp_readme.txt    ${RELEASE}/ssp/README.txt
cp ${TOOLS}/release_libs_4ssp/ssp_makefile      ${RELEASE}/ssp/Makefile

############################################################################
# 供 lite-wakeup 追溯的文件
cp ${LITE_FESP}/config.mk ${RELEASE}/lite_wakeup
rm ${RELEASE}/lite_wakeup/resource.c

log=${RELEASE}/lite_wakeup/git_info.txt
echo -e "仓库地址:\n"`git remote -v`"\n" > ${log}
echo -e "基准CommitId:\n"`git rev-parse HEAD` >> ${log}
echo -e "\n\n\n"    >> ${log}
git status          >> ${log}
echo -e "\n\n\n"    >> ${log}
git diff --staged   >> ${log}
echo -e "\n\n\n"    >> ${log}
git diff            >> ${log}

############################################################################
# 生成压缩包
cd ${LITE_FESP}/Release/4ssp
zip -r ssp_release_`basename ${RELEASE}`.zip `basename ${RELEASE}`
