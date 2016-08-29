## Prediction Algorithm for motion detection in Zolertia Z1

### Priction Function:

activate adxl sensor

while adxl semsor active do
	read x, y, z of adxl for each second continuously
	add x, y, z into 3d array of 50 samples
	
	if 3d array is filled with 50 sample do
		compute magnitude of x, y, z in 3d array
		compute standard daviation of x, y, z in 3d array
		compute average of x, y, z in 3d array

		status = SQRT(X^2 + Y^2 + Z^2)

		if status == 1
			return motion standing
		else status == 2
			return motion walking
		else status == 3
			return motion running
		else status == 4
			return motion falling
		end if

	end if

end while

deactivate sensor.

### GAO Protocol:

Client Start

	while reload page after every 3 seconds.
		get mote status using http
			mote respond with status
			display mote status
	end while
end client


### CoAP Protocol:

Client Start

	start observing mote status via CoAP
	mote checks movement status each second
	if movement status changes from last status then 
		notify client observer via CoAP
	else
		continue predicting
	end

	if observer received notification from mote
		display data
	else
		continue observing
	end
end



### HTTP Protocol:

Client start

	while request mote status each second
		mote check movement status each second
		if movement status changes from last status then 
			responde back with status
		else
			continue predicting
		end

		if mote respond with stats
			display data
		else
			continue nex request
		end
	end while

end client