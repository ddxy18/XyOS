#!/bin/bash

file="./kernel/interrupt/intr_vec.S"

rm -f $file

touch ${file}

echo -e "# interrupt prehandler" >$file

func_name="intr_vec"
i=0
err_code="$""0"
flag=0

while ((${i} <= 255)); do
  echo -e "${func_name}${i}:" >>$file
  echo -e "\tcli" >>$file
  for j in 8 10 11 12 13 14 17 21; do
    if [ ${i} == ${j} ]; then
      flag=1
      break
    fi
  done
  if [ ${flag} == 0 ]; then
    echo -e "\tpushl ${err_code}" >>$file
  fi
  echo -e "\tpushl ""$""${i}" >>$file
  echo -e "\tjmp _alltraps" >>$file
  echo -e ".globl ${func_name}${i}" >>$file
  let i++
  flag=0
done

echo -e "" >>$file
echo -e ".data" >>$file
echo -e ".globl _${func_name}" >>$file
echo -e "_${func_name}:" >>$file

i=0
while ((${i} <= 255)); do
  echo -e "\t.long ${func_name}${i}" >>$file
  let i++
done
