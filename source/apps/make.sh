for i in `ls`
 do
	if [ -d $i ]
	then
		 echo  $i
		 cd $i
		 make clean
		 make
		 echo "===========done ==========="
		 cd ..
	fi
 done

