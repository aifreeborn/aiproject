#! /bin/bash 

DIR_NAME_PREFIX=day
DIR_NAME=""

# 将旧文件夹下的文件拷贝到新的文件夹中
CURRENT_DIR=`pwd`
DIR_PREFIX="day"
FROM_DIR=""
TO_DIR=""
OLD_CNT=1
NEW_CNT=1

if [ $1 == "mk" ]
then
	# 创建新的文件夹用于存储单词照片
	for i in {1..999}
	do
		DIR_NAME=`printf "%03d" $i`
		mkdir $2/${DIR_NAME_PREFIX}${DIR_NAME}
	done
else
	for i in {1..999}
	do
		DIR_SUFFIX=`printf "%03d" $OLD_CNT`
		FROM_DIR=${CURRENT_DIR}/old/${DIR_PREFIX}${DIR_SUFFIX}

		DIR_SUFFIX=`printf "%03d" $NEW_CNT`
		TO_DIR=${CURRENT_DIR}/new/${DIR_PREFIX}${DIR_SUFFIX}

		if [ -d $TO_DIR ]
		then
			if [ -d ${FROM_DIR}o ]
			then
				mv ${FROM_DIR}o/* $TO_DIR
				let NEW_CNT++
			elif [ -d ${FROM_DIR} ]
			then
				mv ${FROM_DIR}/* $TO_DIR
				let NEW_CNT++
			fi
		fi

		let OLD_CNT++
	done	
fi
