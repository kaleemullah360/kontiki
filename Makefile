cooja:
	sudo gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/" --tab -e "sudo wireshark" --tab --working-directory="/home/superuser/kontiki/tools/cooja/" -e "ant run" --tab --working-directory="/home/superuser/nodev/e-MCH-APp/" -e "sudo npm start"

mqtt:
	sudo gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/" --tab -e "sudo wireshark" --tab --working-directory="/home/superuser/nodev/e-MCH-APp/" -e "sudo npm start"
	cd ~/kontiki/examples/zolertia/z1/mqtt-z1/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-mqtt-emch.rtt.upload nodeid=4 nodemac=4
	firefox http://localhost:3000/mqtt &
coap:
	sudo gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/" --tab -e "sudo wireshark" --tab --working-directory="/home/superuser/nodev/e-MCH-APp/" -e "sudo npm start"
	cd ~/kontiki/examples/er-rest-example/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-coap-emch.rtt.upload nodeid=2 nodemac=2
	firefox http://localhost:3000/coap &

http:
	sudo gnome-terminal --tab --working-directory="/home/superuser/kontiki/examples/ipv6/rpl-border-router/" --tab -e "sudo wireshark" --tab --working-directory="/home/superuser/nodev/e-MCH-APp/" -e "sudo npm start"
	cd ~/kontiki/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-emch.rtt.upload nodeid=3 nodemac=3
	firefox http://localhost:3000/http >> 

brouter:
	cd ~/kontiki/examples/ipv6/rpl-border-router/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-border-router.upload nodeid=1 nodemac=1

mchop:
	cd ~/kontiki/examples/ipv6/rpl-collect/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-hop-coap-mqtt.upload

hthop:
	cd ~/kontiki/examples/zolertia/z1/ipv6/z1-websense/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-http-emch.rtt.upload

ptrace:
	cd ~/kontiki/examples/rime/ && sudo make clean && sudo make TARGET=z1 savetarget && sudo make z1-reset && sudo make z1-hop-mote.upload

clean:
	rm -f *.exe

clean-all: 
	rm -f *.o
	rm -f *.exe

push:
	git add -A
	git commit -m $(c)
	git push origin master

pull:
	git pull origin master

reboot:
	sudo reboot

view-project:
	firefox https://github.com/kaleemullah360/kontiki &

view-profile:
	firefox https://github.com/kaleemullah360 &

ifeq ($(c),)
 c = 'updates'
endif