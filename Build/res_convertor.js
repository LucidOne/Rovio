this.res_convertor = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "IPCam WEB files convertor";
	}

	this.bin_reader = new bin_reader( this.app );

	this.__find_file = function( file, base_path, files )
	{
		if( file.Name.match( /~$/ ) )
			return;
		if( file.Name.match( /\.bak$/i ) )
			return;
		if( file.Name.match( /\.swp$/i ) )
			return;
		if( file.Name.match( /^make_gz\.sh$/i ) )
			return;
		if( file.Path.substr( 0, base_path.length ) != base_path )
		{
			this.app.die( "Internal error!\n" );
		}

		var file_relative_path = file.Path.substr( base_path.length );
		var file_cgi_path = file_relative_path.replace( /\\/g, "/" );
		var file_name = "file" + file_relative_path.replace( /[^0-9A-Za-z]/g, "_" );
		
		files[files.length] =
		{
			file_obj: file,
			relative_path: file_relative_path,
			cgi_path: file_cgi_path,
			name: file_name
		};
	}

	this.__find_folder = function( folder, base_path, files )
	{
		if (folder.Name.toUpperCase() == ".svn".toUpperCase())
			return;
		var fc = new Enumerator (folder.SubFolders);
		for (; !fc.atEnd(); fc.moveNext())
		{
			this.__find_folder( fc.item(), base_path, files );
		}

		var fc = new Enumerator (folder.Files);
		for (; !fc.atEnd(); fc.moveNext())
		{
			this.__find_file( fc.item(), base_path, files );
		}
	}

	this.__win32_path = function( path )
	{
		return (path+"").replace( /\//g, "\\" );
	}

	this.__unix_path = function( path )
	{
		return (path+"").replace( /\\/g, "/" );
	}

	this.mkdir = function( path )
	{
		var dirs = [];
		var dir = path;
		while( true )
		{
			if (this.app.fso.FolderExists(dir))
				break;
			dirs[dirs.length] = dir;	
			var new_dir = dir.replace(/[\/\\][^\/\\]*$/, "");
			if (new_dir == dir)
				break;
			dir = new_dir;
		}
		
		for (var i = dirs.length - 1; i >= 0; --i)
			this.app.fso.CreateFolder( dirs[i] );
	}

	this.convert_append_files = function( config )
	{
		var sConvertPath = this.app.app_folder + "/../Binary/FlashFiles/Resource";
		if ( this.app.fso.FolderExists( sConvertPath ) )
			this.app.fso.DeleteFolder( sConvertPath, true );

		var files = [];
		
		var folder;
		if( config == "IPCAM_CONFIG_IP_CAM_VER_0" || config == "IPCAM_CONFIG_IP_CAM_VER_4" || config == "IPCAM_CONFIG_MP4_EVB_VER_1")
			folder = this.app.fso.GetFolder( this.app.app_folder + "/../Host/LibCamera/Res/Flash" );
		else if( config == "IPCAM_CONFIG_IP_CAM_VER_1" || config == "IPCAM_CONFIG_IP_CAM_VER_2" || config == "IPCAM_CONFIG_IP_CAM_VER_3")
			folder = this.app.fso.GetFolder( this.app.app_folder + "/../Host/LibCamera/Res/Flash_Koitech" );
		else
			this.app.die( "No platform configuration defined: [" + config + "]\n" );
		

		this.__find_folder( folder, folder.Path, files );

		/* Convert files by gzip */
		var sNoGZ_Path = "\\audio\\";
		for (var i = 0; i < files.length; i++)
		{
			var file = files[i];
			var target_file = sConvertPath + file.relative_path;
			this.mkdir( target_file.replace(/[\/\\][^\/\\]*$/, "") );
			this.app.fso.CopyFile( file.file_obj.Path, target_file );

			if (file.relative_path.substr(0,1) != '\\')
				this.app.die( "Internal error: relative path not start with '\\'!\n" );
			
			if (file.relative_path.substr(0, sNoGZ_Path.length).toLowerCase() == sNoGZ_Path.toLowerCase()
				|| file.relative_path.substr(file.relative_path.length - 4).toLowerCase() == ".jpg"
				|| file.relative_path.substr(file.relative_path.length - 4).toLowerCase() == ".gif"
				|| file.relative_path.substr(file.relative_path.length - 4).toLowerCase() == ".png"
				)
			{
				files[i].onboard_path = this.__unix_path( file.relative_path.substr(1) );
				files[i].local_path = this.__win32_path( "Resource" + file.relative_path );
				continue;
			}

			files[i].onboard_path = this.__unix_path( file.relative_path.substr(1) + ".gz" );
			files[i].local_path = this.__win32_path( "Resource" + file.relative_path + ".gz" );

			this.app.msg_out( "GZIP: " + target_file + "\n" );
			this.app.wsh.Run( '"' + this.app.app_folder + '\\gzip" -9 '
				+ '"' + this.__win32_path( target_file ) + '" ',
			0, true );
		}

		var ini_file = this.app.fso.CreateTextFile( sConvertPath + "/../FirmwareMaker.ini" );
		ini_file.WriteLine( "[PATH]" );
		ini_file.WriteLine( "FOLDER=." );
		ini_file.WriteLine( "INPUT_BIN=..\\img_spi__camera.bin" );
		ini_file.WriteLine( "OUTPUT_BIN=..\\img_spi_update__camera.bin" );
		ini_file.WriteLine( "FILE_NUM=" + files.length );		
		for (var i = 0; i < files.length; i++)
		{
			ini_file.WriteLine( "FILE_ONBOARD_" + i + "=" + files[i].onboard_path );
			ini_file.WriteLine( "FILE_LOCAL_" + i + "=" + files[i].local_path );
		}
		ini_file.Close( );
	}

	this.convert_includes_files = function( config )
	{
		var files = [];
		var folder = this.app.fso.GetFolder( this.app.app_folder + "/../Host/LibCamera/Res/App" );
		this.__find_folder( folder, folder.Path, files );
		if ( config == "IPCAM_CONFIG_MP4_EVB_VER_0" )
		{
			var folder = this.app.fso.GetFolder( this.app.app_folder + "/../Host/LibCamera/Res/Flash" );
			this.__find_folder( folder, folder.Path, files );
		}

		for (var i = 0; i < files.length; i++)
		{
			var file = files[i];
			this.app.msg_out( "Create: " + file.cgi_path + "\n" );

			this.bin_reader.dump_as_C_header( file.file_obj.Path, this.app.app_folder + "/../Host/LibCamera/Src/WebFiles/" + file.name + ".h" );
		}

		var C_header = this.app.fso.CreateTextFile( this.app.app_folder + "/../Host/LibCamera/Src/WebFiles/web_files_data.h", true, false );
		for (var i = 0; i < files.length; i++)
		{
			C_header.Write ("static char g_ac" + files[i].name + "[]=\n");
			C_header.Write ("{\n");
			C_header.Write ("#include \"" + files[i].name + ".h\"\n");
			C_header.Write ("};\n");	

			C_header.Write ("static WEB_FILE_T g_pg" + files[i].name + "=\n");
			C_header.Write ("{\n");
			C_header.Write ("\tg_ac" + files[i].name + ",\n");
			C_header.Write ("\t\"" + files[i].cgi_path + "\",\n");
			C_header.Write ("\tsizeof(g_ac" + files[i].name + "),\n");
			C_header.Write ("};\n\n");	
		}

		C_header.Write ("void RegisterWebFiles ()\n");
		C_header.Write ("{\n");
		for (var i = 0; i < files.length; i++)
		{
			C_header.Write (
				"\thttpRegisterEmbedFunEx(\"" + files[i].cgi_path + "\", Http_WebFile, AUTH_USER, (void *)&g_pg" + files[i].name + ");\n");
			if (files[i].cgi_path.toUpperCase() == "/index.htm".toUpperCase() )
				C_header.Write (
					"\thttpRegisterEmbedFunEx(\"/\", Http_WebFile, AUTH_USER, (void *)&g_pg" + files[i].name + ");\n");	
		}
		C_header.Write ("}\n\n");
	
		C_header.Close();
	}

	this.convert = function( config )
	{
		if ( config != "IPCAM_CONFIG_MP4_EVB_VER_0" )
			this.convert_append_files( config );
		this.convert_includes_files( config );
	}
}

