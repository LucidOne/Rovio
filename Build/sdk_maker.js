this.sdk_maker = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "IPCam SDK maker";
	}

	this.source = this.app.app_folder + "/..";
	this.bin_target = this.app.app_folder + "/IPCamBin";
	this.target = this.app.app_folder + "/IPCamSDK";


	this.excluded_files =
	[
		/* Do not remove the tow lines. */
		/^\\Build\\IPCamSDK$/i,
		/^\\Binary\\IPCamSDK.*$/i,
		/********************************/

		/\.svn$/i,
		/\.*~$/i,
		/[\/\\]~[^\/\\]+$/i,
		/\.*\.swp$/i,
		/\.*\.bak$/i,
		/\.*\.axf$/i,
		/\.*\.lst$/i,
		/\.*\_Data$/i,

		/^\\Tool\\pmpupdate$/i,
		/^\\Client\\Applications\\Win32\\Verify$/i,
		/^\\Client\\Applications\\Win32\\vcomm2000$/i,

		/^\\Client\\libRtpClient\\.*\.c$/i,
		/^\\Client\\libRtpClient\\.*\.cpp$/i,
		/^\\Client\\libRtpClient\\.*\.cxx$/i,
		/^\\Client\\libRtpClient\\.*\.obj$/i,
		/^\\Client\\libRtpClient\\.*\.sbr$/i,
		/^\\Client\\libRtpClient\\.*\.pch$/i,
		/^\\Client\\libRtpClient\\.*\.dsw$/i,
		/^\\Client\\libRtpClient\\.*\.dsp$/i,
		/^\\Client\\libRtpClient\\.*\.ncb$/i,
		/^\\Client\\libRtpClient\\.*\.opt$/i,
		/^\\Client\\libRtpClient\\.*\.plg$/i,
		/^\\Client\\libRtpClient\\.*\.idb$/i,

		/^\\Client\\libWbDsRtp\\.*\.c$/i,
		/^\\Client\\libWbDsRtp\\.*\.cpp$/i,
		/^\\Client\\libWbDsRtp\\.*\.cxx$/i,
		/^\\Client\\libWbDsRtp\\.*\.obj$/i,
		/^\\Client\\libWbDsRtp\\.*\.sbr$/i,
		/^\\Client\\libWbDsRtp\\.*\.pch$/i,
		/^\\Client\\libWbDsRtp\\.*\.dsw$/i,
		/^\\Client\\libWbDsRtp\\.*\.dsp$/i,	
		/^\\Client\\libWbDsRtp\\.*\.ncb$/i,
		/^\\Client\\libWbDsRtp\\.*\.opt$/i,
		/^\\Client\\libWbDsRtp\\.*\.plg$/i,
		/^\\Client\\libWbDsRtp\\.*\.idb$/i,

		/^\\Client\\Applications\\.*\\Release$/i,
		/^\\Client\\Applications\\.*\\ReleaseUMinDependency$/i,
		/^\\Client\\Applications\\.*\\ReleaseMinSize$/i,
		/^\\Client\\Applications\\.*\\Debug$/i,
		
		/^\\Host\\Hash_Pool\\Src$/i,
		/^\\Host\\AMR\\Src$/i,
//		/^\\Host\\Audio\\Src$/i,
//		/^\\Host\\libeCos\\src$/i,
//		/^\\Host\\DSP\\Src$/i,
//		/^\\Host\\FMI\\eCos_new\\Src$/i,
//		/^\\Host\\GFX\\src$/i,
//		/^\\Host\\I2C\\src$/i,
//		/^\\Host\\JPEG\\Src$/i,
		/^\\Host\\libBMP\\Src$/i,
		/^\\Host\\libCmpImg\\Src$/i,
		/^\\Host\\libFont\\Src$/i,
		/^\\Host\\libFunc_Through_HIC\\Src$/i,
		/^\\Host\\libhttp\\Src$/i,
		/^\\Host\\libIMA_ADPCM\\Src$/i,
		/^\\Host\\libRemoteFunc\\Src$/i,
		/^\\Host\\libTinyThread\\Src$/i,
		/^\\Host\\MP4\\src$/i,
		/^\\Host\\libasf\\Src$/i,
		/^\\Host\\lib3gp_test\\Src$/i,
		/^\\Host\\libFtp\\Src$/i,
		/^\\Host\\libMail\\Src$/i,
		/^\\Host\\libRtspServer\\Src$/i,
//		/^\\Host\\VCE\\src$/i,
//		/^\\Host\\VideoPhone\\Src$/i,
//		/^\\Host\\VPE\\Src$/i,
//		/^\\Host\\VPOST\\Src$/i,
		/^\\Host\\WBFAT\\ecos\\Src$/i,
//		/^\\Host\\powerctrl\\project$/i,
//		/^\\Host\\powerctrl\\src$/i,
		/^\\Host\\libLCDDisplay\\Src$/i,
		/^\\Host\\libAULaw\\Src$/i,
//		/^\\Host\\SPI\\Src$/i,
		/^\\Host\\wpa_supplicant\\Src$/i,
		/^\\Host\\UPnP\\Src$/i
//		/^\\Host\\libUSB\\Src$/i,
//		/^\\Host\\LibCamera\\Src\\.*\.c$/i
	];

	this.excluded_files_special__not_wowwee =
	[
		/^\\Host\\libNS\\doc$/i,
		/^\\Host\\libNS\\Socket$/i,
		/^\\Host\\libNS\\Src$/i,
		/^\\Host\\libNS\\[^\\]*\.[^\\]*$/i
	];

	this.__is_excluded = function( path, config )
	{
		for ( var i = 0; i < this.excluded_files.length; i++ )
		{
			if ( path.match( this.excluded_files[i] ) )
				return true;
		}

		/* The tree target is for Wowwee */
		if( config.name != "IPCAM_CONFIG_IP_CAM_VER_1"
			&& config.name != "IPCAM_CONFIG_IP_CAM_VER_2"
			&& config.name != "IPCAM_CONFIG_IP_CAM_VER_3" )
		{
			for ( var i = 0; i < this.excluded_files_special__not_wowwee.length; i++ )
			{
				if ( path.match( this.excluded_files_special__not_wowwee[i] ) )
					return true;
			}
		}

		return false;
	}

	this.__make_folders = function( folders )
	{
	}

	this.copyfile = function( src, target )
	{
		try
		{
			this.app.fso.CopyFile( src, target );
			return true;
		}
		catch( err )
		{
			alert( 'Failed to copy file "' + src + '" to "' + target + '"!\n'
				+ 'Please check if the previous build steps were selected.' );
			throw err;
		}
	}

	this.__dup_folder = function( folder, base_folder, target_folder )
	{
		//alert( folder + "---" + base_folder + "--" + target_folder );
		//return;

		this.app.msg_out( target_folder + folder + "\n" );
		try
		{
			this.app.fso.CreateFolder( target_folder + folder );
		} catch ( err )
		{
			this.__make_folders( target_folder + folder );
			this.app.fso.CreateFolder( target_folder + folder );
		}
	}

	this.__dup_file = function( file, base_folder, target_folder )
	{
		//alert( file + "---" + base_folder + "--" + target_folder );
		//return;

		this.app.msg_out( target_folder + file + "\n" );
		try
		{
			this.copyfile( base_folder + file, target_folder + file );
		} catch ( err )
		{
			this.__make_folders( target_folder + file );
			this.copyfile( base_folder + file, target_folder + file );
		}
	}
	
	this.__find_folder = function( config, folder, base_folder, target_folder )
	{
		var fc = new Enumerator( folder.SubFolders );
		for ( ; !fc.atEnd( ); fc.moveNext( ) )
		{
			var path = fc.item().Path.substr( base_folder.length );
			if ( !this.__is_excluded( path, config ) )
			{
				this.__dup_folder( path, base_folder, target_folder );
				this.__find_folder( config, fc.item( ), base_folder, target_folder );
			}
		}

		var fc = new Enumerator (folder.Files);
		for (; !fc.atEnd(); fc.moveNext())
		{
			var path = fc.item().Path.substr( base_folder.length );
			if ( !this.__is_excluded( path, config ) )
				this.__dup_file( path, base_folder, target_folder );	
		}
	}


	this.get_release_date = function( )
	{
		/* Get SDK date. */
		var file = this.app.fso.OpenTextFile( this.app.app_folder + "/../ChangeLog.txt", 1 );
		var file_content = file.ReadAll( );
		file.Close( );
		file_content = file_content.replace( /"\s*__DATE__\s*"/, '__DATE__' );
		file_content = file_content.replace( /"\s*__TIME__\s*"/, '__TIME__' );
		eval( "var version = [" + file_content + "];" );
		var date_str = version[1].match(/\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}/);
		if ( date_str == null )
			date_str = "";
		else
		{
			function c2( s )
			{
				s = "00" + s;
				return s.substr( s.length - 2 );
			}
			var arr = date_str[0].replace( /(\s+|:)/g, "-" ).split("-");
			var date = new Date( Date.UTC( arr[0], (arr[1] - 1), arr[2], arr[3], arr[4], arr[5] ) );
			date_str = date.getFullYear( ) + "-" + c2( date.getMonth( ) + 1 ) + "-" + c2( date.getUTCDate( ) )
				+ "_" + c2( date.getHours( ) ) + "-" + c2( date.getMinutes( ) ) + "-" + c2( date.getSeconds( ) );
		}
		return date_str;
	}


	this.make_bin_package = function( config )
	{
		var date_str = this.get_release_date( );

		/* Copy the files */	
		if ( this.app.fso.FolderExists( this.bin_target ) )
			this.app.fso.DeleteFolder( this.bin_target, true );
		this.app.fso.CreateFolder( this.bin_target );	

		this.copyfile( this.source + "/Binary/IPCamInstall.exe", this.bin_target + "/" );
		this.copyfile( this.source + "/Binary/IPCamVerifySystem.exe", this.bin_target + "/" );
		this.app.fso.CopyFolder( this.source + "/Binary/FlashFiles", this.bin_target + "/" );
		if ( this.app.fso.FolderExists( this.bin_target + "/FlashFiles/.svn" ) )
			this.app.fso.DeleteFolder( this.bin_target + "/FlashFiles/.svn", true );

		this.app.fso.CreateFolder( this.bin_target + "/UpdateTool" );
		this.copyfile( this.source + "/Tools/IPCam_Update/8051_bootloader_*.BIN", this.bin_target + "/UpdateTool/" );
		this.app.fso.CopyFolder( this.source + "/Tools/IPCam_Update/8051_IspWriter", this.bin_target + "/UpdateTool/" );
		if ( this.app.fso.FolderExists( this.bin_target + "/UpdateTool/8051_IspWriter/.svn" ) )
			this.app.fso.DeleteFolder( this.bin_target + "/UpdateTool/8051_IspWriter/.svn", true );
		this.copyfile( this.source + "/Tools/IPCam_Update/updatepmp_*.exe", this.bin_target + "/UpdateTool/" );

		this.copyfile( this.source + "/Binary/img_spi_update__camera.bin", this.bin_target + "/img_spi__camera.bin" );
		this.copyfile( this.source + "/Binary/img_spi__emi.bin", this.bin_target + "/img_spi__emi.bin" );
		
		this.copyfile( this.source + "/Host/custom_config.h", this.bin_target + "/build_config.txt" );

		var compressed_file = this.app.app_folder + "/../Binary/IPCamBin_" + date_str + ".7z";
		if ( this.app.fso.FileExists( compressed_file ) )
			this.app.fso.DeleteFile( compressed_file, true );
		this.app.wsh.Run( this.app.app_folder + "/7za.exe a -t7z -mx=5 -ms=on "
			+ "\"" + compressed_file + "\" "
			+ "\"" + this.bin_target + "\"",
			1, true );
		if ( !this.app.fso.FileExists( compressed_file ) )
			this.app.die( "Can not create binary release package!" );		
	}


	this.make = function( config )
	{
		var date_str = this.get_release_date( );
		
		/* Copy the files */	
		if ( this.app.fso.FolderExists( this.target ) )
			this.app.fso.DeleteFolder( this.target, true );
		this.app.fso.CreateFolder( this.target );

		var source_folder = this.app.fso.GetFolder( this.source );
		this.__find_folder( config, source_folder, source_folder.Path, this.target );

		if( config.name == "IPCAM_CONFIG_IP_CAM_VER_1"
			|| config.name == "IPCAM_CONFIG_IP_CAM_VER_2"
			|| config.name == "IPCAM_CONFIG_IP_CAM_VER_3" )
		{	/* For wowwee targets */
			if ( this.app.fso.FileExists( this.target + "/Build/build_modules_sdk_wowwee.js" ) )
			{
				this.copyfile( this.target + "/Build/build_modules_sdk_wowwee.js",
					this.target + "/Build/build_modules.js", true );
			}
		}
		else
		{
			if ( this.app.fso.FileExists( this.target + "/Build/build_modules_sdk_general.js" ) )
			{
				this.copyfile( this.target + "/Build/build_modules_sdk_general.js",
					this.target + "/Build/build_modules.js", true );
			}
		}

		var file = this.app.fso.CreateTextFile( this.target + "/Build/config_sdk.js", true, false );
		file.Write( "var fixed_config=\"" + config + "\";");
		file.Close( );
		
		this.copyfile( this.source + "/Host/custom_config.h", this.target + "/build_config.txt" );

		var compressed_file = this.app.app_folder + "/../Binary/IPCamSDK_" + date_str + ".7z";
		if ( this.app.fso.FileExists( compressed_file ) )
			this.app.fso.DeleteFile( compressed_file, true );
		this.app.wsh.Run( this.app.app_folder + "/7za.exe a -t7z -mx=5 -ms=on "
			+ "\"" + compressed_file + "\" "
			+ "\"" + this.target + "\"",
			1, true );
		if ( !this.app.fso.FileExists( compressed_file ) )
			this.app.die( "Can not create SDK!" );
	}

}

