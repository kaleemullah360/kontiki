//<<-----<<<<<<<[][][][]||=============================================================================||
//<<-----<<<<<<<[][][][]|| The script simulate firefox, chrome copper plugin for CoAP protocol.        || 
//<<-----<<<<<<<[][][][]|| in order to get data on scheduale interval, need to execute this script     ||
//<<-----<<<<<<<[][][][]|| in broweser console.                                                        ||
//<<-----<<<<<<<[][][][]||                                                                             ||
//<<-----<<<<<<<[][][][]|| Goal: Get Mote Data via CoAP protocol for analysis                          ||
//<<-----<<<<<<<[][][][]||                                                                             ||
//<<-----<<<<<<<[][][][]|| Written by: Kaleem Ullah <mscs14059@itu.edu.pk> <kaleemullah360@live.com>   ||
//<<-----<<<<<<<[][][][]||=============================================================================||

index = 0;	// message offset
limit = 100; // total messages to get
miliseconds = 1000; //message request delay in milliseconds
window.setInterval(function () {
  if (index <= limit) {
    var timeStampMilliSeconds = 0;	// set Time
    msg_id = $('#packet_header_tid').getAttribute('label');	// CoAP Received Message ID
    if (msg_id != '') {
      timeStampMilliSeconds = Math.floor(Date.now());	// if message is received and has an ID
    }
    pay_load = $('#info_payload').getAttribute('label');	// get the payload size
    temp = $('#packet_payload').value;		// get payload value
    $('#toolbar_get').click();	// click the GET button after getting previouse values
    // print all the data to console
    console.log('RespTime(mSec) ' + timeStampMilliSeconds + ' Message Number ' + index + ' Data ' + temp + ' Message ID ' + msg_id + ' ' + pay_load);
  } else {
    return;	// if all messages received then terminate
  }
  index++;	// increment by 1 (one)
}, miliseconds);	// message request interval