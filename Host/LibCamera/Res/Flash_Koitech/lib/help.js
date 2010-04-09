/* Copyright (C) 2008 WowWee Group Ltd. */
/* Author: Josh Savage */

var new_version_msg = '';

function showHelp(id){
    var heading = '';
    var text = '';
    var top = '-560px';
    var left = '320px';

    switch(id){
        case 'java_applet_fail':
            heading = 'Unable to start Java audio plugin';
            text = 'To send audio from your PC microphone to Rovio in browsers other than Internet Explorer you will need to download and install the latest version of <a href="http://www.java.com" target="_blank">Java</a>.<br/><input type="checkbox" onclick="if(this.checked) { createCookie(COOKIE_JAVA_WARNING, 1, 365); } else { eraseCookie(COOKIE_JAVA_WARNING); }" /> Don\'t show me this message again';
            break;
        case 'vlc_fail':
            heading = 'Unable to start VLC plugin';
            text += '<p>To receive audio from Rovio in Firefox and Safari browsers you need to download and install VLC Player from <a href="http://www.videolan.org/vlc/" target="_blank">here</a>. When installing make sure to check the Mozilla plugin from the optional components and your browser is closed.</p><p>This interface will now switch to use the MJPEG player which will have no sound.';
            break;
        case 'upnp_connecting_help':
            heading = 'Connecting to router to open ports';
            text = 'Rovio is connecting to your router to open ports in order for you to be able to access it over the internet. Depending on your router, this might take a few minutes.';
            break;
        case 'all_verifying_help':
            heading = 'Verifying internet access';
            text = 'Rovio is using the WowWee server to verify access over the internet to your Rovio.';
            break;
        case 'upnp_failed_help':
            heading = 'Failed to open UPnP ports';
            text = 'Verify that you have enabled UPnP on your router and it is connected to the internet. If UPnP has been enabled on your router and the ports still can\'t be opened, try changing the start port in the ' + returnSettingsLink('network settings tab',5) + ' or opening the ports manually.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'portal_success':
            heading = 'Rovio is connected to the portal';
            text = 'To access your Rovio outside your local network log in to the WowWee Portal at ' + external_address;
            break;
        case 'portal_fail':
            heading = 'Unable to connect Rovio to the Portal';
            text = 'Check that your Rovio is connected to a WiFi router that is connected to the internet.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'upnp_noip_help':
            heading = 'Unable to acquire external IP address';
            text = 'Rovio was unable to acquire an external IP address from your router. Please verify that your router is connected to the internet.';
            text += ' If your router is behind a firewall or another router, you might need to use a dynamic domain to verify access over the internet.'
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'upnp_domain_help':
            heading = 'Using a domain to access your Rovio';
            text = 'If you would like to access your Rovio using a domain instead of an IP address, you need to set up a dynamic domain. ' + returnSettingsLink('Click here',6) + ' to open the dynamic domain settings tab.';
            break;
        case 'unable_to_getip_help':
            heading = 'Unable to acquire external IP address';
            text = 'Rovio was unable to acquire your external IP address. Please verify that your Rovio is connected to a router that has an internet connection.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'verify_ip_help':
            heading = 'Unable to verify external access to your Rovio';
            text = 'Rovio was unable to verify that it can be accessed over the internet on the following IP address <strong>' + upnp_ip + '</strong>. First, ensure your router is connected to the internet and working. Try selecting another start port in the ' + returnSettingsLink('network settings tab',5) + ' as some ports might be blocked by your internet service provider.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'verify_domain_help':
            heading = 'Unable to verify internet access using your domain';
            var address_ip = upnp_ip + ':' + upnp_http_port;
            var address_ip_link = '<a href="http://' + address_ip + '">http://' + address_ip + '</a>';
            
            if(external_ip_status == 0){
                text = 'Rovio was unable to verify that it can be accessed over the internet using the domain <strong>' + dns_domain + '</strong>. However, you can access your Rovio using ' + address_ip_link + '. Make sure your dynamic domain settings are correct at your domain provider.';
            } else {
                text = 'Rovio was unable to verify that it can be accessed over the internet using the domain <strong>' + dns_domain + '</strong>. Make sure your dynamic domain settings are correct at your domain provider.';
            }
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'verify_all_help':
            heading = 'Unable to verify internet access to your Rovio';
            text = 'Rovio was unable to verify that it can be accessed over the internet. First, ensure your router is connected to the internet and working. Try selecting another start port as some ports might be blocked by your internet service provider.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'online_ok_help':
            heading = 'External access verified';
            text = 'Your Rovio can be accessed over the internet using ' + external_address + '.';
            break;
        case 'internet_ip_help':
            heading = 'External IP address';
            text = 'If you are not automatically opening ports using UPnP and are not using a dynamic domain you will need to provide an external IP address for Rovio to verify internet access. You can use the \'Try to get external IP\' button which will attempt to acquire your external IP address using WowWee\'s server. Unless your internet service provider is providing you with a static external IP address, it might change which means you will need to use a ' + returnSettingsLink('dynamic domain',6) + '.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'upnp_failed_popup':
            heading = 'Failed to open UPnP ports';
            text = 'Rovio was unable to open any ports on your router. Please verify that you have enabled UPnP on your router and it is connected to the internet.';
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'upnp_openned_noip_popup':
            heading = 'Unable to acquire external IP address';
            text = 'Rovio successfully opened port <strong>' + upnp_http_port + '</strong> on your router however it was unable to acquire an external IP address. Please verify that your router is connected to the internet.'
            text += '<br/><br/>For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'upnp_disabled_help':
            heading = 'UPnP disabled';
            text = 'Rovio can only verify access over the internet if UPnP is enabled. Enable UPnP in the Network tab under Settings.';
            break;
        case 'video_settings_help':
            heading = 'Video Settings';
            text = 'This tab allows you to change Rovio\'s streaming video settings.';
            text += '<p><strong>Resolution</strong> adjusts the size of the video image in pixels.</p>';
            text += '<p><strong>Quality</strong> adjusts the quality of the video image.</p>';
            text += '<p><strong>Frame rate</strong> adjusts the frequency of video images displayed per second.</p>';
            text += '<p><strong>Brightness</strong> adjusts brightness of the video image.</p>';
            break;
        case 'movement_settings_help':
            heading = 'Movement Settings';
            text = 'This tab allows you to change Rovio\'s movement speeds and toggle Rovio\'s obstacle detection.';
            text += '<p><strong>Movement speed</strong> adjusts the speed of Rovio\'s movements.</p>';
            text += '<p><strong>Turn speed</strong> adjusts the speed of Rovio\'s turning movements.</p>';
            text += '<p><strong>Angle turn speed</strong> adjusts the speed of Rovio\'s turning angle.</p>';
            text += '<p><strong>IR Radar</strong> toggles Rovio\'s obstacle detection feature. When enabled, Rovio\'s viewing area lights up light blue on the joystick pad and Rovio will try to avoid obstacles while navigating home or playing back a path.</p>';
            break;
        case 'photo_settings_help':
            heading = 'Photo Settings';
            text = 'This tab allows you to specify settings for emailing photo screen captures of the video stream from Rovio\'s interface. Note that Rovio does not currently support SMTP servers that use encryption (e.g. SSL or TLS).';
            text += '<p class="help_section"><strong>SMTP Settings</strong></p>';
            text += '<p><strong>Enable email photo</strong> toggles the screen capture emailing function. Enable the option to allow the screen capture emailing function.</p>';
            text += '<p><strong>SMTP server</strong> specifies the address of the SMTP server. Refer to your email client settings for reference.</p>';
            text += '<p><strong>SMTP port</strong> specifies the port to use for your SMTP server. Default is 25.</p>';
            text += '<p><strong>Username</strong> specifies the username for your email server, if required.</p>';
            text += '<p><strong>Password</strong> specifies the password for your email server, if required.</p>';
            text += '<p><strong>Confirm password</strong> specifies the same password as above, if required.</p>';
            text += '<p class="help_section"><strong>Email Settings</strong></p>';
            text += '<p><strong>Sender email address</strong> specifies the email address from which to send the screen capture.</p>';
            text += '<p><strong>Receiver email address</strong> specifies the destination email address to send the screen capture to.</p>';
            text += '<p><strong>Email subject</strong> specifies the subject line for your screen capture email.</p>';
            text += '<p><strong>Email body</strong> specifies the text that appears in the body of the mail.</p>';
            break;
        case 'security_settings_help':
            heading = 'Security Settings';
            text = 'This tab allows you to toggle user authentication and manage user accounts. When you enable user authentication, users will be required to log in before they can access your Rovio.';
            text += '<p><strong>Enable user authentication</strong> enables and disables user authentication. NOTE: You must click  on the Update button to save the setting.</p>';
            break;
        case 'user_accounts_help':
            heading = 'User Accounts';
            text = 'The fields described below will appear when you click the Add New User button or Edit button.';
            text += '<p><strong>Username</strong> specifies the name to use when logging into the Rovio for that user.</p>';
            text += '<p><strong>Password</strong> specifies the password to use when logging into the Rovio for that user. The password must be at least 6 characters long for security reasons.</p>';
            text += '<p><strong>Confirm password</strong> confirms the above password.</p>';
            text += '<p><strong>Admin access</strong>, when enabled, allows the user to access administrator privileges such as the Settings menu and recording paths.</p>';
            break;
        case 'wifi_settings_help':
            heading = 'Wi-Fi Settings';
            text = 'This tab allows you to change Rovio\'s wireless settings.';
            text += '<p><strong>SSID</strong> specifies the name of the wireless router you will use with Rovio. In Ad hoc mode, it specifies the name of the Ad hoc network which you use to directly connect to Rovio. Clicking on the Select Network button displays a list of wireless routers that Rovio detects.';
            text += '<p><strong>MAC address</strong> displays Rovio\'s MAC address. This can not be changed.</p>';
            text += '<p><strong>Mode</strong> specifies the mode of networking. <i>Infrastructure</i> is used to connect Rovio to a wireless router and <i>Ad hoc</i> is used to connect directly to Rovio without using a wireless router.</p>';
            text += '<p><strong>Key</strong> specifies the encription key used for your router, if required. Refer to your router settings for reference.</p>';
            text += '<p><strong>Web port</strong> specifies the port on which to send HTTP data. If you change the default port from 80, you must add it at the end of the browser address. For example, if the web port is set to 8080, you need to enter http://192.168.10.18:8080 in your browser.</p>';
            break;
        case 'ip_setup_help':
            heading = 'IP Setup';
            text = 'These are the settings Rovio uses to connect to your local network.';
            text += '<p><strong>Automatically from DHCP</strong> selects an IP address based on the router\'s DHCP settings. If you choose this option, you might need to use the Rovio Finder utility (installed when you installed the Rovio Setup software) to acquire Rovio\'s IP address.';
            text += '<p><strong>Manually</strong> allows you to specify static IP settings for your Rovio.';
            text += '<p><i>The following fields are only enabled when you select the Manually option.</i></p>';
            text += '<p><strong>Rovio IP address</strong> specifies Rovio\'s IP address.</p>';
            text += '<p><strong>Subnet mask</strong> a range of addresses to use with Rovio. Use 255.255.255.0 if you do not have a specific subnet mask.</p>';
            text += '<p><strong>Default gateway</strong> is the IP address of your gateway to the internet. The default gateway address should be your router\'s IP address. </p>';
            text += '<p><strong>DNS</strong> specifies the primary DNS server. The DNS server is used to resolve domain names to IP addresses. Refer to your router\'s settings for reference.</p>';
            break;
        case 'internet_access_help':
            heading = 'Internet Access';
            text = 'To access your Rovio over the internet you need to do the following:<br/>';
            text += '<ol><li>Connect Rovio to a wireless access point (WAP) with internet access.</li>';
            text += '<li>Forward 2 ports on your internet router to ports 80 and 554 on your Rovio (if UPnP is enabled on your router this can be done automatically)</li>';
            text += '<li>Use a static external IP address (provided by your ISP) or a dynamic domain (see ' + returnSettingsLink('Dynamic Domain Settings', 6) + ')</li>';
            text += '</ol>';
            text += '<p><strong>Automatically open ports using UPnP</strong>, when selected, Rovio will attempt to open ports on your router to allow you to access your Rovio over the internet. Rovio opens 3 ports the start port for HTTP traffic, the start port plus one for TCP RTSP traffic (ActiveX support), and the start port plus 2 for UDP RTSP traffic (QuickTime support).</p>';
            text += '<p><strong>Manually open ports</strong> allows you to specify manual settings that you have configured on your router.</p>';
            text += '<p><strong>Start port</strong> specifies the port to try to open first on your router. If you are manually opening ports, this is the port you have pointed to port 80 on Rovio.</p>';
            text += '<p><strong>External IP</strong> specifies the IP address that your router is connected to the internet. For more information, click the contextual help button.</p>';
            text += '<p><strong>Verify internet access</strong>, when enabled, Rovio will attempt to verify that you can access it over the internet (in addition to within your local area network).</p>';
            text += '<p><strong>Online status</strong> indicates whether you can access your Rovio over the internet.</p>';
            text += '<p><strong>External address</strong> displays the address to use if you want to access Rovio over the internet. This might not work from within the local area network (LAN) your Rovio is currently on.</p>';
            text += 'For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'dynamic_domain_settings_help':
            heading = 'Dynamic Domain Settings';
            text = 'The Domain tab allows you to setup a domain to access your Rovio from internet.';
            text += '<p><strong>Enable</strong> field toggles whether you to want to use a dynamic domain name system to access Rovio. You need to have a domain name registered to use this option.</p>';
            text += '<p><strong>Service provider</strong> specifies the dynamic domain service provider with whom you have registered a domain name.</p>';
            text += '<p><strong>Domain</strong> specifies the domain name you want to use for your Rovio. For example, dansmith.myrovio.com.</p>';
            text += '<p><strong>Username</strong> specifies the username for your account.</p>';
            text += '<p><strong>Password</strong> specifies the password for your account.</p>';
            text += '<p><strong>Overwrite IP</strong> allows you to overwrite and specify the IP address that is sent to your dynamic DNS server. In most cases you do not need to overwrite this.</p>';
            text += 'For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'proxy_settings_help':
            heading = 'Proxy Settings (optional)';
            text = 'Allows you to specify settings for a proxy server, if you use one.';
            text += '<p><strong>Server</strong> specifies proxy server IP address or Domain.</p>';
            text += '<p><strong>Port</strong> specifies proxy server port.</p>';
            text += '<p><strong>Username</strong> specifies proxy server username.</p>';
            text += '<p><strong>Port</strong> specifies proxy server port.</p>';
            text += '<p><strong>Password</strong> specifies proxy server password.</p>';
            text += 'For more information, visit ' + returnSupportLink('WowWee Support') + '.';
            break;
        case 'dyndomain_status_help':
            heading = 'Check Status';
            text = 'The Check Status button displays a message with the current status of your dynamic domain.';
            break;
        case 'firmware_version_help':
            heading = 'Firmware Version';
            text = 'Displays the firmware version currently loaded on Rovio.';
            text += '<p>The \'Alert me when new firmware is available\' option, when enabled, alerts you when new firmware for Rovio becomes available from ' + returnSupportLink('WowWee Support') + '.</p>';
            break;
        case 'reboot_system_help':
            heading = 'Reboot Rovio';
            text = 'Reboots Rovio. Ensure that Rovio is not connected via USB when you reboot.';
            break;
        case 'restore_help':
            heading = 'Restore Default Settings';
            text = 'Restores all settings on Rovio to the default. After you restore the default settings, Rovio will be reset to Ad hoc mode and its IP address will be changed to 192.168.10.18. You will need to reconnect using these network settings.';
            break;
        case 'upload_firmware_help':
            heading = 'Update Firmware Image';
            text = 'Allows you to update the Rovio firmware.';
            text += '<p>To update Rovio\'s firmware:';
            text += '<ol><li>Download the latest firmware from the ' + returnSupportLink('WowWee Support site') + ' on to your computer</li>';
            text += '<li>Locate the downloaded firmware image using the Browse button (*.bin) and then click on the Update button. It might take a up to a minute to upload the new image depending on your wireless connection.</li></ol></p>';
            text += 'IMPORTANT: Do not turn off Rovio and do not disconnect it from your computer until it has finished updating. The update will not take effect until you reboot Rovio. Ensure you have a full battery so that it does not lose power during the update. It is recommended that you do not update the firmware the internet.';
            break;
        case 'verify_internet_access_disabled':
            heading = 'Verify internet access disabled';
            text = 'Rovio is not currently verifying that it can be accessed over the internet. You can enable this feature in the ' + returnSettingsLink('network settings tab', 5) + '.';
            break;
        case 'no_ip_or_domain':
            heading = 'No IP or Domain set up';
            text = 'To verify whether or not your Rovio can be accessed over the internet, enter an external IP address in the ' + returnSettingsLink('network settings tab', 5) + ' or set up a dynamic domain in the ' + returnSettingsLink('domain tab', 6) + '.';
            break;
        case 'quicktime_over_internet_help':
            heading = 'QuickTime over the internet';
            text = 'Depending on your router type, QuickTime might not be able to obtain video and/or audio feed over the internet. If you experience problems, revert back to Motion JPEG.';
            break;
        case 'new_version_msg_popup':
            heading = 'New Firmware Version Available';
            text = new_version_msg;
            text += '<p>You can disable new firmware alerts under settings in the ' + returnSettingsLink('system tab', 7) + '.</p>';
            break;  
        case 'getting_settings':
            heading = 'Getting Settings';
            text += '<p>Please wait while Rovio gets its settings.</p>';
            break; 
        case 'java_vlc_help':
        case 'java_vlc_not_installed_help':
            heading = 'VLC Player and Java not installed';
            text += '<p>For installation instructions and known issues <a href="http://www.wowweesupport.com/download/rovio/rovio_audio.htm" target="_blank" class="url">click here</a>.</p>';
            break;
        case 'java_not_installed_help':
            heading = 'Java is not installed properly';
            text += '<p>For installation instructions and known issues <a href="http://www.wowweesupport.com/download/rovio/rovio_audio.htm" target="_blank" class="url">click here</a>.</p>';
            break;
        case 'vlc_not_installed_help':
            heading = 'VLC Player is not installed properly';
            text += '<p>For installation instructions and known issues <a href="http://www.wowweesupport.com/download/rovio/rovio_audio.htm" target="_blank" class="url">click here</a>.</p>';
            break;
    }
    
    if(text != ''){
        prevent_resize = true;
        help_dialog_open = true;
        
        if(cam_initialized){
            if(v_player != 3){
                $('camera_container').innerHTML = '';
            } else {
                stopVLC();
                stopAudioApplet();
            }
        }
        
        $('disable_controls').style.display = 'block';
        
        $('settingshelp').style.display = 'block';
        $('settingshelp_heading').innerHTML = heading;
        $('settingshelp_text').innerHTML = text;
        $('settingshelp').style.top = top;
        $('settingshelp').style.left = left;
        $('settings_dialog').style.zIndex = "2000";
    }
}
function hideHelp(){
    help_dialog_open = false;

    $('settings_dialog').style.zIndex = "3000";
    $('settingshelp').style.display = 'none';
    
    // if settings dialog is not open we need to re-establish the video
    if(!settings_dialog_open){
        prevent_resize = false;
        // this will enable controls again and start the video/audio
        closeSettingsDialog();
    }
}
function returnHelpBtn(id){
    return '<img id="' + id + '" onclick="showHelp(this.id)" class="help_btn" src="img/btns/help.gif" onmouseover="this.src=\'img/btns/help_over.gif\';" onmouseout="this.src=\'img/btns/help.gif\';" />'
}
function returnSettingsLink(text, tab){
    return '<a onclick="hideHelp(); selectTab($(\'settingtab_' + tab + '\')); openSettingsDialog(); return false;" class="url">' + text + '</a>';
}
function returnSupportLink(text){
    return '<a href="http://www.wowweesupport.com/product_item.php?cat=tech_tab&item=rovio" target="_blank">' + text + '</a>';
}