#!/bin/bash

function ai_print()
{
	declare -A color_list
	local color_list=([red]=31 [green]=32 [bule]=34 \
					  [yellow]=33 [magenta]=35 [cyan]=36 \
					  [white]=37 [black]=30)

	for i in ${!color_list[*]}
	do
		if [[ ${i} == ${1} ]];then
			color_value=${color_list[${i}]}
			break
		fi
	done
	if [ -z ${color_value} ];then
		echo -e "\e[1;31mError:[no ${1} color!!!]\e[0m"
		return 1
	fi

	shift
	echo -e "\e[1;${color_value}m$@\e[0m"

	return 0
}

function ai_print_in_red()
{
	ai_print red $@
}

function ai_print_in_green()
{
	ai_print green $@
}

function ai_print_in_yellow()
{
	ai_print yellow $@
}

function ai_print_in_cyan()
{
	ai_print cyan $@
}
