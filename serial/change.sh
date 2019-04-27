#!/bin/bash
sed -i 's/bi_server/server/g' ./source/*.h ./source/*.c ./source/*/*.h ./source/*/*.c
sed -i 's/bi_client/client/g' ./source/*.h ./source/*.c ./source/*/*.h ./source/*/*.c

mv ./source/bi_server ./source/server 
mv ./source/bi_client ./source/client 

mv ./source/server/bi_server.c ./source/server/server.c 
mv ./source/server/bi_server.h ./source/server/server.h 
mv ./source/client/bi_client.c ./source/client/client.c 
mv ./source/client/bi_client.h ./source/client/client.h 

mv ./source/* ./

rm -r ./source

sed -i 's/source\///g' ./*/*.c ./*/*.h

# sed -i 's/\.\.\/\//\.\.\//g' ./*/*.c ./*/*.h

for file in `find ./ -name "bi_*"`
do
    if [ -d $file ] 
    then
        echo $file is a directory
    else
        echo $file is a file
    fi
done

# sed -i 's/bi_server/server/g' ./source/* ./source/*/*