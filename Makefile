cooja:
	gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/"
	sudo wireshark
	cd ~/kontiki/tools/cooja/ && ant run

mqtt:
	sudo gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/" --tab -e "sudo wireshark" --tab --working-directory="/home/superuser/nodev/e-MCH-APp/" -e "sudo npm start"
	cd ~/kontiki/examples/zolertia/z1/mqtt-z1/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-mqtt-emch.rtt.upload nodeid=4 nodemac=4
	firefox http://localhost:3000/mqtt &
coap:
	gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/"
	sudo wireshark
	cd ~/kontiki/tools/cooja/ && ant run

http:
	gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/"
	sudo wireshark
	cd ~/kontiki/tools/cooja/ && ant run

clean:
	rm -f *.exe

clean-all: 
	rm -f *.o
	rm -f *.exe

view-project:
	firefox https://github.com/kaleemullah360/kontiki &

view-profile:
	firefox https://github.com/kaleemullah360 &

ifeq ($(c),)
 c = 'updates'
endif

push:
	git add -A
	git commit -m $(c)
	git push origin master

pull:
	git pull origin master