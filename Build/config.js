this.config = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "IPCam config";
	}

	this.set_default_ip = function( ip )
	{
	}

	this.configs_all =
	[
		{
			name: "[W99802] IP_CAM+SPI+(OV7670,PO6030K,...)",
			default_essid: "default",
			default_wifi_mode: "infrastructure",/* "ad-hoc" or "Infrastructure" */
			default_mac: "00:01:03:20:70:0a",
			default_ip: "192.168.10.18",
			default_netmask: "255.255.255.0",
			default_gateway: "192.168.10.1",
			default_dhcp: true,
			sensor: "SENSOR_OV7670",
			audio: "WM8978_NO_GPIO",
			memory_size: 8,	//8M
			sdio_bits: 4,
			uart_message: true,
			login_prompt_admin: "Administrator",
			login_prompt_user: "Camera Server",
			other_info: "IP_CAM V1.0 96/08/31\n"
				  + "ICE Board V1.6 2007/04/03\n"
				  + "W99802G\n"
				  + "OV7670, PO6030K, OV9650, OV7725\n"
				  + "WM8978\n"
				  + "SPI flash\n"
				  + "CyterTan SD-8686 Bulverde\n"
				  + "SDIO: disable card detect, power pin & write protected\n",
			definitions: "IPCAM_CONFIG_IP_CAM_VER_0"
		},		
		{
			name: "[W99802] KoiTech Wowwee: IP_CAM+SPI+(OV7670,PO6030K,...)",
			default_essid: "ROVIO_WOWWEE",
			default_wifi_mode: "ad-hoc",/* "ad-hoc" or "Infrastructure" */
			default_mac: "00:01:03:20:70:0a",
			default_ip: "192.168.10.18",
			default_netmask: "255.255.255.0",
			default_gateway: "192.168.10.1",
			default_dhcp: false,
			sensor: "SENSOR_OV7670",
			audio: "WM8978_NO_GPIO",
			memory_size: 8,	//8M
			sdio_bits: 4,
			uart_message: false,
			login_prompt_admin: "Administrator for Rovio",
			login_prompt_user: "Rovio",			
			other_info: "IP_CAM V1.0 96/08/31\n"
				  + "ICE Board V1.6 2007/04/03\n"
				  + "W99802G\n"
				  + "OV7670, PO6030K, OV9650, OV7725\n"
				  + "WM8978\n"
				  + "SPI flash\n"
				  + "CyterTan SD-8686 Bulverde\n"
				  + "SDIO: disable card detect, power pin & write protected\n",
			definitions: "IPCAM_CONFIG_IP_CAM_VER_1"
		},
		{
			name: "[W99802] KoiTech Wowwee: IP_CAM+SPI+PO6030K",
			default_essid: "KOITECH_REV_WB",
			default_wifi_mode: "infrastructure",/* "ad-hoc" or "Infrastructure" */
			default_mac: "00:01:03:20:70:0a",
			default_ip: "192.168.10.18",
			default_netmask: "255.255.255.0",
			default_gateway: "192.168.10.1",
			default_dhcp: false,
			sensor: "SENSOR_PO6030K",
			audio: "WM8978_NO_GPIO",
			memory_size: 8,	//8M
			sdio_bits: 4,
			uart_message: false,
			login_prompt_admin: "Administrator",
			login_prompt_user: "Camera Server",			
			other_info: "IP_CAM V1.0 96/08/31\n"
				  + "ICE Board V1.6 2007/04/03\n"
				  + "W99802G\n"
				  + "PO6030\n"
				  + "WM8978\n"
				  + "SPI flash\n"
				  + "CyterTan SD-8686 Bulverde\n"
				  + "SDIO: disable card detect, power pin & write protected\n",
			definitions: "IPCAM_CONFIG_IP_CAM_VER_2"
		},
		{
			name: "[W99803] KoiTech Wowwee: IP_CAM+SPI+(OV7670,PO6030K,...)",
			default_essid: "ROVIO_WOWWEE",
			default_wifi_mode: "ad-hoc",/* "ad-hoc" or "Infrastructure" */
			default_mac: "00:01:03:20:70:0a",
			default_ip: "192.168.10.18",
			default_netmask: "255.255.255.0",
			default_gateway: "192.168.10.1",
			default_dhcp: false,
			sensor: "SENSOR_OV7670",
			audio: "WM8978_NO_GPIO",
			memory_size: 16,	//8M
			sdio_bits: 4,
			uart_message: false,
			login_prompt_admin: "Administrator for Rovio",
			login_prompt_user: "Rovio",			
			other_info: "IP_CAM V1.0 96/08/31\n"
				  + "ICE Board V1.6 2007/04/03\n"
				  + "W99803G\n"
				  + "OV7670, PO6030K, OV9650, OV7725\n"
				  + "WM8978\n"
				  + "SPI flash\n"
				  + "CyterTan SD-8686 Bulverde\n"
				  + "SDIO: disable card detect, power pin & write protected\n",
			definitions: "IPCAM_CONFIG_IP_CAM_VER_3"
		},
//		{
//			name: "[W99803] IP_CAM+SPI+(OV7670,PO6030K,...)",
//			default_essid: "default",
//			default_wifi_mode: "infrastructure",/* "ad-hoc" or "Infrastructure" */
//			default_mac: "00:01:03:20:70:0a",
//			default_ip: "192.168.10.18",
//			default_netmask: "255.255.255.0",
//			default_gateway: "192.168.10.1",
//			default_dhcp: true,
//			sensor: "SENSOR_OV7670",
//			audio: "WM8978_NO_GPIO",
//			memory_size: 16,	//8M
//			sdio_bits: 4,
//			uart_message: true,
//			login_prompt_admin: "Administrator",
//			login_prompt_user: "Camera Server",
//			other_info: "IP_CAM V1.0 96/08/31\n"
//				  + "ICE Board V1.6 2007/04/03\n"
//				  + "W99802G\n"
//				  + "OV7670, PO6030K, OV9650, OV7725\n"
//				  + "WM8978\n"
//				  + "SPI flash\n"
//				  + "CyterTan SD-8686 Bulverde\n"
//				  + "SDIO: disable card detect, power pin & write protected\n",
//			definitions: "IPCAM_CONFIG_IP_CAM_VER_4"
//		},
		{
			name: "[W99802] MP4-EVB+SPI+OV9650",
			default_essid: "NS20IPCAM",
			default_wifi_mode: "infrastructure",/* "ad-hoc" or "Infrastructure" */
			default_mac: "00:01:03:20:70:0a",
			default_ip: "192.168.0.8",
			default_netmask: "255.255.255.0",
			default_gateway: "192.168.0.1",
			default_dhcp: false,
			sensor: "SENSOR_OV9650",
			audio: "WM8978_GPIO",
			memory_size: 8,	//8M
			sdio_bits: 4,
			uart_message: true,
			login_prompt_admin: "Administrator",
			login_prompt_user: "Camera Server",			
			other_info: "MP4-EVB V1.4 2006/09/08\n"
				  + "ICE Board V1.5 2006/12/29\n"
				  + "W99802G\n"
				  + "OV9650\n"
				  + "WM8978\n"
				  + "SPI flash\n"
				  + "CyterTan SD-8686 Bulverde\n",
			definitions: "IPCAM_CONFIG_MP4_EVB_VER_1"
		},
		{
			name: "[W99802] MP4-EVB+NAND+OV9650",
			default_essid: "default",
			default_wifi_mode: "infrastructure",/* "ad-hoc" or "Infrastructure" */
			default_mac: "00:01:03:20:70:0a",
			default_ip: "10.132.11.88",
			default_netmask: "255.255.0.0",
			default_gateway: "10.132.1.254",
			default_dhcp: true,
			sensor: "SENSOR_OV9650",
			audio: "WM8978_GPIO",
			memory_size: 8,	//8M
			sdio_bits: 4,
			uart_message: true,
			login_prompt_admin: "Administrator",
			login_prompt_user: "Camera Server",			
			other_info: "MP4-EVB V1.4 2006/09/08\n"
				  + "ICE Board V1.5 2006/12/29\n"
				  + "W99802G\n"
				  + "OV9650\n"
				  + "WM8978\n"
				  + "NAND flash\n"
				  + "CyterTan SD-8686 Bulverde\n",
			definitions: "IPCAM_CONFIG_MP4_EVB_VER_0"
		}
	];

	this.__get_configs = function()
	{
		var fixed_config = "";
		if ( this.app.fso.FileExists( this.app.app_folder + "/config_sdk.js" ) )
		{
			eval( this.app.include( this.app.app_folder + "/config_sdk.js" ) );
		}
		for( var i = 0; i < this.configs_all.length; i++ )
		{
			if ( this.configs_all[i].definitions == fixed_config )
				return [this.configs_all[i]];
		}
		return this.configs_all;
	}

	this.configs = this.__get_configs();

//config
//	default
//	wowwee_ver0
//	wowwee_ver1

//FMI, 1 bit / 4 bit
//wowwee pages, include / not include
//High speed UART message

//Default IP settings
//	1. MAC
//	2. IP, netmask, gateway
//	3. DHCP
//	4. ESSID

//Audio configuration
//	AU_I2C_TYPE_T	i2cType;

//sdio, add GPIO
/*
#define WIFI_GPIO_PIN_A	6
#define WIFI_GPIO_PIN_B 11
#define WIFI_GPIO_PIN_C 17
	cyg_thread_delay(10);
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(UINT)WIFI_GPIO_PIN_A);
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | (UINT)WIFI_GPIO_PIN_A);
	cyg_thread_delay(10);
	outpw(REG_GPIO_OE, inpw(REG_GPIO_OE) & ~(UINT)(WIFI_GPIO_PIN_B|WIFI_GPIO_PIN_C));
	outpw(REG_GPIO_DAT, inpw(REG_GPIO_DAT) | (UINT)(WIFI_GPIO_PIN_B|WIFI_GPIO_PIN_C));
*/	

}

