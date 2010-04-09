/* Copyright (C) 2008 WowWee Group Ltd. */
/* Author: Josh Savage */

var playing_path = 0;
var going_home = 0;
var selected_path_id = -1;
var played_path_id = -1;

var DEFAULT_SPEED = 1;
var DEFAULT_TURNING_SPEED = 5;

var MOVEMENT_INTERVAL = 100;

var FWD_BCK_PULSES = 12;
var LFT_RGT_PULSES = 5;
var ROTATE_PULSES = 1;

var head_pos = 'cam_down';

var pulse_count = 0;

var pulse_timeout_id = -1;

var lastSelected = -1;

function $(id){
	return document.getElementById(id);
}
function doNothing(sText)
{
}
function sendPhoto(){
    sendCmd("../rev.cgi", 'Cmd=nav&action=26', doNothing);
    //sendCmd("../SendMail.cgi", '', doNothing);
    alert('Photo Sent');
}
function pulseMove(drive_cmd,speed){
    sendCmd('../rev.cgi', 'Cmd=nav&action=18&drive=' + drive_cmd + '&speed=' + speed, doNothing);
    
    if(drive_cmd == 1 || drive_cmd == 2){
        max_pulse_count = FWD_BCK_PULSES;
    } else if(drive_cmd == 5 || drive_cmd == 6) {
        //max_pulse_count = ROTATE_PULSES;
        return;
    } else {
        max_pulse_count = LFT_RGT_PULSES;
    }
        
    if(pulse_count < max_pulse_count){
        pulse_timeout_id = setTimeout('pulseMove(' + drive_cmd + ', ' + speed + ')', MOVEMENT_INTERVAL);
        pulse_count++;
    } else {
        pulse_count = 0;
    }
}
function move(move_id){
    var drive_cmd = 0;
    var speed = DEFAULT_SPEED;
            
    switch(move_id){
        case 'rot_left':
            drive_cmd = 5;
            speed = DEFAULT_TURNING_SPEED;
            break;
        case 'rot_right':
            drive_cmd = 6;
            speed = DEFAULT_TURNING_SPEED;
            break;
        case 'move_forward':
            drive_cmd = 1;
            break;
        case 'move_left':
            drive_cmd = 3;
            break;
        case 'move_right':
            drive_cmd = 4;
            break;
        case 'move_back':
            drive_cmd = 2;
            break;   
        case 'move_fwd_left':
            drive_cmd = 7;
            break;
        case 'move_fwd_right':
            drive_cmd = 8;
            break;
        case 'move_bck_left':
            drive_cmd = 9;
            break;
        case 'move_bck_right':
            drive_cmd = 10;
            break;
        case 'cam':
            if(head_pos == 'cam_down') { head_pos = 'cam_mid'; drive_cmd = "13"; 
            } else if(head_pos == 'cam_mid') { head_pos = 'cam_up'; drive_cmd = "11"; 
            } else if(head_pos == 'cam_up') { head_pos = 'cam_down'; drive_cmd = "12"; }
            break;
    }
    if(drive_cmd){
        //if(drive_cmd != 5 && drive_cmd != 6){
            if(pulse_timeout_id != -1){
                clearTimeout(pulse_timeout_id);
                pulse_count = 0;
            }
            pulseMove(drive_cmd, speed);
        /*} else {
            sendCmd('../rev.cgi', 'Cmd=nav&action=18&drive=' + drive_cmd + '&speed=' + DEFAULT_TURNING_SPEED, doNothing);
        }*/
    }
    if(navigator.platform == 'WinCE'){
        refreshImage();
    }
}
function refreshImage(){
    $('oCamCtl').src = '../Jpeg/CamImg' + Math.floor (10000 * Math.random ()) + '.jpg';
}
function stop(){
    sendCmd("../rev.cgi", "Cmd=nav&action=17", doNothing);
    sendCmd("../rev.cgi", "Cmd=nav&action=33", doNothing);
    
    playing_path = 0;    
    
    if(played_path_id != -1){
        $('path_' + played_path_id).className = "path";
    }
}
function processPaths(list){
    if(trim(list).length){
        var paths = list.split('|');
        for(i = 0; i < paths.length; i++){
            addPathToSelect(paths[i]);
        }
    }
}
var refreshPathListResponse = function(t) {
    var list = t;
    
    // solves version bug
    /*if(list.indexOf("version = ") != -1){  
        setTimeout("refreshPathList()",7000);
        return;
    }*/
    if(list.indexOf("responses = 0") != -1){
        list = list.substring(list.indexOf("responses = 0")+13);
        if(trim(list).length){
            processPaths(list);
        }
    } else if(list.indexOf("0|") != -1){
        list = list.substring(2);
        if(trim(list).length){
            processPaths(list);
        }
    }
}
function refreshPathList(){
    request("../rev.cgi","Cmd=nav&action=6", refreshPathListResponse);    
}
function playPath(pathName){
    request("../rev.cgi","Cmd=nav&action=7&name=" + pathName, doNothing);
}
function getPathName(e_id){
    return e_id.substring(5);
}
function addPathToList(name){
    $('paths').innerHTML = '<div id="path_' + name + '" class="path" onmousedown="return false;" onclick="playPath(getPathName(this.id));" onselectstart="return false;">' + name + '</div>' + $('paths').innerHTML;
}
function trim(string) {
	return string.replace(/^\s+|\s+$/g,"");
}
function updateVideo(value){
    sendCmd("../ChangeResolution.cgi","ResType=" + value, doNothing);
}
function goDock(){
    sendCmd("../rev.cgi","Cmd=nav&action=13", doNothing);
}
function addPathToSelect(pathname){
    pathname = trim(pathname);
    if(pathname == '') return;
    var numopts = $('selectpath').options.length;
    $('selectpath').options[numopts] = document.createElement('option');
    $('selectpath').options[numopts].value = pathname;
    $('selectpath').options[numopts].innerHTML = pathname;
}

function selectBtn(ele){
    var imgEle = ele.firstChild;
    if(lastSelected != -1){
        lastSelected.style.border = "1px solid #404040";
    }
    imgEle.style.border = "1px solid white";
    lastSelected = imgEle;
}
function request(url, postData, rtn_function){
    var xhr = null;

    try {xhr = new XMLHttpRequest();}
    catch (e) {xhr = null;}
    if (xhr == null)
    {
        try {xhr = new ActiveXObject("Msxml2.XMLHTTP");}
        catch (e) {xhr = null;}
    }
    if (xhr == null)
    {
        try {xhr = new ActiveXObject("Microsoft.XMLHTTP");}
        catch (e) {xhr = null;}
    }
    if(xhr == null){
        return;
    }	            
	
    xhr.open ("POST", url, true);
    xhr.onreadystatechange = function ()
    {
        if (xhr.readyState == 4 && xhr.status == 200)
        {
            rtn_function (xhr.responseText);
        }
    }
    xhr.send (postData);

    return xhr;
}

var battery_values = new Array();
battery_values[0] = 126;battery_values[1] = 126;battery_values[2] = 126;battery_values[3] = 126;battery_values[4] = 126;
var batt_warning = 0;

function setBatteryStrength(value){
    //if(is_moving || going_home || playing_path || recording || saving_home) return;
    
    if(value <= 1) return;

    var avg_value = 0;
    var ii = 0;
    for(ii = 0; ii < 4; ii++){
        battery_values[ii] = battery_values[ii+1];
        avg_value += battery_values[ii+1];
    }
    battery_values[4] = value;
    avg_value += value;
        
    img_num = 5;
        
    avg_value = parseInt(avg_value/5,10);
        
    if(avg_value > 122){
        img_num = 0;
    } else if(avg_value > 117) {
        img_num = 1;
    } else if(avg_value > 115) {
        img_num = 2;
    } else if(avg_value > 112) {
        img_num = 3;
    } else if(avg_value > 108) {
        img_num = 4;
    }
    
    if(img_num != 5){
        $('battery_level').style.backgroundImage = "url(../img/battery/battery" + img_num + ".gif)";   
    } else {
        $('battery_level').style.backgroundImage = "url(../img/battery/battery5.gif)";   
    }
    
    if(!batt_warning && img_num == 5){
        batt_warning = 1;
        if(confirm('Warning: Your battery is extremely low. Do you want Rovio to return to the charging dock now?')){
            checkDock();
        }
    }
    if(batt_warning && img_num != 5){
        setTimeout("batt_warning = 0;", 60000);
    }
}

function updateStatus(){
    var params = "Cmd=nav&action=1";
    request("../rev.cgi", params, processStatus);
}

var processStatus = function(t) {
    results = parseVars2(t,"|","=");
    
    for(i = 0; i < results.length; i++){
        switch(results[i][0]){
            case 'battery':
                setBatteryStrength(parseInt(results[i][1]));
                break;
            case 'charging':
                if(parseInt(results[i][1]) == 0){ // normal
                    $('battery_level').style.display = 'block';
                    $('battery_charging').style.display = 'none';
                    $('battery_charged').style.display = 'none';
                    //if($('status').innerHTML == 'Docked') $('status').innerHTML = 'Roaming';
                } 
                if(parseInt(results[i][1]) == 64){ // complete
                    $('battery_level').style.display = 'none';
                    $('battery_charging').style.display = 'none';
                    $('battery_charged').style.display = 'block';
                    //$('status').innerHTML = 'Docked';
                }
                if(parseInt(results[i][1]) == 80){ // charging
                    $('battery_level').style.display = 'none';
                    $('battery_charging').style.display = 'block';
                    $('battery_charged').style.display = 'none';
                    //$('status').innerHTML = 'Docked';
                }
                break;
        }
    }
}

function parseVars2(str, sep, equ){
  var seperated = str.split(sep);
  var results = new Array();
  for(i = 0; i < seperated.length; i++){
    results[i] = seperated[i].split(equ);
  }
  return results;
}