#!/bin/bash
# 
# A script to make symbolic links.
# rootfs/directory will be parent directory and initramfs/directory is a symbolic links.
# ln -s rootfs/directory initramfs/directory
# Format:
# read_dir [rootfs/directory] [initramfs/directory]
#


read_dir(){
	for file in `ls $1 -A`; do	
		if [ -d $1"/"$file ] && [ -d $2"/"$file ]; then
		echo "have the same directory : $2"/"$file"
			read_dir $1"/"$file $2"/"$file
		elif [ -d $1"/"$file ] && [ ! -d $2"/"$file ]; then 
		#soft link to directory	
			echo "directory: $1"/"$file " 
			ln -sf $1"/"$file $2"/"$file
			if [ "$?" = "0" ] ; then 
				echo -ne "\t[\033[32m OK \033[0m]\n"
			else
				echo -ne "\t[\033[31m Failed \033[0m]\n"
			fi
		else
			if [ -f $1"/"$file ] && [ -f $2"/"$file ]; then
			# in the directory and have the same file , no need to link.
				echo "in the directory have the same file"
				echo "$1"/"$file $2"/"$file"
				echo "=================================="
				if [ -L $1"/"$file ] && [ -L $2"/"$file ]; then
					echo "all is a symbolic-link file"
					echo "$1"/"$file $2"/"$file"
					rm -rf $2"/"$file
					cp $1"/"$file $2"/"$file -d
					echo "================================"
				elif [ -L $1"/"$file ] && [ ! -L $2"/"$file ]; then
					echo " $1"/"$file is a symbolic-link file"
					echo "$1"/"$file $2"/"$file"
					rm $2"/"$file
					cp $1"/"$file $2"/"$file -d
					echo "================================"
				elif [ ! -L $1"/"$file ] && [ -L $2"/"$file ]; then
					echo "$2"/"$file is a symbolic-link file" 
					echo "$1"/"$file $2"/"$file"
					rm -rf $2"/"$file
					ln -sf $1"/"$file $2"/"$file
					echo "================================"
				else
					echo "all is not a symbolic-link file"
					echo "$1"/"$file $2"/"$file"
					rm -rf $2"/"$file
                                        ln -sf $1"/"$file $2"/"$file
					echo "================================"
				fi
			elif [ -L $1"/"$file ] && [ ! -e $2"/"$file ]; then                            
                                echo "$1"/"$file is a symbolic-link file and $2"/"$file is not exist"
                                cp $1"/"$file $2"/"$file -d                                          
                                echo "============================================="                 
                        elif [ -L $1"/"$file ] && [ -d $2"/"$file ]; then                            
                                echo "$1/$file is a symbolic-link file and $2/$file is a dirctory"   
                                rm -rf $2"/"$file                                                    
                                cp $1"/"$file $2"/"$file -d                                          
                                echo "============================================="
			else 
				if [ -f $1"/"$file ] && [ ! -f $2"/"$file ]; then
					if [ -L $1"/"$file ]; then
						echo "soft link: $1"/"$file"
						cp $1"/"$file $2"/"$file -d
					else
						echo "ln file: $1"/"$file $2"/"$file"
						ln -sf $1"/"$file $2"/"$file
						if [ "$?" = "0" ] ; then 
							echo -ne "\t[\033[32m OK \033[0m]\n"
						else
							echo -ne "\t[\033[31m Failed \033[0m]\n"
						fi
					fi
					echo
				fi
			fi

		fi
	done
}

LINK_SUBDIR="bin etc lib sbin tools usr version webservice"

SRC_DIR=$1
DST_DIR=$2
echo $LINXK_SUBDIR
for i in ${LINK_SUBDIR}; do
	src_tmp=$SRC_DIR"/"$i
	if [ $DST_DIR != "/" ];then
		dst_tmp=$DST_DIR"/"$i	
	else
		dst_tmp=$DST_DIR$i
	fi
	echo -ne "$i, $src_tmp\n"
	echo -ne "$i, $dst_tmp\n"
	
	if [ -e $src_tmp ] && [ -e $dst_tmp ]; then
		echo "src=$src_tmp, dst=$dst_tmp"
		read_dir $src_tmp $dst_tmp
	else	
		ln -s $src_tmp $dst_tmp
	fi
	echo "----------------------------------------"
done
ln -sf /mnt/config/asterisk /etc/asterisk
ln -sf /etc/asterisk/passwd /etc/passwd
