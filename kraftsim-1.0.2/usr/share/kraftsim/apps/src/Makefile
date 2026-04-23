all:
		for I in `ls -d */`; do cd $$I; make; cd ..; done

clean:
		for I in `ls -d */`; do cd $$I; make clean; cd ..; done

list:
		pwd
		for I in `ls -d */`; do echo $$I; done

