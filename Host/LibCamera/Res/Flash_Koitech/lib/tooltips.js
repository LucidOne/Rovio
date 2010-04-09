/* Copyright (C) 2008 WowWee Group Ltd. */
/* Author: Josh Savage */

var enableToolTips = true;
var tooltipTimeout = -1;

function showToolTip(){
    showToolTipId(this.id);
}

function showToolTipId(id){
    if(!enableToolTips) return;
    
    var ttDiv = $('sidetooltip');
    var text = '';
    var width = '130px';
    var top = '0px';
    var left = '0px';
    
    ttDiv.style.display = 'none';
    if(tooltipTimeout != -1){
        clearTimeout(tooltipTimeout);
    }
    
    switch(id){
        case 'home':
            text = 'Go home';
            top = '-27px';
            left = '176px';
            break;
        case 'savehome':
            text = 'Save home position';
            top = '-100px';
            left = '176px';
            break;
        case 'cam_down':
        case 'cam_up':
        case 'cam_mid':
            text = 'Change head position';
            top = '-160px';
            left = '90px';
            break;
        case 'headlight':
            if(isClicked($('headlight'))){
                text = 'Turn headlight off';
            } else {
                text = 'Turn headlight on';
            }
            top = '-160px'; left = '0px';
            break;
        case 'snapshot':
            text = "Take photo";
            top = '-160px'; left = '178px';
            break;
        case 'record':
            if(recording){
                text = 'Save recorded path';
            } else {
                text = 'Start recording a path';
            }
            top = '-108px';
            left = '5px';
            break;
        case 'delete_path':
            text = 'Delete selected path';
            top = '-88px';
            left = '5px';
            break;
        case 'reset':
            text = 'Delete all paths';
            top = '-68px';
            left = '5px';
            break;
        case 'stop':
            text = 'Stop Rovio';
            top = '-27px'; 
            left = '5px';
            break;    
        case "move_forward":
            text = "Move forward";
            top = '-470px'; left = '90px';
            break;
        case "move_left":
            text = "Move left";
            top = '-373px'; left = '-1px';
            break;
        case "move_right":
            text = "Move right";
            top = '-372px'; left = '183px';
            break;
        case "move_back":
            text = "Move back";
            top = '-343px'; left = '90px';
            break;
        case "rotr_03":
            text = "Rotate 45&deg; right";
            top = '-475px'; left = '184px';
            break;
        case "rotr_07":
            text = "Rotate 90&deg; right";
            top = '-372px'; left = '183px';
            break;
        case "rotr_11":
            text = "Rotate 135&deg; right";
            top = '-292px'; left = '182px';
            break;
        case "rotr_15":
            text = "Rotate 180&deg;"
            top = '-245px'; left = '94px';
            break;
        case "rotl_03":
            text = "Rotate 45&deg; left";
            top = '-475px'; left = '-1px';
            break;
        case "rotl_07":
            text = "Rotate 90&deg; left";
            top = '-373px'; left = '-1px';
            break;
        case "rotl_11":
            text = "Rotate 135&deg; left";
            top = '-293px'; left = '-1px';
            break;
        case "rot_left":
            text = "Rotate left";
            top = '-579px'; left = '40px';
            break;
        case "rot_right":
            text = "Rotate right";
            top = '-579px'; left = '135px';
            break;
        case "joystick":
            text = "Drag Rovio to move";
            top = '-362px'; left = '92px';
            break;
        case "maintain_ratio":
            text = "Maintain video aspect ratio";
            top = '-95px';
            left = '140px';
            width = '170px';
            break;
        case "nav_signal":
            text = "100% Nav strength";
            if(nav_value == 1) text = "80% Nav strength";
            if(nav_value == 2) text = "60% Nav strength";
            if(nav_value == 3) text = "40% Nav strength";
            if(nav_value == 4) text = "20% Nav strength";
            if(nav_value > 4) text = "Nav strength extremely low";
        
            //text = "TrueTrack beacon strength";
            top = '-580px'; left = '-300px';
            width = '160px';
            break;
        case "wifi_signal":
            text = "100% Wifi strength";
            if(wifi_value == 1) text = "80% Wifi strength";
            if(wifi_value == 2) text = "60% Wifi strength";
            if(wifi_value == 3) text = "40% Wifi strength";
            if(wifi_value == 4) text = "20% Wifi strength";
            if(wifi_value > 4) text = "Wifi strength extremely low";
            
            width = '160px';
            top = '-580px'; left = '-210px';
            break; 
        case "room_id":
            text = "TrueTrack beacon ID";
            
            top = '-580px'; left = '-300px';
            width = '160px';
            break;
        case "status":
            text = "Rovio status";
            top = '-580px'; left = '-595px';
            break;
        case "battery_level":
            text = "100% battery";
            if(battery_level == 1) text = "80% battery";
            if(battery_level == 2) text = "60% battery";
            if(battery_level == 3) text = "40% battery";
            if(battery_level == 4) text = "20% battery";
            if(battery_level > 4) text = "Battery extremely low";
            top = '-580px'; left = '-70px';
            width = '140px';
            break;
        case 'battery_charged':
            text = "Battery fully charged";
            top = '-580px'; left = '-70px';
            width = '140px';
            break;
        case 'battery_charging':
            text = "Battery charging";
            top = '-580px'; left = '-70px';
            width = '140px';
            break;  
        case 'online_status':
            text = online_status_txt;
            top = '-590px'; left = '-100px';
            width = '340px';
            break;
        default:
            return;
    }
    
    ttDiv.innerHTML = text;
    ttDiv.style.width = width;
    ttDiv.style.top = top;
    ttDiv.style.left = left;
    tooltipTimeout = setTimeout("$('sidetooltip').style.display = 'block'; tooltipTimeout = -1;", 500);
}
function hideToolTip(){
    $('sidetooltip').style.display = 'none';
    if(tooltipTimeout != -1){
        clearTimeout(tooltipTimeout);
    }
}

function addTooltips(){
    var btnIds = new Array("room_id", "battery_charged", "battery_charging", "battery_level", "status", "nav_signal", "wifi_signal", "maintain_ratio", "joystick", "home","savehome", "cam_down", "cam_mid", "cam_up", "headlight", "record", "delete_path", "reset", "stop", "move_forward", "move_left", "move_right", "move_back", "rotr_03", "rotr_07", "rotr_11", "rotr_15", "rotl_03", "rotl_07", "rotl_11", "rot_left", "rot_right", "online_status");
    
    for(i = 0; i < btnIds.length; i++){
        $(btnIds[i]).onmouseover = showToolTip;
        $(btnIds[i]).onmouseout = hideToolTip;
    }
}