for i in `ls * -d`
do
  if [ -d $i ]
  then
 	echo  $i
 	cd $i
	c_file=`ls *.c`
 	exec_file=`basename $c_file .c`	
	cp $exec_file $MSO_HOME/bin
 	echo "===========done ==========="
 	cd ..
  fi
done

