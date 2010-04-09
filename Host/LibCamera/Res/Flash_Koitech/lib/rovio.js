/* Copyright (C) 2008 WowWee Group Ltd. */
/* Author: Josh Savage */

/* --- Version 5.03b --- */
var UI_VERSION = '5.03b';
var ppp_firmware_enabled = true;
var portal_address = 'http://www.wowweesupport.com/portal';

/*
---- Video Players: ---
0 - ActiveX
1 - Quicktime
2 - MJPEG
3 - VLC + JavaApplet (option currently unstable especially on mac)
*/
var DEFAULT_IE_PLAYER = 0;
var DEFAULT_MOZ_PLAYER = 2;
var DEFAULT_OTHER_PLAYER = 2;

var COOKIE_RTSP_USER = 'rovio_rtsp_user';
var COOKIE_RTSP_PASS = 'rovio_rtsp_pass';
var COOKIE_JAVA_WARNING = 'rovio_java';

var v_player = -1;

var force_mjpeg = false;

var MOVEMENT_INTERVAL = 200;

var STATUS_INTERVAL = 2000;

var MAX_SPEED = 10;

var MAX_REQUESTS = 10;

var MAX_NO_PATHS = 10;

// parameter indexes
var i_MR = 0;
var i_MS = 1;
var i_TS = 2;
var i_RS = 3;
var i_LR = 4;
var i_SVP = 5; // safari video player
var i_UPnP = 6; // upnp just enabled
var i_SS = 7; // show status
var i_MIIP1 = 8; // manual ip
var i_MIIP2 = 9; // manual ip
var i_VIA = 10; // verify internet access
var i_NFA = 11;
var i_AVF = 12; // auto set video frequency

var rtspAuth = -1;
var rtspAuthUser = '';
var rtspAuthPass = '';

var DEFAULT_SPEED = 5;
var DEFAULT_TURN_SPEED = 5;
var DEFAULT_ROT_SPEED = 2;

var movement_speed = DEFAULT_SPEED;
var turn_speed = DEFAULT_TURN_SPEED;
var rot_speed = DEFAULT_ROT_SPEED;
var latency = MOVEMENT_INTERVAL;

var movement_interval_id = -1;
var status_interval_id = -1;
var checkaccess_interval_id = -1;
var rename_timeout_id = -1;

var aspect_ratio = 1;

var wifi_warning = 0;
var batt_warning = 0;

var video_slider;
var brightness_slider;
var svol_slider;
var mvol_slider;
var battery_values = new Array();
battery_values[0] = 126;battery_values[1] = 126;battery_values[2] = 126;battery_values[3] = 126;battery_values[4] = 126;
var battery_level;
var nav_value, wifi_value;

var speed_movement_slider, speed_turn_slider, speed_angle_slider, lat_slider;

var ctrl_alt_down = false;

var selected_path_id = -1;
var played_path_id = -1;

var key_up = false;
var key_down = false;
var key_left = false;
var key_right = false;
var key_rot_right = false;
var key_rot_left = false;
var key_shiftdown = false;
    
var key_events = true;
var cam_initialized = false;
var prevent_resize = false;

var playing_path = 0;
var going_home = 0;
var saving_home = 0;
var recording = 0;
var is_moving = 0;

var settings_brightness_slider = -1;

var init_load = true;

var IR_val = 0;
var obstacle_val = 0;
var frame_rate = 0;
var brightness = 0;
var resolution = 0;
var compression = 0;
var nav_strength = 0;
var home_pos_saved = 0;
var nav_room = 0;
var user_check = -1;
var user_guest = 1;
if(!user_guest) alert('Warning user_guest set to 0');
var ac_freq = 0;
var auto_cam_freq = -1;
var user_id = -1;

var stream_id = -1;

var mic_volume = -1;
var s_volume = -1;

var settings_dialog_open = false;
var help_dialog_open = false;
var selection_enabled = true;

var reboot_countdown;
var reboot_interval_id;

var net_web_port = -1;

var xhr = new Array();
var xhrRtn = new Array();
var status_xhr = new Array();
var status_xhrRtn = new Array();
var cur_xhr = 0;
var cur_status_xhr = 0;

var last_cam_pos = -1;

var evo_version = "";
var wb_version = "";

var slider_only_val = false;
var after_update = false;
var after_update_timeout_id = -1;

var smtp_enable = 0;
var smtp_server = -1;
var smtp_user = '';
var smtp_pass = '';
var smtp_sender = '';
var smtp_receiver = '';
var smtp_sub = '';
var smtp_check = 0;
var smtp_body = '';
var smtp_port = '25';

var ppp_status = -1;
var ppp_user = -1;
var ppp_enabled = -1;

var verify_internet_access = -1;
var initial_upnp_load = true;
var upnp_just_enabled = false;
var upnp_enabled = -1;
var upnp_port = -1;
var upnp_http_port = -1;
var upnp_tcp_port = -1;
var upnp_udp_port = -1;
var upnp_ip = -1;
var manual_internetip = -1;
var loadUPnPFields_interval_id = -1;
var online_status_txt = '';
var online_status_help_id;
var show_online_status = 1;

var essid = -1;

var pathSaved = 0;

var access_settings_panel = 0;

var update_navstatus = 1;

var last_dns_check = -1;

var num_of_paths = 0;

var path_list_refreshed = 0;

var initial_load_time;

var selected_network = -1;

var external_ip_status = -1;
var external_domain_status = -1;
var external_address = '';

var firmware_alert = -1;

function init(){
    var d = new Date();
    initial_load_time = d.getTime();

    // display hack for IE6
    if(isIEpre7()){
        var panels = $('cam_and_tabbed_panels');
        panels.style.position = "relative";
        panels.style.top = "-380px";
    }

    // set default player    
    if(isIE()){
        v_player = DEFAULT_IE_PLAYER;
    } else {
        // note: safari & firefox video players are set in getAllParameters. Default for them is DEFAULT_MOZ_PLAYER.
        if(!isSafari() && !isFF()){
            v_player = DEFAULT_OTHER_PLAYER;
        }
    }

    // disable text selection
    disableSelection();
    addTooltips();

    $('fake_link').focus();

    // all containers must be visible to create sliders
    $('sidetab_2_container').style.display = 'block';
    $('sidetab_3_container').style.display = 'block';
    $('settings_dialog').style.display = 'block';
    
    settings_brightness_slider = new Control.Slider('video_bhandle','video_btrack',{ range:$R(1,6), onChange: changeSettingsBrightness });
    speed_movement_slider = new Control.Slider('move_movehandle','move_movetrack',{ range:$R(1,10), onChange: enableMoveUpdateBtn });
    speed_turn_slider = new Control.Slider('move_turnhandle','move_turntrack',{ range:$R(1,10), onChange: enableMoveUpdateBtn });
    speed_angle_slider = new Control.Slider('move_anglehandle','move_angletrack',{ range:$R(1,10), onChange: enableMoveUpdateBtn });
    resetMovementSettings();

    new Draggable('joystick',{revert: 1, snap: joystickSnap, onEnd: stopMoving });
    video_slider = new Control.Slider('video_handle','video_track',{ range:$R(0,11), onChange: changeVideoSpeed});
    brightness_slider = new Control.Slider('brightness_handle','brightness_track',{ range:$R(1,6), onChange: changeCamBrightness });
            
    svol_slider = new Control.Slider('svol_handle','svol_track',{ range:$R(0,31), onChange: changeSpeakerVol});
    mvol_slider = new Control.Slider('mvol_handle','mvol_track',{ range:$R(0,31), onChange: changeMicVol});

    createXMLRequestObjs();

    // upnp settings need to be called as soon as possible for RTSP feeds
    loadUPnPFields(); 

    // need to know web port for ActiveX
    loadWebPort();

    // need to know the manual external ip
    getAllParameters(); 

    getMyself();

    loadForceMJPEGFromURL();

    var force_reboot = getQueryVariable("reboot");
    if(force_reboot != null && parseInt(force_reboot)){
        $('settings_dialog').style.display = 'none';
        selectTab($('sidetab_1'));
        initReboot();
        return;
    }
    
    refreshPathList();
    updateStatus();
    
    status_interval_id = setInterval("updateStatus()",STATUS_INTERVAL);
    
    selectTab($('sidetab_1'));
    selectTab($('settingtab_1'));
    $('settings_dialog').style.display = 'none';
        
    $('version').innerHTML = '';
}
function enableMoveUpdateBtn(){
    $('move_update').disabled = false;
}
function enableSelection(){
    selection_enabled = true;
    document.onselectstart=new Function ("return true");
    
    //if the browser is NS6
    if (window.sidebar){
        document.onmousedown = hideToolTip;
    }
}
function disableSelection(){
    selection_enabled = false;
    
    //if the browser is IE4+
    document.onselectstart = new Function ("return false");
    
    //if the browser is NS6
    if (window.sidebar){
        document.onmousedown = disabletext;
        document.onclick = reEnable;
    }
}
function disabletext(e){
    hideToolTip();
    
    if(selection_enabled) return true;

    return false;
}
function reEnable(){
    return true;
}
function setServerTime(){
    var d = new Date();
    var params = "Sec1970=" + parseInt(d.getTime() / 1000) + "&TimeZone=" + d.getTimezoneOffset();
  
    manualRequest('SetTime.cgi', params, null);
}
function clearAllIntervals(){
    if(movement_interval_id != -1) clearInterval(movement_interval_id);
    if(status_interval_id != -1) clearInterval(status_interval_id);
    if(rename_timeout_id != -1) clearInterval(rename_timeout_id);
    if(loadUPnPFields_interval_id != -1) clearInterval(loadUPnPFields_interval_id); 
    if(checkaccess_interval_id != -1) clearInterval(checkaccess_interval_id);
}
function initReboot(){
    clearAllIntervals();
    
    $('camera_container').innerHTML = '';

    openRebootDialog();
    reboot_countdown = 30;
    
    reboot_interval_id = setInterval("rebootCountdown(null)", 1000);
    reboot();
}
function openRebootDialog(){
    $('camera_container').innerHTML = '';
    $('disable_controls').style.display = 'block';
    $('settings_dialog').style.zIndex = "2000";
    $('reboot_dialog').style.display = 'block';
}
function rebootCountdown(location){
    reboot_countdown--;
    $('reboot_countdown').innerHTML = reboot_countdown;
            
    if(reboot_countdown <= 0){
        if(location == null) location = window.location.pathname;
        clearInterval(reboot_interval_id);
        window.location.href = location;
    }
}
function resizeCamera(){
    if(prevent_resize) return;
    if(settings_dialog_open) return;
    prevent_resize = true;

    var theWidth, theHeight;
    // Window dimensions:
    if (window.innerWidth) {
        theWidth=window.innerWidth;
    }
    else if (document.documentElement && document.documentElement.clientWidth) {
        theWidth=document.documentElement.clientWidth;
    }
    else if (document.body) {
        theWidth=document.body.clientWidth;
    }
    if (window.innerHeight) {
        theHeight=window.innerHeight;
    }
    else if (document.documentElement && document.documentElement.clientHeight) {
        theHeight=document.documentElement.clientHeight;
    }
    else if (document.body) {
        theHeight=document.body.clientHeight;
    }

    var cam_box = document.getElementById('camera_box');
    var cam_td = document.getElementById('cam_m');
    var cam_con = document.getElementById('camera_container');
    
    var new_cam_width = theWidth - 367 - 14;
    var new_cam_height = theHeight - 140 - 4;
    new_cam_width = new_cam_width < 640 ? 640 : new_cam_width;
    new_cam_height = new_cam_height < 480 ? 480 : new_cam_height;
        
    if(aspect_ratio){
        new_cam_width = new_cam_width > new_cam_height * (4 / 3) ? new_cam_height * (4 / 3) : new_cam_width;
        new_cam_height = new_cam_height > new_cam_width * (3 / 4) ? new_cam_width * (3 / 4) : new_cam_height;
    }
    
    var new_table_width = new_cam_width + 13;
    var new_table_height = new_cam_height - 6;
    
    
    cam_box.style.width = new_table_width + "px";
    cam_box.style.height = new_table_height + "px";
    
    cam_td.style.width = new_table_width - 20 + "px";
    cam_td.style.height = new_table_height - 20 + "px";
        
    cam_con.style.width = new_cam_width + "px";
    cam_con.style.height = new_cam_height + "px";
    
    if(cam_initialized){
        updateCam(new_cam_width, new_cam_height);
    } else {
        if(v_player != -1){
            $('camera_container').innerHTML = getPlayerCode(new_cam_width, new_cam_height);
            if($('camera_container').innerHTML != ''){
                cam_initialized = true;
                
                // check if ActiveX loaded   
                if(v_player == 0 && $('oCamCtl').AutoStart == null){
                    $('activex_failure').innerHTML = '<label style="color: white;">Can not load live video ActiveX control. When prompted by Internet Explorer make sure you chose to install the ActiveX plugin. If after installing the plugin no video is displayed you may need to refresh this page by pressing F5. <br/><a href="http://www.wowwee.com/support/rovio" style="color: white;">Refer to WowWee\'s support page for more information</a>.';
                }
            } else {
                cam_initialized = false;
            
                // try again in 2 seconds
                // most likely cause browser hasn't got a response back on the rtsp port to use
                setTimeout('resizeCamera();', 2000);
            }
        }
    }
    
    prevent_resize = false;
}
function updateCam(width, height){
    $('oCamCtl').width = width;
    $('oCamCtl').height = height;
}
function maintainAspectRatio(maintain){
    if(maintain == -1) maintain = 1;

    if(maintain){
        maintain = 1;
        changeToClickedImg($('maintain_ratio'));
    } else {
        maintain = 0;
        changeToUnclickedImg($('maintain_ratio'));
    }
    aspect_ratio = maintain;
    setParameter(i_MR, aspect_ratio);
    resizeCamera();
}
function getQueryVariable(variable) {
  var query = window.location.search.substring(1);
  var vars = query.split("&");
  for (var i=0;i<vars.length;i++) {
    var pair = vars[i].split("=");
    if (pair[0] == variable) {
      return pair[1];
    }
  }
  return null;
}
function saveAuthenticationStatusResponse(t){
    $('user_update_auth').disabled = true;
    alert("Authentication Settings Updated.");
}
function saveAuthenticationStatus(){
    var url = "SetUserCheck.cgi";
    var params = "Check=" + ($('user_check').checked ? 1 : 0);
    
    clearAllIntervals();
    manualRequest(url, params, saveAuthenticationStatusResponse);
    window.location.href = window.location.pathname;
}

function selectTab(tab){
    tabs = tab.parentNode.parentNode.getElementsByTagName('div');
    for(i = 0; i < tabs.length; i++){
        changeToUnclickedImg(tabs[i]);
        $(tabs[i].id + '_container').style.display = 'none';
    }
    changeToClickedImg(tab);
    $(tab.id + '_container').style.display = 'block';
}
function updateNavState(state) {
    if(!update_navstatus) return;

    if(state == 5){
        saving_home = 1;
        $('status').innerHTML = 'Saving Home';
    }
    if(state == 3){
        playing_path = 1;
        if($('status').innerHTML.indexOf('Going to') != 0){
            $('status').innerHTML = 'Following path';
        }
    }
    if(state == 2){
        $('status').innerHTML = 'Going Home';
        going_home = 1;
    }
    if(state == 0){
        if(nav_strength == 4){
            $('status').innerHTML = '<blink style="color: #ffc000;">Low Nav Signal</blink>';
        } else if(nav_strength == 5){
            $('status').innerHTML = '<blink style="color: #f11100;">No Nav Signal</blink>';
        } else {
            if($('status').innerHTML.indexOf('Going to') == 0 || $('status').innerHTML.indexOf('Following path') == 0){
                $('status').innerHTML = 'Path completed';
                setTimeout("$('status').innerHTML = 'Roaming';", 3000);
            }
            if($('status').innerHTML.indexOf('Nav Signal') != -1 || $('status').innerHTML.indexOf('Going Home') == 0 || $('status').innerHTML.indexOf('Saving Home') == 0){
                $('status').innerHTML = 'Roaming';
            }
        }
        
        going_home = 0;
        saving_home = 0;
    
        if(playing_path){
            playing_path = 0;
            var temp_id = played_path_id;
            played_path_id = -1;
            selectPath(temp_id);
        }
    }
}
var processStatus = function(t) {
    t = t.responseText;
    var results = parseVars2(t,"|","=");
    
    var update_video_slider = false;
    var old_IR_val = IR_val;
    for(i = 0; i < results.length; i++){
        switch(results[i][0]){
            case 'ss':
                setNavStrength(parseInt(results[i][1],10));
                break;
            case 'flags':
                var nav_flags = parseInt(results[i][1]);
                if(parseInt(nav_flags / 4) != 0){
                    if(IR_val && !is_moving){
                        $('ir_on').style.display = 'block';
                    }
                    IR_val = 1;
                    nav_flags -= 4;
                } else {
                    if(!IR_val && !is_moving){
                        $('ir_on').style.display = 'none';
                    }
                    IR_val = 0;
                }
                if(parseInt(nav_flags / 2) != 0 && IR_val){
                    $('obstacle_detected').style.display = 'block';
                    obstacle_val = 1;
                    nav_flags -= 2;
                } else {
                    $('obstacle_detected').style.display = 'none';
                    obstacle_val = 0;
                }
                home_pos_saved = nav_flags;                               
                
                if(!after_update && IR_val != old_IR_val){
                    $('move_ir').checked = IR_val;
                }
                break;
            case 'brightness':
                if(!after_update && brightness != parseInt(results[i][1])){
                    brightness = parseInt(results[i][1]);
                    brightness_slider.setValue(parseInt(results[i][1]));
                }
                break;
            case 'resolution':
                if(!after_update && resolution != parseInt(results[i][1])){
                    $('video_res').value = parseInt(results[i][1]);
                    update_video_slider = true;
                    resolution = parseInt(results[i][1]);
                    cam_initialized = false;
                    resizeCamera();
                }
                break;
            case 'video_compression':
                if(!after_update && compression != parseInt(results[i][1])){
                    $('video_quality').value = parseInt(results[i][1]);   
                    update_video_slider = true;
                    compression = parseInt(results[i][1]);
                }
                break;
            case 'frame_rate':
                if(!after_update && frame_rate != parseInt(results[i][1])){
                    $('video_fps').value = parseInt(results[i][1]);  
                    update_video_slider = true; 
                    frame_rate = parseInt(results[i][1]);
                }
                break;
            case 'privilege':
                user_guest = parseInt(results[i][1]);
                break;
            case 'user_check':
                user_check = parseInt(results[i][1]);
                if(rtspAuth == -1){
                    if(!user_check){
                        rtspAuth = 1;
                        rtspAuthUser = '';
                        rtspAuthPass = '';
                    } else {
                        if(readCookie(COOKIE_RTSP_USER) != null && readCookie(COOKIE_RTSP_PASS) != null){
                            rtspAuth = 1;
                            rtspAuthUser = readCookie(COOKIE_RTSP_USER);
                            rtspAuthPass = readCookie(COOKIE_RTSP_PASS);
                            $('rtsp_user').value = rtspAuthUser;
                            $('rtsp_pass').value = rtspAuthPass;
                        }
                    }
                }
                break;
            case 'speaker_volume':
                if(s_volume != parseInt(results[i][1])){
                    svol_slider.setValue(parseInt(results[i][1]));
                    s_volume = parseInt(results[i][1]);
                }
                break;
            case 'mic_volume':
                if(mic_volume != parseInt(results[i][1])){
                    mvol_slider.setValue(parseInt(results[i][1]));
                    mic_volume = parseInt(results[i][1]);
                }
                break;
            case 'wifi_ss':
                setWifiStrength(parseInt(results[i][1]));
                break;
            case 'battery':
                if(!is_moving){
                    setBatteryStrength(parseInt(results[i][1]));
                }
                break;
            case 'state':
                updateNavState(parseInt(results[i][1]));
                break;
            case 'charging':
                if(parseInt(results[i][1]) == 0){ // normal
                    $('battery_level').style.display = 'block';
                    $('battery_charging').style.display = 'none';
                    $('battery_charged').style.display = 'none';
                    if($('status').innerHTML == 'Docked') $('status').innerHTML = 'Roaming';
                } 
                if(parseInt(results[i][1]) == 64){ // docked but not charging
                    $('battery_level').style.display = 'block';
                    $('battery_charging').style.display = 'none';
                    $('battery_charged').style.display = 'none';
                    $('status').innerHTML = 'Docked';
                }
                if(parseInt(results[i][1]) == 72){ // completed
                    $('battery_level').style.display = 'none';
                    $('battery_charging').style.display = 'none';
                    $('battery_charged').style.display = 'block';
                    $('status').innerHTML = 'Docked';
                }
                if(parseInt(results[i][1]) == 80){ // charging
                    $('battery_level').style.display = 'none';
                    $('battery_charging').style.display = 'block';
                    $('battery_charged').style.display = 'none';
                    $('status').innerHTML = 'Docked';
                }
                break;
            case 'head_position':
                var hp = parseInt(results[i][1]);
                var pos = '';
                if(hp > 195 && hp < 205) pos='down';
                if(hp > 130 && hp < 140) pos='mid';
                if(hp > 60 && hp < 70) pos='up';
                if(pos != ''){
                    if(!isClicked($('cam_' + pos))){
                        if(last_cam_pos == pos || last_cam_pos == -1){
                            changeToUnclickedImg($('cam_up'));
                            changeToUnclickedImg($('cam_down'));
                            changeToUnclickedImg($('cam_mid'));
                            changeToClickedImg($('cam_' + pos));
                        }                     
                    }
                    last_cam_pos = pos;
                }
                break;
            case 'room':
                nav_room = parseInt(results[i][1]);
                if(nav_strength == 5){
                    $('room_id').innerHTML = "";
                } else {
                    $('room_id').innerHTML = results[i][1];
                }
                break;
            case 'ac_freq':
                old_ac_freq = ac_freq;
                if(parseInt(results[i][1],10) == 1){
                    ac_freq = 50;
                }
                if(parseInt(results[i][1],10) == 2){
                    ac_freq = 60;
                }
                if(auto_cam_freq && old_ac_freq != ac_freq && ac_freq != 0){
                    manualRequest('SetCamera.cgi', 'Frequency=' + ac_freq, null);
                }
                break;
        }
    }
    
    if(!after_update && update_video_slider){
        slider_only_val = true;
        video_slider.setValue(Math.round(resolution * 3) + compression);
        slider_only_val = false;
        $('video_save').disabled = true;
    }
    
    if(init_load){    
        init_load = false;

        updateUserCheck();
        
        access_settings_panel = 1;
        
        $('move_ir').checked = IR_val;
        
        if(!user_guest){
            // need to know if we are using a domain for checking external access
            loadDynDNSSettings();
        
            //loadSettingsPanels();
            setServerTime();
            getEvoVersion();
            getWBVersion();
            loadSMTPSettings();
            
            setTimeout('getLatestVersion()',5000);
        }
             
        //if(!user_guest){
            //setTimeout("access_settings_panel = 1",1000);
        //}
    }
}
function updateStatus(){
    var params = "Cmd=nav&action=1";
    statusRequest("rev.cgi", params, processStatus);
}
function setNavStrength(value){
    if(value > 13000){ 
        img_num = 0;
    } else if(value > 10000){
        img_num = 1;
    } else if(value > 7000){
        img_num = 2;
    } else if(value > 4000){
        img_num = 3;
    } else if(value > 2000){
        img_num = 4;
    } else {
        img_num = 5;
        $('room_id').innerHTML = "";
    }
    
    nav_strength = img_num;
    nav_value = img_num;
    $('nav_signal').className = 'signal' + img_num;
    
    if(recording && img_num == 5){
        $('status').innerHTML = '<blink style="color: #f11100;">Recording Suspended</blink>';
    } else if($('status').innerHTML.indexOf('Recording Suspended') != -1){
        $('status').innerHTML = 'Recording';
    }
}
var last_wifi = 255;
function setWifiStrength(value){
    if(going_home || playing_path) return;

    img_num = 5;
    var avg_value = (value + last_wifi) / 2;
    last_wifi = avg_value;
    if(avg_value > 201){ // 210
        img_num = 0;
    } else if(avg_value > 191){ // 200
        img_num = 1;
    } else if(avg_value > 181){ // 195
        img_num = 2;
    } else if(avg_value > 171){ // 190
        img_num = 3;
    } else if(avg_value > 166){ // 185
        img_num = 4;
    }
    
    // img_num = 0; // HACK: Remove when wifi strength is working

    wifi_value = img_num;
    $('wifi_signal').className = 'signal' + img_num;

    if(img_num < 4 && wifi_warning == 1){
        wifi_warning = 2;
        setTimeout('wifi_warning = 0', 60000);
    }
    
    if(wifi_warning == 0 && (img_num == 5 || img_num == 4)){
        wifi_warning = 1;
        //alert('Warning: Your wireless signal is extremely low. It is highly recommended that you drive towards your wireless access point before you lose access to your Rovio.');
    }
}

function setBatteryStrength(value){
    if(is_moving || going_home || playing_path || recording || saving_home) return;
    
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
    
    battery_level = img_num;
    $('battery_level').className = "battery" + img_num;
        
    if(batt_warning == 0 && img_num == 5 && $('battery_charging').style.display != 'block'){
        batt_warning = 1;
        if(confirm('Warning: Your battery is extremely low. Do you want Rovio to return to the charging dock now?')){
            checkDock();
        }
    }
    if(batt_warning == 1 && img_num != 5){
        batt_warning = -1;
        setTimeout("batt_warning = 0;", 300000);
    }
}
function loadForceMJPEGFromURL(){
    force_mjpeg = getQueryVariable("force_mjpeg");
    if(force_mjpeg == null) force_mjpeg = 0;
    force_mjpeg = parseInt(force_mjpeg);
    
    if(!isIE()){
        $('force_mjpeg_row').style.display = 'none';
    }
    
    return force_mjpeg;
}
function getPlayerCode(width, height){ 
    var path = location.hostname;
    var httppath = path;
    var rtsp_tcp_path = path;
    var rtsp_udp_path = path;
    var http_port = 80;
        
    // make sure quicktime is installed, if not default to mjpeg
    if(v_player == 1 && !isQuicktimeInstalled()){
        v_player = 2;
    }
    
    // make sure VLC is installed, if not default to mjpeg
    if(v_player == 3 && !checkVLCIsInstalledOnNav()){
        v_player = 2;
        showHelp('vlc_fail');
        return;
    }
        
    if(location.port) {
        httppath += ":" + location.port;
        http_port = parseInt(location.port);
    }
        
    // if the port is not 80 and the user wants ActiveX or vlc we need to get the port just incase we are using upnp
    if(location.port && location.path != 80 && location.port != net_web_port && (v_player == 0 || v_player == 1 || v_player == 3)){
        // check if we have a result from loadUPnP and makesure if it enabled we have a port to check
        if(upnp_enabled == -1 || (upnp_enabled == 1 && upnp_http_port < 1) || net_web_port == -1){
            // return nothing ... this method will be called again when we have the port
            return ''; 
        }
        
        // if enabled and we are using the same port then set the RTSP port to the port opened by UPnP
        if(upnp_http_port == location.port){
            rtsp_tcp_path += ':' + upnp_tcp_port;
            rtsp_udp_path += ':' + upnp_udp_port;
        } else {
            var tcp_port = http_port+1;
            var udp_port = http_port+2;
        
            // we are using some other port so just presume TCP is current port + 1 and UDP = TCP + 1
            rtsp_tcp_path += ':' + tcp_port;
            rtsp_udp_path += ':' + udp_port;
            
            //alert(rtsp_tcp_path);
        }
    }
                  
    switch(parseInt(v_player)){
        case 0:
            var RtspURL = force_mjpeg ? '<param name="RtspURL" value="http://' + httppath + '/GetData.cgi" />' : '<param name="RtspURL" value="rtsp://' + rtsp_tcp_path + '/webcam" />';
            // pre v4.0 clsid:3FB37917-B6B9-4FBB-920D-254BFBB8D520
            // pre v4.09b clsid:115B1886-2AE0-4259-9FE4-E32A5DEE5451 ActiveX v1.0.0.5
            // pre v5.0 clsid:115B1886-2AE0-4259-9FE4-E32A5DEE5452 ActiveX v1.0.0.6
            // Secured Not Released clsid:115B1886-2AE0-4259-9FE4-E32A5DEE5453 ActiveX v1.0.0.7
            // pre v5.2b5 clasid:115B1886-2AE0-4259-9FE4-E32A5DEE5454 ActiveX v1.0.0.8
            
            // ActiveX v1.0.0.9
            var classid = '115B1886-2AE0-4259-9FE4-E32A5DEE5455';
            
            //alert(RtspURL);
                        
            return ('<object id="oCamCtl" width="' + width + '" height="' + height + '"'
                + ' classid="clsid:' + classid + '"'
                + ' codebase="http://www.wowweesupport.com/download/rovio/WebSee_v1.0.0.9.cab" />'
                + RtspURL
                + '<param name="URL" value="http://' + httppath + '" />'
                + '<param name="AutoStart" value="1" />'
                + '</object>');
            break;
            
         case 1:
            return '<object id="oCamCtl" CLASSID="clsid:02BF25D5-8C17-4B23-BC80-D3488ABDDC6B" width="' + width + '" height="' + height + '" CODEBASE="http://www.apple.com/qtactivex/qtplugin.cab">' +
                    '<param name="src" value="cam.mp4">' +
                    '<param name="qtsrc" value="rtsp://' + rtsp_udp_path + '/webcam">' +
                    '<param name="autoplay" value="true">' +
                    '<param name="loop" value="false">' +
                    '<param name="controller" value="false">' +
                    '<param name="scale" value="ToFit">' +
                    '<param name="volume" value="255">' +
                    '<embed src="cam.mp4" scale="ToFit" volume="255" qtsrc="rtsp://' + rtsp_udp_path + '/webcam" width="' + width + '" height="' + height + '" autoplay="true" loop="false" controller="false" pluginspage="http://www.apple.com/quicktime/"></embed>' +
                    '</object>';
            break;
            
        case 3:
            var unauthTxt = '<div id="oCamCtrl" style="color: white;">VLC player was unable to authenticate the video/audio stream.<a href="" onclick="openRTSPAuthBox(); return false;" style="color: white;">Click here</a> to re-authenticate.</div>';
            if(user_check == -1){
                // need to wait for user_check from status call
                return '';
            }
            
            if(rtspAuth == -1){
                openRTSPAuthBox();
            }
            
            if(rtspAuth != 1){
                return unauthTxt;
            }
            
            var javaInstalled = checkJavaIsInstalledOnNav();
            
            if(rtspAuthUser != ''){
                rtsp_tcp_path = rtspAuthUser + ':' + rtspAuthPass + '@' + rtsp_tcp_path;
            }
            
            if(javaInstalled){
                $('appletPlaceholder').innerHTML = '<applet id="oJavaApplet" code="a2rApplet" width=1 height=1 archive="a2rApplet.jar"></applet>';
                if(rtspAuthUser != ''){
                    setAudioAppletAuth(rtspAuthUser, rtspAuthPass);
                }
                startAudioApplet(path, http_port);
            } else {
                if(!readCookie(COOKIE_JAVA_WARNING)) {
                    // createCookie(COOKIE_JAVA_WARNING, 1, 365);
                    setTimeout("showHelp('java_applet_fail')",7000);
                }
            }
            
            setTimeout("var vlcstate = $('oCamCtl').input.state; if(!settings_dialog_open && !help_dialog_open) { if(vlcstate == 0 || vlcstate == 1 || vlcstate == 6){ if(user_check){ openRTSPAuthBox(); alert('Unable to open video/audio stream. Make sure your username and password is correct.'); } else { alert('Unable to open video/audio stream.'); }}}",15000);
            
            //alert(rtsp_tcp_path);
                        
            return '<embed type="application/x-vlc-plugin" pluginspage="http://www.videolan.org" version="VideoLAN.VLCPlugin.2" id="oCamCtl" name="oCamCtl" autoplay="yes" loop="no" volume="100" width="' + width + '" height="' + height + '" target="rtsp://' + rtsp_tcp_path + '/webcam" />';
            break;
            
        default:
            v_player = 2;
            closeStream();
            stream_id = Math.floor (10000 * Math.random ());
            
            return ('<img id="oCamCtl" src="http://' + httppath + '/GetData.cgi?' + stream_id + '" width="' + width + 'px" height="' + height + 'px" />'); 
            break;
    }
}
function closeStream(){
    if(stream_id != -1) {
        sendCommand("DropData.cgi", "Query=" + stream_id);
    }
    stream_id = -1;
}
function stopVLC(){
    try {
        $('oCamCtl').playlist.stop();
        $('oCamCtl').style.display = 'none';
    } catch(err) {
    }
    
}
function startAudioApplet(address, port){
    if(checkJavaIsInstalledOnNav() && $('oJavaApplet') != null){
        try {
            $('oJavaApplet').setHost(address);
            $('oJavaApplet').setPort(port);
            $('oJavaApplet').startAudioApplet();
        } catch(err) {
        }
    }
}
function restartAudioApplet(){
    if(checkJavaIsInstalledOnNav() && $('oJavaApplet') != null){
        try {
            $('oJavaApplet').startAudioApplet();
        } catch(err) {
        }
    }
}
function stopAudioApplet(){
    if(checkJavaIsInstalledOnNav() && $('oJavaApplet') != null){
        try {
            $('oJavaApplet').stopAudioApplet();
        } catch(err) {
        }
    }
}
function setAudioAppletAuth(username,password){
    if(checkJavaIsInstalledOnNav() && $('oJavaApplet') != null){
        try {
            $('oJavaApplet').setAuthentication(username,password);
        } catch(err) {
        }
    }
}
function changeSettingsBrightness(value){
    $('video_save').disabled = false;
}
function changeMicVol(value){
    value = Math.round(value);

    sendCommand("ChangeMicVolume.cgi","MicVolume=" + value);
}
function changeSpeakerVolResponse(t){
    if(isSafari() && v_player == 1){
        cam_initialized = false;
        resizeCamera();
    }
}
function changeSpeakerVol(value){
    value = Math.round(value);

    if(v_player == 0 && cam_initialized){
        if(!settings_dialog_open){
            if(value == 0 && $('oCamCtl').UseMic) {
                $('oCamCtl').UseMic = false;
            }
            if(value > 0 && !($('oCamCtl').UseMic)) {
                $('oCamCtl').UseMic = true;
            }
        }
    }
    manualRequest("ChangeSpeakerVolume.cgi","SpeakerVolume=" + value,changeSpeakerVolResponse);
}
function changeCamRes(value){
    value = Math.round(value);

    resolution = value;
    $('video_res').value = value;
    
    sendCommand("ChangeResolution.cgi","ResType=" + value);
}
function changeVideoSpeedResponse(t){
    cam_initialized = false;
    resizeCamera();
}
function changeVideoSpeed(value){
    if(slider_only_val) return true;

    value = Math.round(value);

    resolution = parseInt(value / 3);
    compression = value - (parseInt(value / 3) * 3);
    if(compression==0) frame_rate = 10;
    if(compression==1) frame_rate = 20;
    if(compression==2) frame_rate = 30;
    
    params = 'Cmd=ChangeResolution.cgi&ResType=' + resolution;
    params += '&Cmd=ChangeCompressRatio.cgi&Ratio=' + compression;
    params += '&Cmd=ChangeFramerate.cgi&Framerate=' + frame_rate;
        
    manualRequest('Cmd.cgi', params, changeVideoSpeedResponse);
    
    $('video_res').value = resolution;
    $('video_fps').value = frame_rate;
    $('video_quality').value = compression;
    
    setAfterUpdate();
}
function setAfterUpdate(){
    after_update = true;
    if(after_update_timeout_id != -1) clearTimeout(after_update_timeout_id);
    after_update_timeout_id = setTimeout("after_update = false;", 10000);
}
function changeCamCompression(value){
    value = Math.round(value);

    compression = value;
    sendCommand("ChangeCompressRatio.cgi","Ratio=" + value);
}
function changeCamFramerate(value){
    value = Math.round(value);

    frame_rate = value;
    sendCommand("ChangeFramerate.cgi","Framerate=" + value);
}
function changeCamBrightnessResponse(t){
    if(isSafari() && v_player == 1){
        cam_initialized = false;
        resizeCamera();
    }
}
function changeCamBrightness(value, ctr){

    settings_brightness_slider.setValue(value);
    
    value = Math.round(value);
    
    if(brightness != value){
        manualRequest("ChangeBrightness.cgi", "Brightness=" + value,changeCamBrightnessResponse);
        setAfterUpdate();
    }
        
    brightness = value;
}
function isClicked(element){
    if(element.className.indexOf('__clicked') == -1){
        return 0;
    } else {
        return 1;
    }
}
function toggleClickedImg(element){
    if(isClicked(element)){
        changeToUnclickedImg(element);
    } else {
        changeToClickedImg(element);
    }
}
function changeToClickedImg(element){
    if(!isClicked(element)){
        element.className += '__clicked';
    }
}
function changeToUnclickedImg(element){
    if(isClicked(element)){
        classname = element.className;
        element.className = classname.substring(0,classname.lastIndexOf('__clicked'));               
    }
}
function sendMCUCommand(params_arg){
    var mcu_page = 'mcu';
    var params = 'parameters=' + params_arg;
    manualRequest(mcu_page, params, null);
}
function sendCommand(cgi_page,params){
    manualRequest(cgi_page, params, null);
}
function setCamPosTo(pos){
    changeToUnclickedImg($('cam_up'));
    changeToUnclickedImg($('cam_down'));
    changeToUnclickedImg($('cam_mid'));
            
    var cmd = -1;
    switch(pos){
        case 'down':
            //cmd = "00";
            cmd = "12";
            break;
        case 'mid':
            //cmd = "7F";
            cmd = "13";
            break;
        case 'up':
            //cmd = "FF";
            cmd = "11";
            break;
    }
    
    if(cmd != -1){
        sendCommand("rev.cgi", "Cmd=nav&action=18&drive=" + cmd);
        changeToClickedImg($('cam_' + pos));
    }
}
function joystickMove(angle,speed){
    var move_id = 0;
    
    if(angle < 22.5){
        move_id = 'move_forward';
    } else if(angle < 67.5) {
        move_id = 'move_fwd_right';
    } else if(angle < 112.5) {
        move_id = 'move_right';
    } else if(angle < 157.5) {
        move_id = 'move_bck_right';
    } else if(angle < 202.5) {
        move_id = 'move_back';
    } else if(angle < 247.5) {
        move_id = 'move_bck_left';
    } else if(angle < 292.5) {
        move_id = 'move_left';
    } else if(angle < 337.5) {
        move_id = 'move_fwd_left';
    } else if(angle <= 360) {
        move_id = 'move_forward';
    }
    
    startMoving(move_id,speed);
}  
function joystickSnap(x,y,draggable){
    hideToolTip();
    var parent_dimensions = Element.getDimensions(draggable.element.parentNode); 
    var element_dimensions = Element.getDimensions(draggable.element);
    var xMax = parent_dimensions.width - element_dimensions.width;
    var yMax = parent_dimensions.height - element_dimensions.height;
    
    var angle_rad = Math.atan2(x - (xMax / 2),-(y - (yMax / 2)));
           
    if(angle_rad < 0) angle_rad = (Math.PI*2)+angle_rad;
    
    var angle_deg = angle_rad * (180 / Math.PI);
    
    r = (xMax / 2);
    relX = x - (xMax/2);
    relY = y - (yMax/2);
    cirX = r * Math.sin(angle_rad);
    cirY = -r * Math.cos(angle_rad);
    
    if(cirX > 0){
        relX = relX>cirX ? cirX : relX;
    } else {
        relX = relX<cirX ? cirX : relX;
    }
    
    if(cirY > 0){
        relY = relY>cirY ? cirY : relY;
    } else {
        relY = relY<cirY ? cirY : relY;
    }
    
    x = relX + (xMax/2);
    y = relY + (yMax/2);
    
    speed = ((1 - (Math.sqrt(Math.pow(relX,2) + Math.pow(relY,2)) / r)) * (MAX_SPEED-1))+1;
            
    joystickMove(angle_deg,speed);
                            
    return [x,y];
}   
function startMoving(move_id,speed){
    if(speed == null){
        speed = movement_speed;
    }
    if(movement_interval_id != -1){
        clearInterval(movement_interval_id);
    }
    move(move_id,speed);    
    movement_interval_id = setInterval("move('" + move_id + "', " + speed + ")",MOVEMENT_INTERVAL);
}
function move(move_id,speed){
    // lower head if recording
    if(recording && !isClicked($('cam_down'))){
        $('status').innerHTML = 'Lowering head to record';
        setTimeout("$('status').innerHTML = 'Recording';", 4000);
        setCamPosTo('down');
        return;
    }
    
    if(key_shiftdown){
        speed = 7;
    }

    speed = Math.round(speed);
    is_moving = 1;

    var drive_cmd = 0;
            
    switch(move_id){
        case 'rot_left':
            drive_cmd = 5;
            break;
        case 'rot_right':
            drive_cmd = 6;
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
    }
    if(move_id.indexOf('rotr_') != -1){
        var angle = move_id.substr(5);
        //sendMCUCommand('114D4D000100534852540001000112' + angle + '' + (speed < 10 ? '0' + speed : speed) + '00');
        sendCommand("rev.cgi","Cmd=nav&action=18&drive=18&speed=" + speed + "&angle=" + angle);
    } else if(move_id.indexOf('rotl_') != -1) {
        var angle = move_id.substr(5);
        //sendMCUCommand('114D4D000100534852540001000111' + angle + '' + (speed < 10 ? '0' + speed : speed) + '00');    
        sendCommand("rev.cgi","Cmd=nav&action=18&drive=17&speed=" + speed + "&angle=" + angle);
    } else {
        if(drive_cmd){
            sendCommand("rev.cgi","Cmd=nav&action=18&drive=" + drive_cmd + "&speed=" + speed);
        }
    }
}

function stopMoving(){
    is_moving = 0;
    sendCommand("rev.cgi","Cmd=nav&action=33");
    //sendMCUCommand("114D4D000100534852540001000100000000");
    if(movement_interval_id != -1){
        clearInterval(movement_interval_id);
    }
}
function stop(){
    if(recording){
        if(!confirm('Stopping Rovio now will not save the recorded path. Are you sure you want to do this?')){
            return;
        }
    }

    stopMoving();
    sendCommand("rev.cgi","Cmd=nav&action=17");
    
    $('status').innerHTML = 'Roaming';
    
    playing_path = 0;    
    recording = 0;
    if(played_path_id != -1){
        $('path_' + played_path_id).className = "path";
    }
    changeToUnclickedImg($('record'));
}
function setMovementSpeed(value){
    if(value < 1) value = 1;
    if(value > MAX_SPEED) value = MAX_SPEED;
    speed_movement_slider.setValue(value);
    updateMovementSpeeds();
}
function setTurnSpeed(value){
    if(value < 1) value = 1;
    if(value > MAX_SPEED) value = MAX_SPEED;
    speed_turn_slider.setValue(value);
    updateMovementSpeeds();
}
function setRotSpeed(value){
    if(value < 1) value = 1;
    if(value > MAX_SPEED) value = MAX_SPEED;
    speed_angle_slider.setValue(value);
    updateMovementSpeeds();
}
function addPathToList(name){
    name = trim(name);
    if(name == '') return;
    num_of_paths++;
    if($('path_' + name) != null) return;
    
    $('paths').innerHTML = '<div id="path_' + name + '" class="path" onclick="selectPath(getPathName(this.id));" onmousedown="return false;" ondblclick="playPath(getPathName(this.id));" onselectstart="return false;">' + name + '</div>' + $('paths').innerHTML;
}
function removePathFromList(name) {
  $('paths').removeChild($('path_' + name));
  if(selected_path_id == name){
    selected_path_id = -1;
  }
}
var refreshPathListResponse = function(t) {
    var list = t.responseText;

    // solves version bug
    if(list.indexOf("version = ") != -1){  
        setTimeout("refreshPathList()",2000);
        return;
    }
    
    path_list_refreshed = 1;
    
    if(list.indexOf("responses = 0") != -1){
        list = list.substring(list.indexOf("responses = 0")+13);
        if(trim(list).length){
            var paths = list.split('|');
            for(i = 0; i < paths.length; i++){
                addPathToList(paths[i]);
            }
        }
    }
}

function refreshPathList(){
    path_list_refreshed = 0;
    var params = "Cmd=nav&action=6";
    manualRequest("rev.cgi", params, refreshPathListResponse);
    setTimeout("if(!path_list_refreshed) refreshPathList()",5000);
}
function getPathName(e_id){
    return e_id.substring(5);
}
function selectPath(pathname){
    if(recording){ alert('You must stop recording before you can select a path.'); return; }
    if(playing_path) return;
    if($('rename_path') != null) $('rename_path').onblur();
    if($('new_path') != null && $('new_path').style.display != 'none') addNewPath();

    if(selected_path_id != -1){
        if(selected_path_id == pathname){
            rename_timeout_id = window.setTimeout(function () { addRenamePathInput(pathname) },500);
        } else {
            $('path_' + selected_path_id).className = "path";
        }
    }
    selected_path_id = pathname;
    $('path_' + pathname).className = "path_selected";
    
    var now = new Date();
    last_path_click = now.getTime();
}
function findUniquePathName(pathName){
    if($('path_' + pathName) != null){
        var num = 2;
        while($('path_' + pathName + num) != null){
            num++;
        }
        return pathName + num;
    } else {
        return pathName;
    }
}
function addNewPathInput(){
    pathSaved = 0;
    key_events = false;
    $('status').innerHTML = 'Saving New Path';
    $('paths').innerHTML = '<input id="new_path" class="pathinput" value="' + findUniquePathName('New Path') + '" type="text" onblur="addNewPath()" onkeypress="if(isEnterKey(event)){ addNewPath(); return false; } else { return true; }" />' + $('paths').innerHTML;
    $('new_path').focus();
    unselectPath();
    enableSelection();
}
function isEnterKey(e){
    var evtobj = window.event ? event : e;
    var unicode = evtobj.charCode ? evtobj.charCode : evtobj.keyCode;

    if(unicode == 13) {
        return true;
    } else {
        return false;
    }
}
function addRenamePathInput(pathname){
    if(playing_path || recording) return;
    
    enableSelection();
    
    pathSaved = 0;
    rename_timeout_id = -1;
    key_events = false;
    $('status').innerHTML = 'Renaming Path';
    var renameInput = document.createElement('input');
    renameInput.id = "rename_path";
    renameInput.className = "pathinput";
    renameInput.value = pathname;
    renameInput.onblur = function () { renameFinished(pathname); }
    renameInput.onkeypress = function (e) { if(isEnterKey(e)){ renameFinished(pathname); return false; } else { return true; } }
    
    $('path_' + pathname).style.display = "none";
    $('paths').insertBefore(renameInput, $('path_' + pathname));
    $('rename_path').focus();
}
function renameFinished(old_pathname){
    if(pathSaved) return;
    
    disableSelection();
    pathSaved = 1;
    
    newPathName = $('rename_path').value;
    if(trim(newPathName) == ''){
        alert('Please enter a path name.');
        return;
    }
    if(newPathName != old_pathname){
        newPathName = findUniquePathName(newPathName);
        renamePathCmd(old_pathname, newPathName);
    }
    
    $('path_' + old_pathname).innerHTML = newPathName;
    $('path_' + old_pathname).style.display = 'block';
    $('path_' + old_pathname).id = 'path_' + newPathName;
    $('paths').removeChild($('rename_path'));
    
    selected_path_id = newPathName;
                                            
    $('status').innerHTML = 'Roaming';
    key_events = true;
}
function addNewPath(){
    if(pathSaved) return;

    pathSaved = 1;
    
    pathName = $('new_path').value;
    if(trim(pathName) == ''){
        alert('Please enter a path name.');
        return;
    }
    key_events = true;
    $('new_path').value = findUniquePathName(pathName);
    
    disableSelection();
    savePath();
    $('status').innerHTML = 'Roaming';
}
function recordClicked(){
    if(user_guest){
        alert('You must log on as an administrator to record a path.');
        return;
    }
    if(!path_list_refreshed){
        alert('Rovio is refreshing the path list please wait.');
        return;
    }
    if(playing_path || going_home || saving_home){
        alert('Rovio is busy. You must stop Rovio before you can start recording a path.');
        return;
    }
    if(num_of_paths >= MAX_NO_PATHS){
        alert('Rovio can only record up to 10 paths. You must delete a path before you can record a new one.');
        return;
    }
    recordBtn = $('record');
    if(!isClicked(recordBtn) && nav_strength == 5){
        if(!confirm('The navigation signal strength is very weak. Rovio may not be able to record the path properly. Are you sure you want to start recording?')){
            return;                                                                                    
        }
    }
    if(isClicked(recordBtn)){
        recording = false;
        addNewPathInput();
    } else {
        recording = true;
        startRecord();
    }
    toggleClickedImg(recordBtn);
}
function unselectPath(){
    if(selected_path_id != -1){
        $('path_' + selected_path_id).className = "path";
        selected_path_id = -1;
    }
}
function getRadioValue(radioName){
    radioBoxes = document.getElementsByName(radioName);
    for(r_i = 0; r_i < radioBoxes.length; r_i++){
        if(radioBoxes[i].checked){
            return radioBoxes[i].value;
        }
    }
    return null;
}
function setRadioValue(radioName,radioValue){
    radioBoxes = document.getElementsByName(radioName);
    for(r_i = 0; r_i < radioBoxes.length; r_i++){
        radioBoxes[r_i].checked = false;
        if(radioBoxes[r_i].value == radioValue){
            radioBoxes[r_i].checked = true;
        }
    }
}
/* --- Northstar functions --- */
function saveHomeResponse(t){
    var response = t.responseText;
    if(response.indexOf('responses = 0') == -1){
        alert("Unable to save the home position at this time.");
        $('status').innerHTML = 'Roaming';
    } else {
        $('status').innerHTML = 'Home Saved';
        setTimeout("$('status').innerHTML = 'Roaming';", 4000);
    }
}
function saveHome(){
    $('status').innerHTML = 'Saving Home';
    manualRequest("rev.cgi","Cmd=nav&action=14",saveHomeResponse);
}
function checkSaveHome(){
    if(user_guest){
        alert('You must log on as an administrator to save the home position.');
        return;
    }
    if(recording){ alert('You must stop recording before you can save home.'); return; }
    if(nav_strength == 5){
        if(!confirm('The navigation signal strength is very weak. Rovio may not be able to save home properly. Are you sure you want to try to save home?')){
            return;
        }
    }
    if(confirm('This will overwrite your current home position. Are you sure you want to do this?')) saveHome();
}
function checkDock(){
    if(recording){ alert('You must stop recording before you can go home.'); return; }
    if(!home_pos_saved){ alert('The Home position has not been saved. You need to save the Home before Rovio can navigate Home.'); return; }
    if(nav_strength == 5){
        if(!confirm('The navigation signal strength is very weak. Rovio may not be able to navigate properly. Are you sure you want to try to go home?')){
            return;
        }
    }
    goDock();
}
function goDock(){
    going_home = 1;
    sendCommand("rev.cgi","Cmd=nav&action=13");
    $('status').innerHTML = 'Going Home';
}
function startRecord(){
    sendCommand("rev.cgi","Cmd=nav&action=2");
    $('status').innerHTML = 'Recording';
}
function pauseRecord(){
    sendCommand("rev.cgi","Cmd=nav&action=10");
    $('status').innerHTML = 'Recording Paused';
}
function abortRecord(){
    sendCommand("rev.cgi","Cmd=nav&action=3");
    $('status').innerHTML = 'Roaming';
}
var savePathResponse = function(t) {
    if(t.responseText.indexOf("Can't save path") != -1){
        alert('Unable to save path. Please verify that the nearest TrueTrack Beacon is on and in range.');
        removePathFromList($('new_path').value);
    }
    $('paths').removeChild($('new_path'));
}
function savePath(){
    $('new_path').style.display = 'none';
        
    pathname = $('new_path').value;
    pathname = pathname.replace(/\|/g,"");
    
    addPathToList(pathname);
    var params = "Cmd=nav&action=4&name=" + pathname;
    manualRequest("rev.cgi", params, savePathResponse);
}
function renamePathCmd(oldPathName, newPathName){
    sendCommand("rev.cgi","Cmd=nav&action=11&name=" + oldPathName + "&newname=" + newPathName);
}
function confirmDeletePath(){
    if(user_guest){
        alert('You must log on as an administrator to delete a path.');
        return;
    }
    if(selected_path_id == -1){
        alert('Please select a path to delete.');
        return;
    }
    if(confirm('Are you sure you want to delete this path?')) { 
        deleteSelectedPath();
    }
}
function deleteSelectedPath(){
    if(playing_path || going_home || recording || saving_home){
        alert('Rovio is busy. You must stop Rovio before you can delete a path.');
        return;
    }
    if(selected_path_id == -1){
        alert('Please select a path to delete.');
    } else {
        deletePath(selected_path_id);
        removePathFromList(selected_path_id);
    }
}
function resetAll(){
    if(user_guest){
        alert('You must log on as an administrator to reset all paths.');
        return;
    }
    if(playing_path || going_home || recording || saving_home){
        alert('Rovio is busy. You must stop Rovio before you can reset all paths.');
        return;
    }
    if(!confirm('Are you sure you want to DELETE ALL saved paths?')) return;
    num_of_paths = 0;
    
    sendCommand("rev.cgi", "Cmd=nav&action=21");
    
    $('paths').innerHTML = '';
    selected_path_id = -1;
    played_path_id = -1;
}
function deletePath(pathName){
    num_of_paths--;
    sendCommand("rev.cgi","Cmd=nav&action=5&name=" + pathName);
}
function playPath(pathName){
    if(nav_strength == 5){
        if(!confirm('The navigation signal strength is very weak. Rovio may not be able to navigate properly. Are you sure you want to play the path?')){
            return;
        }
    }
    /*if(playing_path){
        alert('Rovio is currently playing a path. You must click stop first if you want to play another path.');
        return;
    }*/
    playing_path = 1;

    if(rename_timeout_id != -1){
        clearTimeout(rename_timeout_id);
        rename_timeout_id = -1;
    }
    selected_path_id = -1;
    if(played_path_id != -1){
        $('path_' + played_path_id).className = "path";
    }
    $('path_' + pathName).className = "path_played";
    played_path_id = pathName;
    
    sendCommand("rev.cgi","Cmd=nav&action=7&name=" + pathName);
    
    update_navstatus = 0;
    setTimeout("update_navstatus = 1", 4000);
    $('status').innerHTML = 'Going to ' + pathName;
}


/* ---- Keyboard ---- */
function getKeyMoveDir(){
    var dir;
    if(key_up){
        if(key_left){
            dir = 'move_fwd_left';
        } else if (key_right){
            dir = 'move_fwd_right';
        } else {
            dir = 'move_forward';
        }
    } else if(key_down){
        if(key_left){
            dir = 'move_bck_left';
        } else if (key_right){
            dir = 'move_bck_right';
        } else {
            dir = 'move_back';
        }
    } else if(key_left){
        dir = 'move_left';
    } else if(key_right){
        dir = 'move_right';
    } else {
        dir = null;
    }
    return dir;
}
function keyboardDown(e){
    if(!key_events){
        key_up=false;key_down=false;key_left=false;key_right=false;key_rot_right = false;key_rot_left = false;
        return;
    }

    if(ctrl_alt_down){
        return;
    }
        
    var evtobj = window.event ? event : e;
    var unicode = evtobj.charCode? evtobj.charCode : evtobj.keyCode;
    
    switch(unicode){
        // ctrl or alt
        case 17:
        case 18:
            ctrl_alt_down = true;
            break;
        case 38:
        case 87:
            if(!key_up){
                key_up = true;
                startMoving(getKeyMoveDir(),movement_speed);
            }
            break;
        case 40:
        case 83:
            if(!key_down){
                key_down = true;
                startMoving(getKeyMoveDir(),movement_speed);
            }
            break;
        case 37:
        case 65:
            if(!key_left){
                key_left = true;
                startMoving(getKeyMoveDir(),movement_speed);
            }
            break;  
        case 39:
        case 68:
            if(!key_right){
                key_right = true;
                startMoving(getKeyMoveDir(),movement_speed);
            }
            break;         
        case 81:
            if(!key_rot_left){
                key_rot_left = true;
                startMoving('rot_left',turn_speed);
            }
            break;
        case 69:
            if(!key_rot_right){
                key_rot_right = true;
                startMoving('rot_right',turn_speed);
            }
            break;
        case 46:
            if(selected_path_id != -1 && confirm('Are you sure you want to delete this path?')){
                deleteSelectedPath();
            }
            break;
        case 16:
            key_shiftdown = 1;
            break;
    }
    
}  
function keyboardPressed(e){
    if(!key_events || ctrl_alt_down){
        return;
    }
    
    var evtobj = window.event ? event : e;
    var unicode = evtobj.charCode? evtobj.charCode : evtobj.keyCode;
        
    switch(unicode){
        case 49: // 1
            setCamPosTo('down');
            break;
        case 50: // 2
            setCamPosTo('mid');
            break;
        case 51: // 3
            setCamPosTo('up');
            break;
        case 72:  // 'h'
        case 104: // 'H'
            checkDock();
            break;
        case 32: 
            stop();
            break;
        case 61: // =
        case 43: // +
            setMovementSpeed(speed_movement_slider.value+1);
            setTurnSpeed(speed_turn_slider.value+1);
            //updateMovementSpeeds();
            break;
        case 95: // _
        case 45: // -
            setMovementSpeed(speed_movement_slider.value-1);
            setTurnSpeed(speed_turn_slider.value-1);
            //updateMovementSpeeds();
            break;
        case 76:
        case 108:
            toggleClickedImg($('headlight'));
            headlight(isClicked($('headlight')));
            break;
    }
}

function keyboardUp(e){
    if(!key_events) return;

    var evtobj = window.event ? event : e;
    var unicode = evtobj.charCode? evtobj.charCode : evtobj.keyCode;
    var dir;
    var start_move = false;
        
    switch(unicode){
        // ctrl or alt
        case 17:
        case 18:
            ctrl_alt_down = false;
            break;
        case 38:
        case 87:
            key_up = false;
            if(getKeyMoveDir() != null){
                startMoving(getKeyMoveDir(),move);   
            } else {
                stopMoving();
            }
            break;
        case 40:
        case 83:
            key_down = false;
            if(getKeyMoveDir() != null){
                startMoving(getKeyMoveDir(),movement_speed);   
            } else {
                stopMoving();
            }
            break;
        case 37:
        case 65:
            key_left = false;
            if(getKeyMoveDir() != null){
                startMoving(getKeyMoveDir(),movement_speed);   
            } else {
                stopMoving();
            }
            break;  
        case 39:
        case 68:
            key_right = false;
            if(getKeyMoveDir() != null){
                startMoving(getKeyMoveDir(),movement_speed);   
            } else {
                stopMoving();
            }
            break;         
        case 81:
            key_rot_left = false;
            stopMoving();
            break;
        case 69:
            key_rot_right = false;
            stopMoving();
            break;
        case 16:
            key_shiftdown = 0;
            break;
    }
}
function headlight(on) {
	sendCommand("rev.cgi","Cmd=nav&action=19&LIGHT="+on);
}
function openSettingsDialog() {
    if(!access_settings_panel){
        alert('Please wait updating settings panel.');
        return;
    }
    if(user_guest){
        alert('Only administrator users are allowed to edit settings.');
        return;
    }

    key_events = false;
    settings_dialog_open = true;
    enableSelection();
    
    if(v_player != 3){
        $('camera_container').innerHTML = '';
    } else {
        stopVLC();
    }
    
    $('settings_dialog').style.display = 'block';
    $('disable_controls').style.display = 'block';
    
    loadSettingsPanels();
    stopAudioApplet();
}
function loadSettingsPanels(){
    loadSMTPSettings();
    loadNetworkPanel();
    loadDynDNSSettings();
    loadSecurityPanel();
}
function openHelpDialog() {
    helpwin = window.open('help/index.html','Help','screenX=' + (screen.width - 670) + ',screenY=0,left=' + (screen.width - 670) + ',top=0,width=850,height=700,location=no,resizable=yes,scrollbars=yes');
    helpwin.focus();
}
function closeSettingsDialog() {
    key_events = true;
    settings_dialog_open = false;
    prevent_resize = false;
    
    $('settings_dialog').style.display = 'none';
    $('disable_controls').style.display = 'none';
    disableSelection();
    if(v_player != 3){
        cam_initialized = false;
        resizeCamera();
    } else {
        restartAudioApplet();
        cam_initialized = true;
        
        if($('camera_container').innerHTML != ''){
            try {
                $('oCamCtl').style.display = 'block';
                $('oCamCtl').playlist.start();
            } catch(err) {
            }
        } else {
            cam_initialized = false;
            resizeCamera();
        }
    }
}
 /* --- Authentication Functions --- */
function loadSecurityPanel(){
    refreshUserList();
    updateUserCheck();
}
function addUserToList(username,admin){
    username = username.replace(/\s/g, "_");
    username_with_spaces = username.replace(/_/g, " ");
    
    var tbl = document.getElementById('added_users');
    var lastRow = tbl.rows.length;
    var row = tbl.insertRow(lastRow);
    row.id = username + "_row";
    var username_cell = row.insertCell(0);
    username_cell.className = "user_name";
    username_cell.innerHTML = username_with_spaces;
    var admin_cell = row.insertCell(1);
    admin_cell.id = username + "_admin";
    if(admin){
        admin_cell.innerHTML = "[Admin access]";
    } else {
        admin_cell.innerHTML = "[Guest access]";
    }
    var remove_cell = row.insertCell(2);
    if(username.toLowerCase() != 'administrator' || username.toLowerCase() != 'admin'){
        remove_cell.innerHTML = '&nbsp;&nbsp;<input type="button" id="edit_' + username + '" onclick="showEditUserForm(\'' + username_with_spaces + '\', ' + admin + ');" value="Edit" />&nbsp;&nbsp;<input type="button" id="remove_' + username + '" onclick="confirmDeleteUser(\'' + username_with_spaces + '\', ' + admin + ');" value="Delete" />';
    }
}
function getMyselfResponse(t){
    var tstr = t.responseText;
    var varArr = tstr.split(" = ");
    
    if(varArr[1] != null){
        user_id = trim(varArr[1]);
    } else {
        user_id = '';
    }
}
function getMyself(){
    if(user_id == -1){
        manualRequest("GetMyself.cgi", "", getMyselfResponse);
        setTimeout("getMyself()", 5000);
    }
}
function confirmDeleteUser(username, admin){
    if(user_id == -1){
        getMyself();
        setTimeout("confirmDeleteUser('" + username + "', '" + admin + "');", 1000);
        return;
    }
    if(user_id == username){
        alert('You cannot delete this user account because you are currently using it.');
        return;
    }
    
    if(admin){
        var user_tbl_obj = $('added_users');
        var num_users = user_tbl_obj.rows.length;
        var num_admins = 0;
        for(i = 0; i < num_users; i++){
            if(user_tbl_obj.rows[i].cells[1].innerHTML.indexOf('Admin') != -1){
                num_admins++;
            }
        }
        
        if(num_admins < 2){
            alert('Unable to delete user. You must have at least one admin user.');
            return;
        }
    }
    
    if(confirm("Are you sure you want to delete this user?")){
        deleteUser(username);
    }
}
function deleteUserFromList(username){
    username = username.replace(/\s/g, "_");
    var user_row = $(username + '_row');
    user_row.parentNode.removeChild(user_row);
}
function addUser(){
    var username = $('user_name').value;
    var password = $('user_password').value;
    var admin = $('user_admin').checked ? 1 : 0;
    var editmode = $('user_name').disabled;
    
    if(trim(username) == ''){
        $('user_error').innerHTML = 'Please enter a name for the user.';
        return;
    }
    if(!editmode && $(username.replace(/\s/g, "_") + '_row') != null){
        $('user_error').innerHTML = 'A user with the same username already exists.<br/>Please enter a different username.';
        return;
    }
    if(password != $('user_confirm').value){ 
        $('user_error').innerHTML = 'The passwords entered are not the same.';
        $('user_password').value = '';
        $('user_confirm').value = '';
        return;
    }
    if(password.length < 6){ 
        $('user_error').innerHTML = 'The password must be at least 6 characters long.';
        $('user_password').value = '';
        $('user_confirm').value = '';
        return;
    }
    sendCommand("SetUser.cgi", "User=" + username + "&Pass=" + password + "&Privilege=" + admin);
    cancelUserForm();
    
    if(!editmode){
        addUserToList(username,admin);
    } else {
        if(admin){
            $(username.replace(/\s/g, "_") + "_admin").innerHTML = "[Admin access]";
        } else {
            $(username.replace(/\s/g, "_") + "_admin").innerHTML = "[Guest access]";
        }
        alert('User saved.');
    }
}
function resetUserForm(){
    $('user_name').value = '';
    $('user_password').value = '';
    $('user_confirm').value = '';
    $('user_error').innerHTML = '';
    $('user_admin').checked = false;
}
function showNewUserForm(){
    resetUserForm();
    $('user_form').style.display = 'block';
    $('user_form_hd').innerHTML = 'New User';
    $('user_name').disabled = false;
    $('add_user').value = 'Add User';
}
function showEditUserForm(username, admin){
    resetUserForm();
    $('user_form').style.display = 'block';
    $('user_form_hd').innerHTML = 'Edit User';
    $('user_name').value = username;
    $('user_name').disabled = true;
    $('user_admin').checked = admin;
    $('add_user').value = 'Save';
}
function cancelUserForm(){
    resetUserForm();
    $('user_form').style.display = 'none';
}
function deleteUser(username){
    sendCommand("DelUser.cgi", "User=" + username);
    deleteUserFromList(username);
}
function refreshUserList(){
    var params = "ShowPrivilege=true";
    manualRequest("GetUser.cgi", params, refreshUserListResponse);
}
var refreshUserListResponse = function(t) {
    var result = parseVars(t.responseText);
    
    var n_i = 0;
    var names = new Array();
    var a_i = 0;
    var admin = new Array();
    
    var user_tbl_obj = $('added_users');
    var num_users = user_tbl_obj.rows.length;
    for(i = 0; i < num_users; i++){
        user_tbl_obj.deleteRow(0);
    }
       
    for(i = 0; i < result.length; i++){
        if(result[i].key == 'Name'){
            names[n_i] = result[i].value;
            n_i++;
        }
        if(result[i].key == 'Privilege'){
            admin[a_i] = result[i].value;
            a_i++;
        }
    }
    for(i = 0; i < names.length; i++){
        addUserToList(names[i],!parseInt(admin[i]));
    }
}

function updateUserCheck(){
    $('user_check').checked = parseInt(user_check);
    $('user_update_auth').disabled = true;
}


/* --- Network Form Functions --- */
function loadNetworkPanel(){
    $('net_mac').disabled = true;
    
    loadNetworkFields();
    
    initial_upnp_load = true;
    loadUPnPFields();
}
var loadWebPortResponse = function(t) {
     var result = parseVars(t.responseText);
    
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        if(k == 'Port0'){
            net_web_port = parseInt(v);
        }
    }
}
function loadWebPort(){
    var params = "Cmd=GetHttp.cgi";
    manualRequest("Cmd.cgi", params, loadWebPortResponse);
}
var loadNetworkFieldsResponse = function(t) {
    var result = parseVars(t.responseText);
    
    for(i = 0; i < result.length; i++){
        
        k = result[i].key;
        v = result[i].value;
        
        if(k != null && v != null){            
            switch(k){
                case 'IPWay':
                    setRadioValue('ip_setting', v);
                    break;
                case 'ESSID':
                    essid = v;
                case 'Mode':
                case 'Netmask':
                case 'Gateway':
                case 'DNS0':
                case 'MAC':
                    $('net_' + k.toLowerCase()).value = v;
                    break;
                case 'IP':
                    $('net_' + k.toLowerCase()).value = v;
                    $('net_ip_hidden').value = v;
                    break;
                case 'Port0':
                    $('net_port').value = v;
                    net_web_port = parseInt(v);
                    break;
                case 'CurrentIP':
                case 'CurrentNetmask':
                case 'CurrentGateway':
                case 'CurrentDNS0':
                    if($('net_ip_auto').checked){
                        $('net_' + k.substr(7).toLowerCase()).value = v;
                    }
                    break;
            }
        }
    }
    
    $('net_key').value = '$$**__**$$';
    $('net_confirm').value = '$$**__**$$';
    $('net_key_changed').checked = false;
    
    /*if($('net_ip_auto').checked){
        $('net_ip').value = '';
        $('net_netmask').value = '';
        $('net_gateway').value = '';
        $('net_dns0').value = '';
    }*/
    
    $('net_save').disabled = true;
    $('net_wifi_changed').checked = false;
    $('net_upnp_changed').checked = false;
    
    if($('net_mode').value=='Ad-Hoc'){ 
        $('net_key').disabled = true; 
        $('net_confirm').disabled = true; 
        $('net_key').value = '';
        $('net_confirm').value = '';
    } else { 
        $('net_key').disabled = false;
        $('net_confirm').disabled = false; 
    }
    
    updateIPFields();
}
function loadNetworkFields(){
    var params = "Cmd=GetWlan.cgi&Cmd=GetIP.cgi&Cmd=GetMac.cgi&Cmd=GetHttp.cgi";
    manualRequest("Cmd.cgi", params, loadNetworkFieldsResponse);
}
function resetNetworkFields(){
    $('net_essid').value = 'rovio_wowwee';
    $('net_mode').selectedIndex = 1;
    $('net_port').value = 80;
    $('net_key').value = '';
    $('net_confirm').value = '';
    $('net_ip_man').checked = true;
    $('net_ip').value = '192.168.10.18';
    $('net_netmask').value = '255.255.255.0';
    $('net_gateway').value = '192.168.1.18';
    $('net_dns0').value = '0.0.0.0';
    $('net_upnpenabled').checked = 1;
    $('net_upnpport').value = '8168';
    $('net_internetip').value = '';
    
    $('net_verify_access').checked = 1;
    $('net_include_status').checked = 1;
        
    updateIPFields();
    updateUPnPFields();
}
function updateIPFields(){   
    var IPInputs = $('IPFields').getElementsByTagName('input');
    for(i = 0; i < IPInputs.length; i++){
        IPInputs[i].disabled = $('net_ip_auto').checked;
    }
}
var loadUPnPResponse = function(t) {
    var result = parseVars(t.responseText);
    var upnpip, port;
       
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        switch(k){
            case 'Enable':
                upnp_enabled = parseInt(v);
                if(initial_upnp_load){
                    if(parseInt(v)){
                        $('net_upnpenabled').checked = 1;
                    } else {
                        $('net_upnpdisabled').checked = 1;
                    }
                }
                break;
            case 'Port':
                if(initial_upnp_load){
                    $('net_upnpport').value = parseInt(v);
                }
                upnp_port = v;
                break;
            case 'IP':
                upnp_ip = v;
                break;
            case 'HTTP':
                upnp_http_port = parseInt(v);
                break;
            case 'RTSP_TCP':
                upnp_tcp_port = parseInt(v);
                break;    
            case 'RTSP_UDP':
                upnp_udp_port = parseInt(v);
                break;        
        }
    }
    
    updateOnlineStatus();
    updateUPnPFields();
    
    initial_upnp_load = false;
}
function loadUPnPFields(){
    manualRequest("GetUPnP.cgi", "", loadUPnPResponse);
}

var loadPPPResponse = function(t) {
    var result = parseVars(t.responseText);
       
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        switch(k){
            case 'Status':
                ppp_status = v;
                break;
            case 'User':
                ppp_user = v;
                break;
            case 'Enable':
                ppp_enabled = v;
                break;    
        }
    }
    updateOnlineStatus();
}
function loadPPPFields(){
    manualRequest("GetPPP.cgi", "", loadPPPResponse);
}

function updateNetStatus(status){
    var settings_status = '';
    var update = true;
    var o_status = '';
    var popup_help = '';
    external_address = '';

    switch(status){
        /* --- UPnP enabled --- */
        case 'openning_ports':
            settings_status = '<div class="online_connecting_settings">Connecting to Router to open ports... (this may take several minutes)</div>';
            o_status = 'connecting';
            online_status_txt = 'Connecting to Router to open ports... (this may take several minutes)';
            online_status_help_id = 'upnp_connecting_help';
            external_address = '----';
            break;
        case 'failed_to_open_ports':
            settings_status = '<div class="online_error_settings">Failed to open ports</div>' + returnHelpBtn('upnp_failed_help');
            external_address = '----';
            o_status = 'error';
            online_status_txt = 'Failed to open ports';
            online_status_help_id = 'upnp_failed_help';
            //popup_help = 'upnp_failed_popup';
            break;
        case 'openned_port_verifying_access':
            settings_status = '<div class="online_connecting_settings">Openned port ' + upnp_http_port + ', verifying access from internet...</div>';  
            o_status = 'connecting';
            online_status_txt = 'Openned port ' + upnp_http_port + ', verifying access from internet';
            online_status_help_id = 'all_verifying_help';
            break;
        case 'openned_port_checking_ip':
            settings_status = '<div class="online_connecting_settings">Openned port ' + upnp_http_port + ', getting external IP address...</div>';  
            o_status = 'connecting';
            online_status_txt = 'Openned port ' + upnp_http_port + ', getting external IP address';
            online_status_help_id = '';
            break;
        case 'openned_port_no_ip':
        case 'no_ip':
            settings_status = '<div class="online_error_settings">Unable to get external IP address</div>' + returnHelpBtn('upnp_noip_help');
            external_address = 'Unable to get external IP address' + returnHelpBtn('upnp_noip_help');
            online_status_txt = 'Unable to get external IP address. Click icon for to view help.';
            o_status = 'error';
            online_status_help_id = 'upnp_noip_help';
            break;
        /* --- UPnP disabled --- */
        case 'verifying_domain':
            settings_status = '<div class="online_connecting_settings">Verifying access from internet to ' + dns_domain + ' on port ' + upnp_http_port + '...</div>';
            online_status_txt = 'Verifying access from internet to ' + dns_domain + ' on port ' + upnp_http_port;
            o_status = 'connecting';
            online_status_help_id = 'all_verifying_help';
            break;
        case 'verifying_ip':
            settings_status = '<div class="online_connecting_settings">Verifying access from internet to ' + upnp_ip + ' on port ' + upnp_http_port + '...</div>';
            online_status_txt = 'Verifying access from internet to ' + upnp_ip + ' on port ' + upnp_http_port;
            o_status = 'connecting';
            online_status_help_id = 'all_verifying_help';
            break;
        case 'no_ip_or_domain':
            settings_status = '<div class="online_error_settings">No IP or domain to verify</div>';
            online_status_txt = 'No IP or domain to verify';
            o_status = 'error';
            online_status_help_id = 'no_ip_or_domain';
            external_address = '---';
            break;
        /* --- External verification --- */
        case 'domain_verified':
            settings_status = '<div class="online_ok_settings">Internet access verified</div>';
            external_address = '<a href="http://' + dns_domain + ':' + upnp_http_port + '" class="url">http://' + dns_domain + ':' + upnp_http_port + '</a>';
            o_status = 'ok';
            online_status_txt = 'Internet access verified on http://' + dns_domain + ':' + upnp_http_port;
            online_status_help_id = 'online_ok_help';
            break;
        case 'ip_verified':
            settings_status = '<div class="online_ok_settings">Internet access verified</div>';
            external_address = '<a href="http://' + upnp_ip + ':' + upnp_http_port + '" class="url">http://' + upnp_ip + ':' + upnp_http_port + '</a>';
            online_status_txt = 'Internet access verified on http://' + upnp_ip + ':' + upnp_http_port;
            o_status = 'ok';
            online_status_help_id = 'online_ok_help';
            break;
        case 'ip_not_verified':
            settings_status = '<div class="online_unknown_settings">Unable to verify internet access to your Rovio</div>' + returnHelpBtn('verify_ip_help');
            external_address = '<a href="http://' + upnp_ip + ':' + upnp_http_port + '" class="urlnoaccess">http://' + upnp_ip + ':' + upnp_http_port + '</a>';
            online_status_txt = 'Unable to verify internet access to your Rovio. Click icon for to view help.';
            o_status = 'unknown';
            online_status_help_id = 'verify_ip_help';
            //popup_help = 'ip_net_verified_popup';
            break;
        case 'ip_verified_domain_not_verified':
            settings_status = '<div class="online_unknown_settings">Unable to verify internet access to your Rovio</div>' + returnHelpBtn('verify_domain_help');
            external_address = '<a href="http://' + upnp_ip + ':' + upnp_http_port + '" class="urlnoaccess">http://' + upnp_ip + ':' + upnp_http_port + '</a>';
            online_status_txt = 'Unable to verify internet access using domain. Click icon for to view help.';
            online_status_help_id = 'verify_domain_help';
            o_status = 'unknown';
            //popup_help = 'ip_verified_domain_not_popup';
            break;
        case 'access_not_verified':
            settings_status = '<div class="online_unknown_settings">Unable to verify internet access to your Rovio</div>' + returnHelpBtn('verify_all_help');
            external_address = '---';
            online_status_txt = 'Unable to verify internet access to your Rovio. Click icon for to view help.';
            online_status_help_id = 'verify_all_help';
            o_status = 'unknown';
            break;
        case 'verify_internet_access_disabled':
            settings_status = '<div class="online_unknown_settings">Verify internet access disabled</div>' + returnHelpBtn('verify_all_help');
            if(dns_enable){
                external_address = '<a href="http://' + dns_domain + ':' + upnp_http_port + '" class="url">http://' + dns_domain + ':' + upnp_http_port + '</a>';
            } else if(upnp_ip != null && upnp_ip != -1 && upnp_ip != ''){
                external_address = '<a href="http://' + upnp_ip + ':' + upnp_http_port + '" class="url">http://' + upnp_ip + ':' + upnp_http_port + '</a>';
            } else {
                external_address = '---';    
            }
            online_status_txt = 'Verify internet access disabled.';
            online_status_help_id = 'verify_internet_access_disabled';
            o_status = 'unknown';
            break;
        case 'getting_settings':
            settings_status = '<div class="online_connecting_settings">Getting settings</div>';
            external_address = '---';
            online_status_txt = 'Getting settings';
            online_status_help_id = '';
            o_status = 'connecting';
            break;
        case 'ppp_success':
            settings_status = '<div class="online_ok_settings">Connected to portal</div>';
            external_address = '<a href="' + portal_address + '" class="url">' + portal_address + '</a>';
            o_status = 'ok';
            online_status_txt = 'To access your Rovio outside your local network log in to the WowWee Portal at ' + external_address;
            online_status_help_id = 'portal_success';
            break;
        case 'ppp_failed':
            settings_status = '<div class="online_error_settings">Failed to connect to portal</div>';
            online_status_txt = 'Unable to connect Rovio to the portal';
            o_status = 'error';
            online_status_help_id = 'portal_fail';
            external_address = '---';
            break;
        default:
            update = false;
            break;
        
    }
    if(update){
        $('net_status').innerHTML = settings_status;
        $('net_external_address').innerHTML = external_address;
        o_status = 'online_' + o_status;
        $('online_status').className = o_status;
        
        if(upnp_just_enabled && popup_help != ''){
            showHelp(popup_help);
            setParameter(i_UPnP,0);
            upnp_just_enabled = 0;
        }
    }
}



function updateOnlineStatus(){
    var d = new Date();
    
    if(ppp_firmware_enabled){
        if(ppp_enabled == -1){
            loadPPPFields();
            return;
        }
    }
    
    if(ppp_enabled){
        if(ppp_status == 'online'){
            updateNetStatus('ppp_success');
        } else {
            updateNetStatus('ppp_failed');
            setTimeout('loadPPPFields()', 10000);
        }
    } else {    
        // update upnp status
        if(upnp_enabled == 1){
            if(upnp_tcp_port == 0){
                // --- Ports not open ---
                if(d.getTime() - initial_load_time > 120000){
                    updateNetStatus('failed_to_open_ports');
                    
                    // clear interval and check every 30 seconds
                    if(loadUPnPFields_interval_id != -1){
                        clearInterval(loadUPnPFields_interval_id);
                        loadUPnPFields_interval_id = -1;
                    }
                    loadUPnPFields_interval_id = setInterval('loadUPnPFields()', 30000);
                } else {
                    updateNetStatus('openning_ports');
                    
                    // while connecting check every 5 seconds
                    if(loadUPnPFields_interval_id == -1){
                        loadUPnPFields_interval_id = setInterval('loadUPnPFields()', 5000);
                    }
                }
            } else {
                // --- Successfully openned ports ---
                            
                // clear interval as ports are open
                if(loadUPnPFields_interval_id != -1){
                    clearInterval(loadUPnPFields_interval_id);
                    loadUPnPFields_interval_id = -1;
                }
                            
                // if using a domain or we can get an ip then verify access
                if(dns_enable || (upnp_ip != null && trim(upnp_ip) != '')){
                    // ports open check external access
                    updateNetStatus('openned_port_verifying_access');
                    checkExternalAccess();
                } else {
                    // if not then keep checking for an IP
                    loadUPnPFields_interval_id = setInterval('loadUPnPFields()', 20000);
                    updateNetStatus('openned_port_checking_ip');
                }
            }
            
            
            // if after 4 minutes stop checking
            if(d.getTime() - initial_load_time > 240000){
                if(loadUPnPFields_interval_id != -1){
                    clearInterval(loadUPnPFields_interval_id);
                    loadUPnPFields_interval_id = -1;
                }
                // if just enabled
                if(upnp_just_enabled){
                    // not successful
                    if(upnp_tcp_port == 0){
                        updateNetStatus('failed_to_open_ports');
                    } else {
                        // successful but no ip
                        if(upnp_ip == null || trim(upnp_ip) == ''){
                            updateNetStatus('openned_port_no_ip');
                        
                            showHelp('upnp_openned_noip_popup');
                            setParameter(i_UPnP,0);
                        }
                    }
                }
            }
            
        } else {
            // presuming manually openned
            if(manual_internetip != -1){
                upnp_ip = manual_internetip;
            }
            if(trim($('net_upnpport').value) != ''){
                upnp_http_port = parseInt($('net_upnpport').value);
            } else {
                upnp_http_port = net_web_port;
            }
            upnp_tcp_port = upnp_http_port + 1;
            upnp_udp_port = upnp_http_port + 2;
            if(initial_upnp_load) checkExternalAccess();
        }
    }
}
function checkExternalIPResponse(t){
    if(t.responseText.indexOf('<!--no response-->') == 0){
        external_ip_status = -3;
    } else {
        external_ip_status = parseInt(t.responseText);
    }
    //alert('IP status: ' + external_ip_status);
    setTimeout("updateExternalStatus();", 1000);
}
function checkExternalDomainResponse(t){
    if(t.responseText.indexOf('<!--no response-->') == 0){
        external_domain_status = -3;
    } else {
        external_domain_status = parseInt(t.responseText);
    }
    //alert('Domain status: ' + external_domain_status);
    setTimeout("updateExternalStatus();", 1000);
}
function updateExternalStatus(){
    // user has set up a domain and it works
    if(external_domain_status == 0){
        updateNetStatus('domain_verified');
    
    // no domain set up
    } else if(external_domain_status == -2) { 
        switch(external_ip_status){
            // ip successful
            case 0:
                updateNetStatus('ip_verified');
                break;
            // can't get ip
            case -2:
                updateNetStatus('no_ip');
                break;
            // not set    
            case -1:
                break;
            // can't verify access to ip
            default:
                updateNetStatus('ip_not_verified');
                break;
        }
        
    // domain set up but doesn't work
    } else {
        switch(external_ip_status){
            // ip successful but domain failed
            case 0:
                updateNetStatus('ip_verified_domain_not_verified');
                break;
            // not set
            case -1:
                break;
            // unable to get IP address
            case -2:
                updateNetStatus('access_not_verified');
                break;
            // can't verify IP or domain
            default:
                updateNetStatus('access_not_verified');
                break;
        }
    }
}
function pingTest(address,port,returnFunction){
    var params = 'Server=www.wowweesupport.com&Path=/product/tech/rovio/tools/ping.php';
    params += '%3fip=' + address + '%26port=' + port;
    params += '&Transparent=true';
    manualRequest('SendHttp.cgi',params,returnFunction,'GET');
}
function tryToGetIPResponse(t){
    if(t.responseText.indexOf('<!--no response-->') == 0){
        $('net_internetip').value = '';
        showHelp('unable_to_getip_help');
    } else {
        $('net_internetip').value = t.responseText;
        $('net_save').disabled = false;
    }
}
function tryToGetIP(){
    var params = 'Server=www.wowweesupport.com&Path=/product/tech/rovio/tools/getip.php';
    params += '&Transparent=true';
    manualRequest('SendHttp.cgi',params,tryToGetIPResponse,'GET');
}
function getLatestVersionResponse(t){
    var result = parseVars(t.responseText);
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        
        if(k == 'message'){
            if(v != ''){
                new_version_msg = v;
                showHelp('new_version_msg_popup');
            }
        }
    }
}
function getLatestVersion(){
    if(firmware_alert == 1){
        var params = 'Server=www.wowweesupport.com&Path=/product/tech/rovio/tools/vchk.php?v=' + UI_VERSION;
        //var params = 'Server=192.168.10.231&Path=/vchk.php?v=' + UI_VERSION;
        params += '&Transparent=true';
        
        manualRequest('SendHttp.cgi',params,getLatestVersionResponse,'GET');
    }
}
function saveFirmwareAlert(){
    if($('firmware_alert_box').checked != firmware_alert){
        firmware_alert = $('firmware_alert_box').checked ? 1 : 0;
        setParameter(i_NFA,firmware_alert);
        $('firmware_alert_btn').disabled = true;
        alert('Firmware alert setting updated.');
        setTimeout('getLatestVersion()', 5000);
    }
}
/* Check external access is initiated from load updateOnlineStatus but is also manually called if the manual external IP changes
 * or the dyn dns settings change */
function checkExternalAccess(){
    if(verify_internet_access == 1){
        external_ip_status = -1;
        external_domain_status = -1;
        
        // check IP address if we have one
        if(upnp_ip != -1 && upnp_ip != null && trim(upnp_ip) != ''){
            updateNetStatus('verifying_ip');
            pingTest(upnp_ip,upnp_http_port,checkExternalIPResponse);
        } else {
            external_ip_status = -2;
        }
        // check domain address if we have one
        if(dns_enable && dns_domain != -1 && trim(dns_domain) != ''){
            updateNetStatus('verifying_domain');
            pingTest(dns_domain,upnp_http_port,checkExternalDomainResponse);
        } else {    
            external_domain_status = -2;
        }
        
        if(external_domain_status == -2 && external_ip_status == -2){
            updateNetStatus('no_ip_or_domain');
        }
        
        if(checkaccess_interval_id == -1){
            checkaccess_interval_id = setInterval('checkExternalAccess()', 120000);
        }
    } else {
        if(verify_internet_access != -1){
            if(checkaccess_interval_id != -1){
                clearInterval(checkaccess_interval_id);
            }
            updateNetStatus('verify_internet_access_disabled');
        } else {
            updateNetStatus('getting_settings');
            // check again when the verify_internet_access variable has been set
            setTimeout('checkExternalAccess()', 5000);
        }
    }
}
function updateUPnPFields(){
    $('net_internetip').disabled = $('net_upnpenabled').checked;
    $('net_getip_btn').disabled = $('net_upnpenabled').checked;
}
function updateDynDomainFields(){   
    /*var DynDNSFlds = $('DynDNSFields').getElementsByTagName('input');
    for(i = 0; i < DynDNSFlds.length; i++){
        DynDNSFlds[i].disabled = $('dyn_enable').checked ? false : true;
    }
    $('dyn_service').disabled = $('dyn_enable').checked ? false : true;*/
}
function outputChannelOpts(){
    var s = '';
	for (var i = 0; i < 10; i++)
	    s += '<option value="'+i+'">'+i+'</option>';
    document.write(s);
}
function openPhotoAndSend(){
    var picwin;
    $('fake_link').focus();
    picwin=window.open('snapshot.htm','photo','width=640,height=480,toolbar=0,resizable=0');
    picwin.focus();
    
    if(smtp_enable) {
        if($('net_mode').value == 'Ad-Hoc'){
            picwin.alert("Photo's can not be emailed in Ad hoc mode.");
        } else {
            //manualRequest("SendMail.cgi", '');
            manualRequest("rev.cgi", 'Cmd=nav&action=26');
            picwin.alert('Photo emailed.');
        }
    }
}
function loadSMTPSettingsResponse(t){
    var result = parseVars(t.responseText);
    
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        switch(k){
            case 'MailServer':
                smtp_server = v;
                break;
            case 'Sender':
                smtp_sender = v;
                break;
            case 'Receiver':
                smtp_receiver = v;
                break;
            case 'Subject':
                smtp_sub = v;
                break;
            case 'Body':
                smtp_body = v;
                break;
            case 'Port':
                smtp_port = v;
                break;
            case 'User':
                smtp_user = v;
                break;
            case 'PassWord':
                smtp_pass = v;
                break;
            case 'CheckFlag':
                smtp_check = v;
                break;
            case 'Enable':
                smtp_enable = parseInt(v);
                break;
        }
    }
    if(smtp_server == -1){
        setTimeout("loadSMTPSettings()",2000);
        return;
    }
    $('photo_server').value = smtp_server;
    $('photo_user').value = smtp_user;
    $('photo_pass').value = smtp_pass;
    $('photo_confirm').value = smtp_pass;
    $('photo_sender').value = smtp_sender;
    $('photo_receiver').value = smtp_receiver;
    $('photo_sub').value = smtp_sub;
    $('photo_body').value = smtp_body;
    $('photo_enable').checked = smtp_enable;
    $('photo_port').value = smtp_port;
    
    $('photo_update').disabled = true;
}
function loadSMTPSettings(){
    manualRequest("GetMail.cgi", '', loadSMTPSettingsResponse);
}
function saveSMTPSettingsResponse(){
    $('photo_update').disabled = true;

    if($('net_mode').value == 'Ad-Hoc'){
        alert('SMTP settings updated. However, photos will not be emailed while Rovio is in Ad hoc mode.');
    } else {
        alert('SMTP settings updated.');
    }
}
function saveSMTPSettings(){
    if($('photo_pass').value != $('photo_confirm').value){
        alert('The two passwords are not the same.');
        return;
    }

    smtp_server = trim($('photo_server').value);
    smtp_user = trim($('photo_user').value);
    smtp_pass = $('photo_pass').value;
    smtp_sender = $('photo_sender').value;
    smtp_receiver = $('photo_receiver').value;
    smtp_sub = $('photo_sub').value;
    smtp_body = $('photo_body').value;
    smtp_port = trim($('photo_port').value);
    smtp_enable = $('photo_enable').checked ? 1 : 0;

    var params = 'MailServer=' + chkStr(smtp_server);
    params += '&Sender=' + chkStr(smtp_sender);
    params += '&Receiver=' + chkStr(smtp_receiver);
    params += '&Subject=' + chkStr(smtp_sub);
    params += '&Body=' + chkStr(smtp_body);
    params += '&Port=' + chkStr(smtp_port);
        
    params += '&User=' + chkStr(smtp_user);
        
    if(trim(smtp_pass) != ''){
        params += '&CheckFlag=CHECK';
    }
    params += '&PassWord=' + chkStr(smtp_pass);
    
    params += '&Enable=' + smtp_enable;
   
    manualRequest("SetMail.cgi", params, saveSMTPSettingsResponse);
}
function resetSMTPSettings(){
    $('photo_server').value = '';
    $('photo_user').value = '';
    $('photo_pass').value = '';
    $('photo_confirm').value = '';
    $('photo_sender').value = '';
    $('photo_receiver').value = '';
    $('photo_sub').value = 'Rovio Snapshot';
    $('photo_body').value = 'Check out this photo from my Rovio.';
    $('photo_enable').checked = 0;
    $('photo_port').value = '25';
}
/*
function updateSMTPFields(){
    var SMTPFields = $('photoFields').getElementsByTagName('input');
    for(i = 0; i < SMTPFields.length; i++){
        SMTPFields[i].disabled = $('photo_enable').checked ? false : true;
    }
}*/
function saveNetworkSettingsResponse(t){
    $('net_save').disabled = true;
    $('net_key_changed').checked = false;
    $('net_wifi_changed').checked = false;
    $('net_upnp_changed').checked = false;
    alert("Network settings updated.");
}

function saveNetworkSettings(){
    var params = '';
        
    if($('net_include_status').checked != show_online_status){
        show_online_status = $('net_include_status').checked ? 1 : 0;
        setParameter(i_SS, show_online_status);
        updateShowOnlineStatus();
    }
    if($('net_internetip').value != manual_internetip){
        manual_internetip = $('net_internetip').value;
        upnp_ip = manual_internetip; // presuming not using upnp
        saveIP(manual_internetip);
        checkExternalAccess();
    }
    if($('net_verify_access').checked != verify_internet_access){
        verify_internet_access = $('net_verify_access').checked ? 1 : 0;
        setParameter(i_VIA, verify_internet_access);
        checkExternalAccess();
    }
    
    if($('net_wifi_changed').checked || $('net_upnp_changed').checked){
        if($('net_upnpenabled').checked != upnp_enabled || $('net_upnpport').value != upnp_port){
            params += '&Cmd=SetUPnP.cgi';
            params += '&Enable=' + ($('net_upnpenabled').checked ? '1' : '0') + '&Port=' + $('net_upnpport').value + '&';
                    
            // if user is turning on upnp
            if($('net_upnpenabled').checked && !upnp_enabled){
                setParameter(i_UPnP,1);
            }
        }
            
        params += 'Cmd=SetWlan.cgi';
        params += '&ESSID=' + chkStr($('net_essid').value);
        params += '&Mode=' + $('net_mode').value;
        
        if($('net_mode').value != 'Ad-Hoc'){
            if($('net_key_changed').checked){
                params += '&Key=' + chkStr($('net_key').value);
            }
        } else {
            params += '&Key=';
        }
      
        params += '&Cmd=SetIP.cgi';
        if($('net_ip_man').checked){
            params += '&IPWay=' + $('net_ip_man').value;
            params += '&IP=' + $('net_ip').value;
            params += '&Netmask=' + $('net_netmask').value;
            params += '&Gateway=' + $('net_gateway').value;
            params += '&DNS0=' + $('net_dns0').value;
        } else {
            params += '&IPWay=' + $('net_ip_auto').value;
        }
        
        params += '&Cmd=SetHttp.cgi';
        params += '&Port=' + $('net_port').value;
           
        manualRequest("Cmd.cgi", params, null);
                
        var newaddress = '';
        if($('net_ip_man').checked){
            newaddress = "http://" + $('net_ip').value;
        } else {
            newaddress = "http://" + location.hostname;
        }
        if($('net_port').value != 80) newaddress += ":" + $('net_port').value;

        $('reboot_text').innerHTML = "<b>Rovio is trying to reconnect to the network, please wait...<br />Page will attempt to refresh in <label id=\"reboot_countdown\">20</label> seconds.</b><br/>If the page does not automatically refresh, make sure your computer is on the same network as Rovio, and/or verify Rovio's network settings using the Rovio Setup software.";
        
        clearAllIntervals();
        
        openRebootDialog();
        
        reboot_countdown = 30;
        rebootCountdown(newaddress);
        reboot_interval_id = setInterval("rebootCountdown('" + newaddress + "')", 1000);
    } else {
        saveNetworkSettingsResponse(null);
    }
}
function updateShowOnlineStatus(){
    $('td_online_status_txt').className = show_online_status ? 'visible' : 'hidden';
    $('td_online_status').className = show_online_status ? 'visible' : 'hidden';
}
function confirmSaveNetworkSettings(){
    if(trim($('net_essid').value) == ''){ alert("Please enter the SSID."); return; }
    if($('net_ip_man').checked){
        if(trim($('net_ip').value) == ''){ alert("Please enter an IP address for Rovio."); return; }
        if(trim($('net_netmask').value) == ''){ alert("Please enter a subnet mask."); return; }
    }
    if($('net_mode').value != 'Ad-Hoc' && $('net_confirm').value != $('net_key').value){
        alert('The network keys you have entered do not match. Please re-enter them.');
        $('net_key').value = '';
        $('net_confirm').value = '';
        return;
    }
    if($('net_wifi_changed').checked || $('net_upnp_changed').checked){
        if(confirm('Are you sure you want to change your network settings?')){
            saveNetworkSettings();
        }
    } else {
        saveNetworkSettings();
    }
}
/* --- Video Functions --- */
function outputFrameRateOpts(){
    var s = '';
    for (var i = 1; i <= 30; i+=1)
        s += '<option value="'+i+'">'+i+' fps</option>';
    document.write(s);
}
function resetVideoSettings(){
    video_slider.setValue(Math.round(resolution * 3) + compression);

    if(isFirefox() || isSafari()){
        $('video_player').value = DEFAULT_OTHER_PLAYER;
    }

    $('video_res').selectedIndex = 2;
    $('video_fps').value = 30;
    $('video_quality').selectedIndex = 1;
    $('video_freq').value = 0;
    settings_brightness_slider.setValue(6);
    //brightness_slider.setValue(3);
    
    //$('video_force_mjpeg').checked = force_mjpeg;
}
function saveVideoSettingsResponse(t){
    $('video_save').disabled = true;
    alert("Video settings updated.");
}
function saveVideoSettings(){
    resolution = parseInt($('video_res').value);
    compression = parseInt($('video_quality').value);
    frame_rate = parseInt($('video_fps').value);
    brightness = parseInt(settings_brightness_slider.value);

    params =  'Cmd=ChangeResolution.cgi&ResType=' + resolution;
    params += '&Cmd=ChangeCompressRatio.cgi&Ratio=' + compression;
    params += '&Cmd=ChangeFramerate.cgi&Framerate=' + frame_rate;
    params += '&Cmd=ChangeBrightness.cgi&Brightness=' + brightness;
    brightness_slider.setValue(settings_brightness_slider.value);
    
    slider_only_val = true;
    video_slider.setValue(Math.round(resolution * 3) + compression);
    slider_only_val = false;
    
    force_mjpeg = $('video_force_mjpeg').checked ? 1 : 0;
    
    // save video player if in safari and preference has changed
    if(isSafari() || isFF()){
        v_player = $('video_player').value;
        setParameter(i_SVP, v_player);
        $('camera_container').innerHTML = '';
    }
    
    manualRequest('Cmd.cgi', params, saveVideoSettingsResponse);
    
    if($('video_freq').value){
        manualRequest('SetCamera.cgi', 'Frequency=' + $('video_freq').value, null);
    } else {
        auto_cam_freq = 1;
        manualRequest('SetCamera.cgi', 'Frequency=' + ac_freq, null);
    }
    setParameter(i_AVF, parseInt($('video_freq').value));
    
    setAfterUpdate();
}
function evoVersion(t){
    var result = parseVars2(t.responseText,"|","=");
    
    try {
        evo_version = trim(result[1][1]);
        if($('version').innerHTML != ''){
            $('version').innerHTML += ' | ';
        }
        $('version').innerHTML += 'TrueTrack: v' + evo_version;
    } catch(e) {
        evo_version = '';
    }
}
function getEvoVersion(){
    if(evo_version != '') return;

    var params = "Cmd=nav&action=25";
    manualRequest('rev.cgi', params, evoVersion);
    
    setTimeout("if(evo_version == '') getEvoVersion();", 5000);
}
function WBVersion(t){
    text = t.responseText;
    
    try {
        wb_version = text.substring(text.indexOf('$Revision: ')+11);
        wb_version = wb_version.substring(0,wb_version.indexOf('$'));
        
        if($('version').innerHTML != ''){
            $('version').innerHTML += ' | ';
        }
        $('version').innerHTML += 'Base Build: v' + wb_version;
    } catch(e) {
        wb_version = '';
    }
}
function getWBVersion(){
    if(wb_version != '') return;

    manualRequest('GetVer.cgi', null, WBVersion);
    
    setTimeout("if(wb_version == '') getWBVersion();", 5000);
}
/* --- System Functions --- */
function reboot(){
    sendCommand("Reboot.cgi", "");
}
function confirmReboot(){
    if(confirm("Are you sure you want to Reboot?")){
        initReboot();
    }
}
/*function restoreResponse(t){
    alert('Factory settings restored. You will need to use the setup software to reconfigure Rovio or connect using Ad hoc mode.');
}*/
function confirmRestore(){
    if(confirm("Are you sure you want to Restore Factory Defaults?")){
        manualRequest("SetFactoryDefault.cgi", "", null);
        alert('Factory settings restored. You will need to use the setup software to reconfigure Rovio or connect using Ad hoc mode.');
    }
}
/* --- Helper Functions --- */
function parseVars(varString){
    if(trim(varString) == "") return new Array();

    var varArr = varString.replace(/\n/g, " = ").split(" = ");
    var rtnObj = new Array();
    
    for(i = 0; i < varArr.length; i+=2){
        rtnObj[parseInt(i/2)] = new Object();
        rtnObj[parseInt(i/2)].key = varArr[i];
        rtnObj[i/2].value = varArr[i+1];
    }
    
    return rtnObj;
}
function parseVars2(str, sep, equ){
  var seperated = str.split(sep);
  var results = new Array();
  for(i = 0; i < seperated.length; i++){
    results[i] = seperated[i].split(equ);
  }
  return results;
}
function trim(string) {
	return string.replace(/^\s+|\s+$/g,"");
}
function resetMovementSettings(){
    speed_movement_slider.setValue(MAX_SPEED + 1 - DEFAULT_SPEED);
    speed_turn_slider.setValue(MAX_SPEED + 1 - DEFAULT_TURN_SPEED);
    speed_angle_slider.setValue(MAX_SPEED + 1 - DEFAULT_ROT_SPEED);
    $('move_ir').checked = 0;
}
function saveMovementSettingsResponse(t){
    $('move_update').disabled = true;
    alert('Movement settings updated.');
}
function setIR(val){
    IR_val = (val ? '1' : '0');
    params = 'Cmd=nav&action=19&IR=' + val;
    manualRequest('rev.cgi', params, null);
}
function updateMovementSpeeds(){
    movement_speed = MAX_SPEED + 1 - Math.round(speed_movement_slider.value);
    turn_speed = MAX_SPEED + 1 - Math.round(speed_turn_slider.value);
    rot_speed = MAX_SPEED + 1 - Math.round(speed_angle_slider.value);
}
function updateMovementSettings(){
    updateMovementSpeeds();
    
    IR_val = ($('move_ir').checked ? '1' : '0');
    params = 'Cmd=nav&action=19&IR=' + IR_val;
    
    setParameter(i_MS, speed_movement_slider.value);
    setParameter(i_TS, speed_turn_slider.value);
    setParameter(i_RS, speed_angle_slider.value);    
    manualRequest('rev.cgi', params, saveMovementSettingsResponse);
    
    setAfterUpdate();
}
function confirmFirmwareUpload(oForm){
    var img_file = oForm.SourceFile.value;
    
	if (img_file == "")
	{
		alert("You must select an image file to upload.");
		return false;
	}
	if (img_file.substring(img_file.length, img_file.length-4) != ".bin"){
	    alert("You must select an image file ending with '.bin'.");
		return false;
	}
	
	if(confirm("You are about to update the firmware on Rovio. This might restore Rovio to its default settings and you might need to use the Rovio Setup wizard to reconfigure it. Are you sure you want to do this?")){
	    $('reboot_text').innerHTML = "<b>Uploading firmware image, please wait...</b>";
	    openRebootDialog();
	    
	    return true;
	} else {
	    return false;
	}
}

function createXMLRequestObjs(){
    var t = 0;

    for(t = 0; t < MAX_REQUESTS; t++){
        xhr[t] = createXMLRequestObj();
        xhrRtn[t] = null;
    }
    
    for(t = 0; t < MAX_REQUESTS; t++){
        status_xhr[t] = createXMLRequestObj();
        status_xhrRtn[t] = null;
    }
}

function createXMLRequestObj(){
    var xmlReqObj = null;

    if (xmlReqObj == null){
	    try {xmlReqObj = new XMLHttpRequest();}
	    catch (e) {xmlReqObj = null;}
    }
    if (xmlReqObj == null){
	    try {xmlReqObj = new ActiveXObject("Msxml2.XMLHTTP");}
	    catch (e) {xmlReqObj = null;}
    }
    if (xmlReqObj == null){
	    try {xmlReqObj = new ActiveXObject("Microsoft.XMLHTTP");}
	    catch (e) {xmlReqObj = null;}
    }
    return xmlReqObj;
}

function manualRequest(url, params, onSuccess, method){
    if(method == null) method = 'POST';
    if(method == 'GET'){
        url += '?' + params;
        params = '';
    }

    // make sure that no threads require authentication
    var t;
    for(t = 0; t < MAX_REQUESTS; t++){
        if(xhr[t].readyState != 0){
            try {
                if(xhr[t].status == 401){
                    return;
                }
            } catch(e){
            }
        }
    }
    
    // find a free thread that's DONE
	var count = 0;
	do {
	    count++;
	    cur_xhr++;
	    if(cur_xhr >= MAX_REQUESTS) cur_xhr=0;
	    if(count > MAX_REQUESTS*2){
	        return;
        }
	} while(xhr[cur_xhr].readyState != 0 && xhr[cur_xhr].readyState != 4)
		
	var cur_index = cur_xhr;
	xhr[cur_index].open (method, url, true);
	
	if(onSuccess != null){
	    xhrRtn[cur_index] = onSuccess;
        // call return function once request returns successfully
	    xhr[cur_index].onreadystatechange = new Function("var index = " + cur_index + "; if (xhr[index].readyState == 4 && xhr[index].status == 200){ xhrRtn[index](xhr[index]); }");
	} else {
	    xhrRtn[cur_index] = null;
	    xhr[cur_index].onreadystatechange = function (){ }
	}
	
	try {
        xhr[cur_index].send(params);
    } catch(e) {
    }
}

function statusRequest(url, params, onSuccess){
    // make sure that no threads require authentication
    var t;
    for(t = 0; t < MAX_REQUESTS; t++){
        if(status_xhr[t].readyState != 0){
            try {
                if(status_xhr[t].status == 401){ 
                    return;
                }
            } catch(e){
            }
        }
    }
    
    // find a free thread that's DONE
	var count = 0;
	do {
	    count++;
	    cur_status_xhr++;
	    if(cur_status_xhr >= MAX_REQUESTS) cur_status_xhr=0;
	    if(count > MAX_REQUESTS*2){
	        return;
        }
	} while(status_xhr[cur_status_xhr].readyState != 0 && status_xhr[cur_status_xhr].readyState != 4)
	var cur_index = cur_status_xhr;
	status_xhr[cur_index].open ("POST", url, true);
	
	if(onSuccess != null){
	    status_xhrRtn[cur_index] = onSuccess;
        // call return function once request returns successfully
	    status_xhr[cur_index].onreadystatechange = new Function("var index = " + cur_index + "; if (status_xhr[index].readyState == 4 && status_xhr[index].status == 200){ status_xhrRtn[index](status_xhr[index]); }");
	} else {
	    status_xhrRtn[cur_index] = null;
	    status_xhr[cur_index].onreadystatechange = function (){ }
	}
	
	try {
        status_xhr[cur_index].send(params);
    } catch(e) {
    }
}

function bodyMouseUp(){
   changeToUnclickedImg($('rot_left'));
   changeToUnclickedImg($('rot_right'));
   changeToUnclickedImg($('move_forward'));
   changeToUnclickedImg($('move_left'));
   changeToUnclickedImg($('move_right'));
   changeToUnclickedImg($('move_back'));
   changeToUnclickedImg($('rotr_03'));
   changeToUnclickedImg($('rotr_07'));
   changeToUnclickedImg($('rotr_11'));
   changeToUnclickedImg($('rotr_15'));
   changeToUnclickedImg($('rotl_11'));
   changeToUnclickedImg($('rotl_07'));
   changeToUnclickedImg($('rotl_03'));
   changeToUnclickedImg($('delete_path'));
   changeToUnclickedImg($('reset'));
   changeToUnclickedImg($('stop'));
   changeToUnclickedImg($('home'));
   changeToUnclickedImg($('savehome'));
   stopMoving();
}

// --------- DYNDNS ------
var dns_enable = 0;
var dns_domain = '';
var dns_user = '';
var dns_pass = '';
var dns_service = '';
var dns_manualip = '';
var proxy_server = '';
var proxy_user = '';
var proxy_pass = '';
var proxy_port = '';


function loadDynDNSSettingsResponse(t){
    var result = parseVars(t.responseText);
    
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        switch(k){
            case 'Enable':
                dns_enable = parseInt(v);
                break;
            case 'DomainName':
                dns_domain = v;
                break;
            case 'User':
                dns_user = v;
                break;
            case 'Pass':
                dns_pass = v;
                break;
            case 'Proxy':
                proxy_server = v;
                break;
            case 'ProxyPort':
                proxy_port = v;
                break;
            case 'ProxyUser':
                proxy_user = v;
                break;
            case 'ProxyPass':
                proxy_pass = v;
                break;
            case 'IP':
                dns_manualip = v;
                break;
            case 'Service':
                dns_service = v;
                break;
        }
    }
    setDynDNSSettings();
    updateDynDomainFields();
    
    $('dyn_update').disabled = true;
}
function loadDynDNSSettings(){
    manualRequest("GetDDNS.cgi", '', loadDynDNSSettingsResponse);
}
function saveDynDNSSettingsResponse(){
    $('dyn_update').disabled = true;
    alert('Dynamic domain settings updated.');
    
    setTimeout("checkExternalAccess();",5000);
}
function saveDynDNSSettings(){
    if($('dyn_pass').value != $('dyn_confirm').value){
        alert('The two domain passwords are not the same.');
        return;
    }
    if($('dyn_proxypass').value != $('dyn_proxyconfirm').value){
        alert('The two proxy passwords are not the same.');
        return;
    }
    if($('dyn_service').value == ''){
        alert('You must select a service provider.');
        return;
    }

    dns_domain = $('dyn_domain').value;
    dns_user = $('dyn_user').value;
    dns_pass = $('dyn_pass').value;
    dns_service = $('dyn_service').value;
    
    dns_enable = $('dyn_enable').checked ? 1 : 0;
    if($('dyn_overwrite').checked){
        dns_manualip = $('dyn_manualip').value;
    } else {
        dns_manualip = '';
        $('dyn_manualip').value = '';
    }
    
    proxy_server = $('dyn_proxy').value;
    proxy_port = $('dyn_proxyport').value;
    proxy_user = $('dyn_proxyuser').value;
    proxy_pass = $('dyn_proxypass').value;

    var params = 'Enable=' + dns_enable;
    params += '&Service=' + dns_service;
    params += '&IP=' + chkStr(dns_manualip);
    params += '&User=' + chkStr(dns_user);
    params += '&Pass=' + chkStr(dns_pass);
    params += '&DomainName=' + chkStr(dns_domain);
    params += '&Proxy=' + chkStr(proxy_server);
    params += '&ProxyPort=' + proxy_port;
    params += '&ProxyUser=' + chkStr(proxy_user);
    params += '&ProxyPass=' + chkStr(proxy_pass);
   
    manualRequest("SetDDNS.cgi", params, saveDynDNSSettingsResponse);
}
function setDynDNSSettings(){
    $('dyn_domain').value = dns_domain;
    $('dyn_service').value = dns_service;
    $('dyn_user').value = dns_user;
    $('dyn_pass').value = dns_pass;
    $('dyn_confirm').value = dns_pass;
    $('dyn_enable').checked = dns_enable;
    $('dyn_manualip').value = dns_manualip;
    if(trim(dns_manualip) == '0.0.0.0' || trim(dns_manualip) == ''){
        $('dyn_overwrite').checked = 0;
        $('dyn_manualip').disabled = true;
    } else {
        $('dyn_overwrite').checked = 1;
        $('dyn_manualip').disabled = $('dyn_overwrite').disabled;
    }
    
    $('dyn_proxy').value = proxy_server;
    $('dyn_proxyport').value = proxy_port;
    $('dyn_proxyuser').value = proxy_user;
    $('dyn_proxypass').value = proxy_pass;
    $('dyn_proxyconfirm').value = proxy_pass;
}
function resetDynDNSSettings(){
    $('dyn_domain').value = '';
    $('dyn_service').selectedIndex = 0;
    $('dyn_user').value = '';
    $('dyn_pass').value = '';
    $('dyn_confirm').value = '';
    $('dyn_enable').checked = 0;
    $('dyn_manualip').value = '';
    $('dyn_overwrite').checked = 0;
    $('dyn_manualip').disabled = true;
    $('dyn_proxy').value = '';
    $('dyn_proxyport').value = 0;
    $('dyn_proxyuser').value = '';
    $('dyn_proxypass').value = '';
    $('dyn_proxyconfirm').value = '';
}
function checkDynDNSStatusResponse(t){
    var result = parseVars(t.responseText);
    var info = '';
    for(i = 0; i < result.length; i++){
        k = result[i].key;
        v = result[i].value;
        switch(k){
            case 'Info':
                info = trim(v);
                break;
        }
    }
    var text = "Unable to get the status.";
    switch(info.toLowerCase()){
        case 'updated':
            text = 'Domain’s IP address updated.';
            break;
        case 'updating failed':
            text = "Domain's IP address failed to update or failed to get IP address.";
            break;
        case 'updating':
            text = "Currently updating Domain's IP address.";
            break;
        case 'ip checked':
            text = "Successfully found IP and updated.";
            break;
        case 'not update':
            text = "Domain’s IP address not updated.";
            break;
    }           
    alert(text);
}
function checkDynDNSStatus(){
    var now = new Date();
    
    if(last_dns_check != -1 && now.getTime() - last_dns_check < 1000){
        return;
    }
    
    last_dns_check = now.getTime();
    
    manualRequest("GetDDNS.cgi", '', checkDynDNSStatusResponse);
}
function chkStr(str){
    str = str.replace(/%/g,"%25");
    str = str.replace(/&/g,"%26");
    str = str.replace(/\?/g,"%3F");
    str = str.replace(/\+/g,"%2B");
    str = str.replace(/=/g,"%3D");
    str = str.replace(/#/g,"%23");
    return str;
}
function setParameter(i,v){
    manualRequest("rev.cgi", 'Cmd=nav&action=23&index=' + i + '&value=' + parseInt(v), null);
}
function getAllParametersResponse(t){
    t = t.responseText;
    var results = parseVars2(t,"|","=");
    var ip1;
   
    for(i = 0; i < results.length; i++){
        k = results[i][0];
        v = parseInt(results[i][1]);
        if(v == null || v > 1000 || v < 0){
            // not set
            v = -1;
        }
        switch(k){
            case 'v0':
                maintainAspectRatio(v);
                break;
            case 'v' + i_MS:
                if(v > -1){
                    setMovementSpeed(v);
                }
                break;
            case 'v' + i_TS:
                if(v > -1){
                    setTurnSpeed(v);
                }
                break;
            case 'v' + i_RS:
                if(v > -1){
                    setRotSpeed(v);
                }
                break;
            case 'v' + i_LR:
                /*latency = v;
                lat_slider.setValue(v);*/
                break;
            case 'v' + i_SVP:
                if(isSafari() || isFF()) {
                    if(v != -1){
                        v_player = v;
                    } else {
                        v_player = DEFAULT_MOZ_PLAYER;
                    }
                    $('video_player').value = v_player;
                    resizeCamera();
                }
                break;
            case 'v' + i_UPnP:
                if(v == -1){
                    upnp_just_enabled = true;
                    setParameter(i_UPnP,1);
                } else {
                    upnp_just_enabled = v;
                }
                break;
            // show online status
            case 'v' + i_SS:
                if(v == -1){
                    show_online_status = 1;
                    setParameter(i_SS,1);
                } else {
                    show_online_status = v ? 1 : 0;
                }
                $('net_include_status').checked = show_online_status;
                updateShowOnlineStatus();
                break;
            case 'v' + i_MIIP1:
                if(parseInt(results[i][1]) < 0){
                    manual_internetip = '';
                } else {
                    ip1 = parseInt(results[i][1]);
                }
                break;
            case 'v' + i_MIIP2:
                if(parseInt(results[i][1]) < 0){
                    manual_internetip = '';
                } else {
                    var ip2 = parseInt(results[i][1]);
                    var oct2 = parseInt(ip1 / 256);
                    var oct1 = parseInt(ip1 - (oct2 * 256));
                    var oct4 = parseInt(ip2 / 256);
                    var oct3 = parseInt(ip2 - (oct4 * 256));
                    manual_internetip = oct1 + '.' + oct2 + '.' + oct3 + '.' + oct4;
                }
                if(!upnp_enabled){
                    upnp_ip = manual_internetip;
                    updateExternalStatus();
                }
                $('net_internetip').value = manual_internetip;
                break;
            case 'v' + i_VIA:
                if(v == -1){
                    verify_internet_access = 1;
                    setParameter(i_VIA,1);
                } else {
                    verify_internet_access = v ? 1 : 0;
                }
                $('net_verify_access').checked = verify_internet_access;
                break;
            case 'v' + i_NFA:
                if(v == -1){
                    firmware_alert = 1;
                    setParameter(i_NFA,1);
                } else {
                    firmware_alert = v ? 1 : 0;
                }
                $('firmware_alert_box').checked = firmware_alert;
                break;
            case 'v' + i_AVF:
                if(v != 0 && v != 50 && v != 60){
                    v = 0;
                }
                if(v == 0){
                    auto_cam_freq = 1;    
                } else {
                    auto_cam_freq = 0;    
                }
                $('video_freq').value = v;
                break;
        }
    }
    
    $('move_update').disabled = true;
}
function getAllParameters(){
    manualRequest("rev.cgi", 'Cmd=nav&action=24', getAllParametersResponse);
}
function getParameter(i,v){
    manualRequest("rev.cgi", 'Cmd=nav&action=24&index=' + i, getParameterResponse);
}
/* --- Select network box --- */
function selectNetwork(id){
    if(selected_network != -1) $(selected_network).className = 'unselectedNetwork';
    $(id).className = 'selectedNetwork';
    selected_network = $(id).id;
}
function getSelectedNetworkName(){
    return $(selected_network).id.substring(4);
}
function closeSelectNetworkBox(){
    $('selectedNetworkBox').style.display = 'none';
    $('settings_dialog').style.zIndex = "3000";
    enableSelection();
}
function openSelectNetworkBox(){
    $('selectedNetworkBox').style.display = 'block';
    $('settings_dialog').style.zIndex = "2000";
    $('networks_selectNetwork').style.display = 'block';
    $('networks_password').style.display = 'none';
    refreshNetworks();
    disableSelection();
}
function processNetworks(t){
    clearNetworks();

    t = t.responseText;
    //t = "ESSID = JS_AP\nESSID = JS_AP\nMode = Managed\nMode = Ad-Hoc\nEncode = 1\nEncode = 1\nQuality = 59\nQuality = 50\n";
    var results = parseVars(t);
    var i;
    var m = 0;
    var e = 0;
    var q = 0;
    var networks = new Array();
    for(i = 0; i < results.length; i++){
        switch(results[i].key){
            case 'ESSID':
                networks[i] = new Object();
                networks[i].name = results[i].value;
                break;
            case 'Mode':
                networks[m].mode = results[i].value;
                m++;
                break;
            case 'Encode':
                networks[e].encode = parseInt(results[i].value);
                e++;
                break;
            case 'Quality':
                networks[q].quality = parseInt(results[i].value);
                q++;
                break;
        }
    }
    for(i = 0; i < networks.length; i++){
        addNetwork(networks[i].name, networks[i].mode, networks[i].encode, networks[i].quality);
    }
}
function refreshNetworks(){
    clearNetworks();
    
    var tbl = document.getElementById('networks_table');
    var lastRow = tbl.rows.length;
    var row = tbl.insertRow(0);
    var cell = row.insertCell(0);
    cell.innerHTML = 'Searching for wireless networks in range...';
    cell.className = 'networksearching';
    
    // try to find networks
    manualRequest('ScanWlan.cgi',null,processNetworks);
    //processNetworks('adsf');
}
function okNetwork(){
    // if requires password pop up password box otherwise populate settings panel
    if($('net_' + getSelectedNetworkName() + '_security') != null && $('networks_selectNetwork').style.display == 'block'){
        enableSelection();
        $('networks_password').style.display = 'block';    
        $('networks_selectNetwork').style.display = 'none';
        $('netbox_password').value = '';
        $('netbox_confirm').value = '';
        $('passwordbox_netid').innerHTML = getSelectedNetworkName();
    } else {
        // set the password
        if($('networks_password').style.display == 'block'){
            if($('netbox_password').value != $('netbox_confirm').value){
                alert('The network keys you have entered do not match. Please re-enter them.');
                return;
            }
            $('net_key').value = $('netbox_password').value;
            $('net_confirm').value = $('netbox_password').value;
        } else {
            $('net_key').value = '';
            $('net_confirm').value = '';
        }
        $('net_key_changed').checked = 1;
        $('net_essid').value = getSelectedNetworkName();
        if($('net_' + getSelectedNetworkName() + '_adhoc') != null){
            $('net_mode').value = 'Ad-Hoc';
            $('net_key').disabled = true;
            $('net_confirm').disabled = true; 
        } else {
            $('net_mode').value = 'Managed';
            $('net_key').disabled = false;
            $('net_confirm').disabled = false; 
        }
        closeSelectNetworkBox();
        $('net_save').disabled = false;
        $('net_wifi_changed').checked = true;
    }
}
function cancelNetwork(){
    if($('networks_selectNetwork').style.display == 'block'){
        closeSelectNetworkBox();    
    } else {
        disableSelection();
        $('networks_password').style.display = 'none';    
        $('networks_selectNetwork').style.display = 'block';   
    }
}
function addNetwork(name,mode,secure,strength){
    var ss = 4;
    if(strength > 80){ // 54
        ss = 0;
    } else if(strength > 60) { // 49
        ss = 1;
    } else if(strength > 40) { // 44
        ss = 2;
    } else if(strength > 20) { // 44
        ss = 3;
    }
    
    // don't add duplicates just update wifi strength
    if($('net_' + name) != null){
        oldSS = $('net_' + name + '_ss').alt;
        if(ss < oldSS){
            $('net_' + name + '_ss').src = 'img/wifi/signal' + ss + '.gif';
            $('net_' + name + '_ss').alt = ss;
        }
        return;
    }
    
    var description = '';
    if(secure){
        description = '<img src="img/wifi/lock.gif" id="net_' + name + '_security" /> Security-enabled ';
    } else {
        description = 'Unsecure ';
    }
    var networkicon = '';
    if(mode == 'Managed'){
        description += 'wireless network';
        networkicon = '<img id="net_' + name + '_managed" src="img/wifi/wifisignal.gif" />';
    } else {
        description += 'computer-to-computer network';
        networkicon = '<img id="net_' + name + '_adhoc" src="img/wifi/adhoc.gif" />';
    }
            
    var tbl = document.getElementById('networks_table');
    var lastRow = tbl.rows.length;
    var row = tbl.insertRow(lastRow);
    row.className = "unselectedNetwork";
    row.id = 'net_' + name;
    row.onclick = new Function("selectNetwork(this.id)");
    row.ondblclick = new Function("selectNetwork(this.id); okNetwork()");
    var neticon = row.insertCell(0);
    neticon.innerHTML = networkicon;
    var networkname_cell = row.insertCell(1);
    networkname_cell.className = 'networktext';
    networkname_cell.innerHTML = '<label class="networkname">' + name + '</label><br/><label class="networkdesc">' + description + '</label>';
    var ss_cell = row.insertCell(2);
    ss_cell.className = 'networkstatus';
    if(essid == name){
        ss_cell.innerHTML = '<strong style="font-size: 8pt;">Connected</strong><br/>';
    }
    ss_cell.innerHTML += '<img id="net_' + name + '_ss" alt="' + ss + '" src="img/wifi/signal' + ss + '.gif" />';
}
function clearNetworks(){
    var tbl = document.getElementById('networks_table');
    var numNetworks = tbl.rows.length;
    var i;
    for(i = numNetworks-1; i >= 0; i--){
        tbl.deleteRow(i);
    }
}
/* --- RTSP Auth box --- */
function closeRTSPAuthBox(){
    disableSelection();
    
    key_events = true;
    settings_dialog_open = false;
    
    $('disable_controls').style.display = 'none';
    $('rtspAuthBox').style.display = 'none';
}
function okRTSPAuth(){
    rtspAuth = 1;
    rtspAuthUser = $('rtsp_user').value;
    rtspAuthPass = $('rtsp_pass').value;
    if($('rtsp_save').checked){
        createCookie(COOKIE_RTSP_USER, $('rtsp_user').value, 365);
        createCookie(COOKIE_RTSP_PASS, $('rtsp_pass').value, 365);
    }
    
    closeRTSPAuthBox();
    
    cam_initialized = false;
    resizeCamera();
}
function cancelRTSPAuth(){
    rtspAuth = 0;
    closeRTSPAuthBox();
}
function openRTSPAuthBox(){
    $('camera_container').innerHTML = '';
    $('rtspAuthBox').style.display = 'block';
    
    enableSelection();
    key_events = false;
    settings_dialog_open = true;
    
    $('disable_controls').style.display = 'block';
}

function onVideoPlayerChanged(){
    if($('video_player').value == 3){
        if(!checkJavaIsInstalledOnNav() && !checkVLCIsInstalledOnNav()){
            showHelp('java_vlc_not_installed_help');
        } else if(!checkJavaIsInstalledOnNav()){
            showHelp('java_not_installed_help');
        } else if(!checkVLCIsInstalledOnNav()){
            showHelp('vlc_not_installed_help');
        }
    }
}

/* --- Browser functions --- */
var javaInstalled = -1;
function checkJavaIsInstalledOnNav(){
    if(isIE()) return 0;
    if(javaInstalled != -1) return javaInstalled;
    for (i=0; i < navigator.plugins.length; i++){
        for (j = 0; j < navigator.plugins[i].length; j++){
            if(navigator.plugins[i][j].type == "application/x-java-applet;version=1.3"){
                javaInstalled = 1;
                return 1;
            }
        }
    }
    javaInstalled = 0;
    return 0;
}
var VLCInstalled = -1;
function checkVLCIsInstalledOnNav(){
    if(isIE()) return 0;
    if(VLCInstalled != -1) return VLCInstalled;
    for (i=0; i < navigator.plugins.length; i++){
        for (j = 0; j < navigator.plugins[i].length; j++){
            if(navigator.plugins[i][j].type == "application/x-vlc-plugin"){
                VLCInstalled = 1;
                return 1;
            }
        }
    }
    VLCInstalled = 0;
    return 0;
}
function isSafari(){
    return (navigator.userAgent.indexOf('Safari') != -1);
}
function isIE(){
    return (navigator.appName == "Microsoft Internet Explorer");
}
function getIEVer() {
    var ua = navigator.userAgent;
    var MSIEOffset = ua.indexOf("MSIE ");
    
    if (MSIEOffset == -1) {
        return 0;
    } else {
        return parseFloat(ua.substring(MSIEOffset + 5, ua.indexOf(";", MSIEOffset)));
    }
}
function isIEpre7(){
    return (isIE() && getIEVer() < 7);
}
function isFF(){
    return (navigator.userAgent.indexOf('Firefox') != -1);
}
function isQuicktimeInstalled(){
    var i;
    for (i = 0; i < navigator.plugins.length; i++){
        if(navigator.plugins[i].name.indexOf('QuickTime Plug-in') != -1) return true;
    }
    return false;
}
function ip2int(ip){
    if (ip == "") {
        return -1;
    } else {
        var ips = ip.split(".");
        return parseInt(ips[3]) + (ips[2] * 256) + (ips[1] * 256 * 256) + (ips[0] * 256 * 256 * 256);
    }
}
function int2ip(number){
    var oct1 = parseInt(number / (16777216));
    var oct2 = parseInt((number - (oct1 * 16777216)) / (256 * 256));
    var oct3 = parseInt((number - (oct1 * 16777216) - (oct2 * 65536)) / 256);
    var oct4 = parseInt(number - (oct1 * 16777216) - (oct2 * 65536) - (oct3 * 256));
    
    if(oct1 > 255 || oct2 > 255 || oct3 > 255 || oct4 > 255){
        return '';
    } else {
        return oct1 + "." + oct2 + "." + oct3 + "." + oct4;
    }
}
function saveIP(ip){
    var ips = ip.split(".");
    setParameter(i_MIIP1, parseInt(ips[0]) + (ips[1] * 256));
    setParameter(i_MIIP2, parseInt(ips[2]) + (ips[3] * 256));
}

function createCookie(name,value,days) {
	if (days) {
		var date = new Date();
		date.setTime(date.getTime()+(days*24*60*60*1000));
		var expires = "; expires="+date.toGMTString();
	}
	else var expires = "";
	document.cookie = name+"="+value+expires+"; path=/";
}

function readCookie(name) {
	var nameEQ = name + "=";
	var ca = document.cookie.split(';');
	for(var i=0;i < ca.length;i++) {
		var c = ca[i];
		while (c.charAt(0)==' ') c = c.substring(1,c.length);
		if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length,c.length);
	}
	return null;
}

function eraseCookie(name) {
	createCookie(name,"",-1);
}
