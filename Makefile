#! /bin/sh
current_user = $USER
CPWD = /home/${USER}/kontiki
ifeq ($(C),)
 C = 'updates'
endif
	
cooja:

	sudo gnome-terminal --tab --working-directory='${CPWD}/examples/ipv6/rpl-border-router/' --tab --working-directory='/home/${USER}/kontiki/tools/cooja/' -e "ant run"
		sudo service mosquitto start
	echo "Starting mosquitto service....."
	service mosquitto status| grep running
	firefox http://[aaaa::c30c:0:0:1]/ &

mqtt:
	sudo gnome-terminal --tab --working-directory='${CPWD}/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	cd ${CPWD}/examples/zolertia/z1/mqtt-z1/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-mqtt-emch.pow.upload nodeid=4 nodemac=4
		sudo service mosquitto start
	echo "Starting mosquitto service....."
	service mosquitto status| grep running
	firefox http://localhost:3000/mqtt >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/ &

coap:
	sudo gnome-terminal --tab --working-directory='${CPWD}/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	cd ${CPWD}/examples/er-rest-example/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-coap-emch.pow.upload nodeid=2 nodemac=2
	firefox http://localhost:3000/coap >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/ &

http:
	sudo gnome-terminal --tab --working-directory='${CPWD}/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-emch.pow.upload nodeid=3 nodemac=3
	firefox http://localhost:3000/http >> '/home/${USER}/logs/firelog.log' &
	firefox http://[aaaa::c30c:0:0:1]/ &

burn-mqtt:
	cd ${CPWD}/examples/zolertia/z1/mqtt-z1/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-mqtt-emch.pow.upload nodeid=4 nodemac=4
		sudo service mosquitto start
	echo "Starting mosquitto service....."
	service mosquitto status| grep running


burn-coap:
	cd ${CPWD}/examples/er-rest-example/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-coap-emch.pow.upload nodeid=2 nodemac=2

burn-http:
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-emch.pow.upload nodeid=3 nodemac=3

burn-brouter:
	cd ${CPWD}/examples/ipv6/rpl-border-router/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-border-router.upload nodeid=1 nodemac=1

burn-hop-a:
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-emch-1.hop.upload nodeid=10 nodemac=10

burn-hop-b:
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-emch-1.hop.upload nodeid=11 nodemac=11

burn-mhop:
	cd ${CPWD}/examples/zolertia/z1/mqtt-z1/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-mqtt-emch.hop.upload

burn-chop:
	cd ${CPWD}/examples/er-rest-example/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-coap-emch.hop.upload

burn-hhop:
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-hop.upload

ptrace:
	cd ${CPWD}/examples/rime/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-hop-mote.upload

clean:
	rm -f *.exe

clean-all: 
	rm -f *.o
	rm -f *.exe

log:
	vim '/home/${USER}/logs/firelog.log'

console-mqtt:
	mosquitto_sub -u use-token-auth -P AUTHZ -d -t iot-2/evt/status/fmt/json -v

clean-emch:
	cd ${CPWD}/examples/zolertia/z1/ && sudo make clean
	cd ${CPWD}/examples/zolertia/z1/mqtt-z1/ && sudo make clean
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean
	cd ${CPWD}/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean
	cd ${CPWD}/examples/ipv6/rpl-border-router/ && sudo make clean
	cd ${CPWD}/examples/ipv6/rpl-collect/ && sudo make clean
	cd ${CPWD}/examples/er-rest-example/ && sudo make clean
	cd ${CPWD}/examples/rime/ && sudo make clean

push:
	@#	make commit C="Added-some-test"
	git add -A
	git commit -m $(C)
	git push origin master

pull:
	git pull origin master

run:
	sudo gnome-terminal --tab --working-directory='${CPWD}/examples/ipv6/rpl-border-router/' --tab -e "sudo wireshark" --tab --working-directory='/home/${USER}/nodev/e-MCH-APp/' -e "sudo npm start"
	firefox http://localhost:3000/coap >> '/home/${USER}/logs/firelog.log' &

list:
	cd ${CPWD}/examples/ipv6/rpl-border-router/ && sudo make z1-motelist && cd ~/kontiki/

login:
	cd ${CPWD}/examples/ipv6/rpl-border-router/ && sudo make z1-reset && sudo make login && cd ~/kontiki/

reboot:
	sudo reboot

view-project:
	firefox https://github.com/kaleemullah360/kontiki &

view-profile:
	firefox https://github.com/kaleemullah360 &

open-project:
	subl ~/kontiki &

life:
	bash ./life.sh
