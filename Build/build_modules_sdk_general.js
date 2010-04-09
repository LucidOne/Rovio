this.build_modules = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "build_modules";
	}


	this.sdk_maker = new sdk_maker( this.app );
	this.builder_vc = new builder_vc( this.app );
	this.cab_maker = new cab_maker( this.app );
	this.installer = new installer( this.app );
	this.res_convertor = new res_convertor( this.app );
	this.firmware_convertor = new firmware_convertor( this.app );
	this.builder_mcp = new builder_mcp( this.app );
	this.mcp_ide_is_created = false;

	this.build_node = function( module, config )
	{
		this.app.msg_out( "Check: " + module.name + "\n" );
		return false;
	}
	this.build_copy_file = function( module, config )
	{
		this.app.fso.CopyFile( this.app.app_folder + "/" + module.args.from, this.app.app_folder + "/" + module.args.to, true );
		this.app.msg_out( "Copy file: " + module.name + ", OK!\n" );
		return false;
	}
	this.build_make_signed_cab = function( module, config )
	{
		this.cab_maker.make( this.app.app_folder + "/" + module.args.from, 
			this.app.app_folder + "/" + module.args.to,
			this.app.app_folder + "/" + module.args.sign_script );
		this.app.msg_out( "Make cab file: " + module.name + "\n" );
		return false;
	}
	this.__build_mcp_check_close = function( module, config )
	{
		function check_mcp_module( module, args )
		{
			if ( module.func != args.func )
				return;
				
			if ( args.found_me && module.selected )
				args.exist_mcp = true;
				
			if ( args.my_module == module )
				args.found_me = true;
		}
		
		var args = { func: this.build_mcp, my_module: module, found_me: false, exist_mcp: false };
		this.dump_module ( check_mcp_module, args );
		if ( !args.exist_mcp && this.mcp_ide_is_created )
		{
			this.builder_mcp.delete_ide( );
			this.mcp_ide_is_created = false;
		}
		return false;	
	}
	this.build_mcp = function( module, config )
	{
		if ( !this.mcp_ide_is_created )
		{
			this.builder_mcp.create_ide( );
			this.mcp_ide_is_created = true;
		}

		var path = this.app.fso.GetFile( this.app.app_folder + "/" + module.name ).Path;
		this.builder_mcp.build( path, module.args.target );
		this.app.msg_out( "Build mcp: " + module.name + "\n" );
		this.__build_mcp_check_close( module, config );
		return false;
	}
	this.build_mcp_bug_warning = function( module, config )
	{
		if ( confirm( "It seems that a version of ADS has bugs that can not build the module:\n"
			+ "  " + module.name + "\n"
			+ "You should manually build this module firstly, and reload the build script and build again!\n"
			+ "Would you like to stop the script now and manually build this module file by file?" ) )
			throw new Error("Building procedure was stopped by user!");	
		return this.build_mcp( module, config );
	}
	this.build_mcp_no_check = function( module, config )
	{	
		if ( !this.mcp_ide_is_created )
		{
			this.builder_mcp.create_ide( );
			this.mcp_ide_is_created = true;
		}

		var path = this.app.fso.GetFile( this.app.app_folder + "/" + module.name ).Path;
		this.builder_mcp.build_without_checking_mcp( path, module.args.target );
		this.app.msg_out( "Build mcp: " + module.name + "\n" );
		this.__build_mcp_check_close( module, config );	
		return false;
	}
	this.build_vc = function( module, config )
	{
		this.builder_vc.build( this.app.app_folder + "/" + module.name, module.args.proj, module.args.target );
		this.app.msg_out( "Build VC: " + module.name + ", OK!\n" );
		return false;
	}
	this.build_installer = function( module, config )
	{
		this.installer.create( module.args );
		this.app.msg_out( "Create installer: " + module.args + "\n" );
		return false;
	}
	this.build_server = function( module, config )
	{
		this.__build_submods( module.submods, config );
		return true;
	}
	this.build_convert_res = function( module, config )
	{
		this.app.msg_out( "Convert resource file: " + module.name + "\n" );
		this.res_convertor.convert( config );	
		return false;
	}
	this.build_ipcam_image = function( module, config )
	{
		this.app.msg_out( "Create WEB update image: " + module.args.zip_bin + "\n" );
		this.firmware_convertor.make_self_unzipped( module.args.src_bin, module.args.zip_bin );
		if( config.definitions == "IPCAM_CONFIG_MP4_EVB_VER_0" )
		{
			this.app.msg_out( "Create USB update image for nand flash: " + module.args.nand_update_bin + "\n" );
			this.firmware_convertor.make_usb_image_for_nand( module.args.zip_bin, module.args.nand_update_bin );
		}
		return false;
	}
	this.append_ipcam_image_res = function( module, config )
	{
		if( config.definictions != "IPCAM_CONFIG_MP4_EVB_VER_0" )
		{
			this.app.msg_out( "Create USB update image for spi flash: " + module.args.spi_update_bin + "\n" );
			this.firmware_convertor.make_usb_image_for_spi( );
		}
		return false;
	}
	this.build_bin_package = function( module, config )
	{
		this.sdk_maker.make_bin_package( config );
		this.app.msg_out( "Create IPCam binary release package\n" );
		return false;
	}
	this.build_sdk = function( module, config )
	{
		this.sdk_maker.make( config );
		this.app.msg_out( "Create IPCam SDK\n" );
		return false;
	}

	this.module =
	{
		func: this.build_node,
		name: "All components",
		args: null,
		submods:
		[
			{
				func: this.build_node,
				name: "Client components",
				args: null,
				submods:
				[
					{
						func: this.build_vc,
						name: "../Client/Applications/Win32/WebSee/WebSee.dsw",
						args:
						{
							proj: "WebSee",
							target: "WebSee - Win32 Release MinSize"
						},
						submods:
						[
							{
								func: this.build_copy_file,
								name: "Copy to WebSee.dll to folder ../Binary",
								args:
								{
									from: "../Client/Applications/Win32/WebSee/ReleaseMinSize/WebSee.dll",
									to: "../Binary/WebSee.dll"
								},
								submods: [ ]
							},
							{
								func: this.build_make_signed_cab,
								name: "Compress WebSee.dll to WebSee.cab and sign it",
								args:
								{
									from: "../Client/Applications/Win32/WebSee/cabfile.txt",
									to: "../Binary/WebSee.cab",
									sign_script: "../Client/Applications/Win32/WebSeeSign"
								},
								submods: [ ]
							}
						]
					},
					{
						func: this.build_vc,
						name: "../Client/Applications/Win32/WbStreamPlayer/WbStreamPlayer.dsw",
						args:
						{
							proj: "WbStreamPlayer",
							target: "WbStreamPlayer - Win32 Release"
						},
						submods:
						[
							{
								func: this.build_copy_file,
								name: "Copy to WbStreamPlayer.exe to folder ../Binary",
								args:
								{
									from: "../Client/Applications/Win32/WbStreamPlayer/Release/WbStreamPlayer.exe",
									to: "../Binary/WbStreamPlayer.exe"
								},
								submods: [ ]
							}
						]
					},
					{
						func: this.build_vc,
						name: "../Client/Applications/Win32/IPCamConfigure/IPCamConfigure.dsw",
						args:
						{
							proj: "IPCamConfigure",
							target: "IPCamConfigure - Win32 Release"
						},
						submods:
						[
							{
								func: this.build_copy_file,
								name: "Copy to IPCamConfigure.exe to folder ../Binary",
								args:
								{
									from: "../Client/Applications/Win32/IPCamConfigure/Release/IPCamConfigure.exe",
									to: "../Binary/IPCamConfigure.exe"
								},
								submods: [ ]
							},
							{
								func: this.build_copy_file,
								name: "Copy to enumvcom.dll to folder ../Binary",
								args:
								{
									from: "../Client/Applications/Win32/IPCamConfigure/enumvcom.dll",
									to: "../Binary/enumvcom.dll"
								},
								submods: [ ]
							}
						]
					},
					{
						func: this.build_vc,
						name: "../Client/Applications/Win32/IPEdit/src/ipEdit.dsw",
						args:
						{
							proj: "ipEdit",
							target: "ipEdit - Win32 Release"
						},
						submods:
						[
							{
								func: this.build_copy_file,
								name: "Copy to WbStreamPlayer.exe to folder ../Binary",
								args:
								{
									from: "../Client/Applications/Win32/IPEdit/src/Release/ipEdit.exe",
									to: "../Binary/ipEdit.exe"
								},
								submods: [ ]
							}
						]
					},
					{
						func: this.build_installer,
						name: "Create IPCam installer package",
						args: "../Tools/IPCamInstaller/IPCam.iss",
						submods: [ ]
					}
				]
			},
			{
				func: this.build_server,
				name: "Server components",
				args: null,
				submods:
				[
					{
						func: this.build_mcp,
						name: "../Host/Audio/Src/Audio.mcp",
						args:
						{
							target: "WM8978"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/DSP/Src/dsplib.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/FMI/eCos_new/Src/FMILIB/FMILIB.mcp",
						args:
						{
							target: "MASS"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/GFX/src/gfxlib.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/I2C/src/i2clib.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/JPEG/Src/JPEGLib.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/libHIC_Host/Src/libHIC_Host.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/VCE/src/caplib.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/VideoPhone/Src/Build/VideoPhone.mcp",
						args:
						{
							target: "IPCam_WiFi"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/VPE/Src/vpelib.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/VPOST/Src/VPOST.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/powerctrl/project/ecos/powerctrl.mcp",
						args:
						{
							target: "DebugRel"
						},						
						submods: [ ]
					},
					{
						func: this.build_mcp,
						name: "../Host/SPI/Src/libSPI.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					}, 
					{
						func: this.build_mcp,
						name: "../Host/libUSB/Src/libUSB.mcp",
						args:
						{
							target: "Release"
						},						
						submods: [ ]
					},  
					{
						func: this.build_node,
						name: "IPCam application",
						args: null,
						submods:
						[
							{
								func: this.build_convert_res,
								name: "Convert IPCam resource files.",
								args: null,
								submods: [ ]
							},
							{
								func: this.build_mcp,
								name: "../Host/LibCamera/Src/LibCamera.mcp",
								args:
								{
									target: "Release"
								},						
								submods: [ ]
							},
							{
								func: this.build_mcp,
								name: "../Host/LibCamera/Samples/CameraTest/Src/CameraTest.mcp",
								args:
								{
									target: "functest"
								},						
								submods:
								[
									{
										func: this.build_ipcam_image,
										name: "Create update image for CameraTest.mcp.",
										args:
										{
											src_bin: "../Host/LibCamera/Samples/CameraTest/Bin/CameraTest.bin",
											zip_bin: "img_spi__camera.bin",
											nand_update_bin: "img_nand__camera.bin"
										},
										submods:
										[
											{
												func: this.append_ipcam_image_res,
												name: "Append WEB files to update image.",
												args:
												{
													zip_bin: "img_spi__camera.bin",
													spi_update_bin: "img_spi_update__camera.bin"
												},
												submods: [ ]
											}
										]
									}
								]
							},
							{
								func: this.build_mcp,
								name: "../Host/LibCamera/Samples/EMI_Test/Src/EMI_Test.mcp",
								args:
								{
									target: "functest"
								},						
								submods:
								[
									{
										func: this.build_ipcam_image,
										name: "Create update image for EMI_Test.mcp.",
										args:
										{
											src_bin: "../Host/LibCamera/Samples/EMI_Test/Bin/EMI_Test.bin",
											zip_bin: "img_spi__emi.bin",
											spi_update_bin: "img_spi_update__emi.bin",
											nand_update_bin: "img_nand__emi.bin"
										},
										submods: [ ]
									}
								]
							}
						]
					}
				]
			},
			{
				func: this.build_bin_package,
				name: "Create Binaray release package",
				args: null,
				submods: []
			},
			{
				func: this.build_sdk,
				name: "Create IPCam SDK",
				args: null,
				submods: [ ]
			}
		]
	};

	function __dump_module( module, func, args )
	{
		func( module, args );
		for ( var i = 0; i < module.submods.length; i++ )
			__dump_module( module.submods[i], func, args );			
	}

	this.dump_module = function( func, args )
	{
		__dump_module( this.module, func, args );
	}



	this.__get_state = function( module, args )
	{
		args.str += (module.selected ? "1" : "0");
	}

	this.get_state = function( )
	{
		var args = {str:""};
		this.dump_module( this.__get_state, args );
		return args.str;
	}

	this.__set_state = function( module, args )
	{
		if ( args.index >= args.state.length )
			module.selected = true;
		else
			module.selected = ( args.state.charAt(args.index) == '0' ? false : true );
		args.index++;
	}

	this.set_state = function( state )
	{
		var args = {state:state, index: 0};
		this.dump_module( this.__set_state, args );
		return args.str;
	}

	function __init_module( module, id )
	{
		module.id = id;
		module.selected = true;
		for ( var i = 0; i < module.submods.length; i++ )
		{
			var new_id = id + "_" + i;
			__init_module( module.submods[i], new_id );
		}
	}

	this.cookie = new cookie( this.g_app );
	this.load_state = function( )
	{
		var selected = this.cookie.get_cookie( "s" );
		if (selected == null )
			selected = "";
		this.set_state( selected );
	}
	this.save_state = function( )
	{
		var selected = this.get_state( );
		this.cookie.set_cookie( "s", selected );
	}

	__init_module( this.module, "0" );
	this.load_state( );
	


	this.func = 0;

	this.__build_submods = function( submods, config )
	{
		for ( var i = 0; i < submods.length; i++ )
			this.__build_module( submods[i], config );
	}

	this.__build_module = function( module, config )
	{
		var rt = false;
		if ( module.selected )
		{
			this.func = module.func;
			rt = this.func( module, config );
		}

		if ( !rt )
		{
			this.__build_submods( module.submods, config );
		}
	}


	this.build = function( config )
	{
		this.__build_module( this.module, config );
	}


}

