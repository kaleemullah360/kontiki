current_user = $USER
cooja:
	sudo gnome-terminal --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start" --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/kontiki/examples/ipv6/rpl-border-router/' --tab --working-directory='/home/${USER}/kontiki/tools/cooja/' -e "ant run"
		sudo service mosquitto start
	echo "==================================================================="
	echo "Starting mosquitto service"
	service mosquitto status| grep running
	echo "==================================================================="
	firefox http://localhost:3000/mqtt >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/

mqtt:
	sudo gnome-terminal --tab --working-directory='/home/${USER}/kontiki/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	cd ~/kontiki/examples/zolertia/z1/mqtt-z1/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-mqtt-emch.rtt.upload nodeid=4 nodemac=4
		sudo service mosquitto start
	echo "==================================================================="
	echo "Starting mosquitto service"
	service mosquitto status| grep running
	echo "==================================================================="
	firefox http://localhost:3000/mqtt >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/
coap:
	sudo gnome-terminal --tab --working-directory='/home/${USER}/kontiki/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	cd ~/kontiki/examples/er-rest-example/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-coap-emch.rtt.upload nodeid=2 nodemac=2
	firefox http://localhost:3000/coap >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/

http:
	sudo gnome-terminal --tab --working-directory='/home/${USER}/kontiki/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	cd ~/kontiki/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-emch.rtt.upload nodeid=3 nodemac=3
	firefox http://localhost:3000/http >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/

brouter:
	cd ~/kontiki/examples/ipv6/rpl-border-router/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-border-router.upload nodeid=1 nodemac=1

mchop:
	cd ~/kontiki/examples/ipv6/rpl-collect/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-hop-coap-mqtt.upload nodeid=9 nodemac=9

hthop:
	cd ~/kontiki/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-hop.upload

ptrace:
	cd ~/kontiki/examples/rime/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-hop-mote.upload

clean:
	rm -f *.exe

clean-all: 
	rm -f *.o
	rm -f *.exe

log:
	vim '/home/${USER}/logs/firelog.log'

clean-emch:
	cd ~/kontiki/examples/zolertia/z1/mqtt-z1/ && sudo make clean
	cd ~/kontiki/examples/er-rest-example/ && sudo make clean
	cd ~/kontiki/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean
	cd ~/kontiki/examples/ipv6/rpl-border-router/ && sudo make clean
	cd ~/kontiki/examples/ipv6/rpl-collect/ && sudo make clean
	cd ~/kontiki/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean
	cd ~/kontiki/examples/rime/ && sudo make clean

push:
	git add -A
	git commit -m $(c)
	git push origin master

pull:
	git pull origin master
run:
	sudo gnome-terminal --tab --working-directory='/home/${USER}/kontiki/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	firefox http://localhost:3000/coap >> '/home/${USER}/logs/firelog.log' &

reboot:
	sudo reboot

view-project:
	firefox https://github.com/kaleemullah360/kontiki &

view-profile:
	firefox https://github.com/kaleemullah360 &

ifeq ($(c),)
 c = 'updates'
endif