this.firmware_convertor = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "Compress firmware and create update binary"
	}

	this.make_self_unzipped = function( src_bin, zip_bin )
	{
		try
		{
			this.app.wsh.Run( "\"" + this.app.app_folder + "/7za.exe\" a -tzip -mx=9 "
				+ "\"" + this.app.app_folder + "/" + zip_bin + ".zip\" "
				+ "\"" + this.app.app_folder + "/" + src_bin + "\"",
				1, true );
		} catch( e )
		{
			this.app.die( "Can not call 7z to compress zip file!" );
		}

		try
		{
			this.app.wsh.Run( "cmd /C copy /b "
				+ "\"" + this.app.app_folder + "\\wpackage.bin\" + "
				+ "\"" + this.app.app_folder + "\\" + zip_bin + ".zip\" "
				+ "\"" + this.app.app_folder + "\\..\\Binary\\" + zip_bin + "\" ",
				0, true );
			this.app.wsh.Run( "cmd /C del /f "
				+ "\"" + this.app.app_folder + "\\" + zip_bin + ".zip\" ",
				0, true );
			if ( !this.app.fso.FileExists( this.app.app_folder + "\\..\\Binary\\" + zip_bin ) )
				throw zip_bin + " not created";
		} catch( e )
		{
			this.app.die( "Can not create auto decompressed executable: " + zip_bin + "." );
		}
		
	}


	this.make_usb_image_for_nand = function( zip_bin, nand_update_bin )
	{
		try
		{
			var folder = this.app.app_folder + "\\bin";
			if ( this.app.fso.FolderExists( folder ) )
				this.app.fso.DeleteFolder( folder, true );
			this.app.fso.CreateFolder( folder );
			this.app.fso.CopyFile(  this.app.app_folder + "\\..\\Binary\\" + zip_bin, folder + "\\pmp.bin", true );
			this.app.wsh.Run( "\"" + this.app.app_folder + "/7za.exe\" a -tzip -mx=9 "
				+ "\"" + this.app.app_folder + "/../Binary/" + nand_update_bin + "\" "
				+ "\"" + folder + "\"",
				1, true );
			this.app.fso.DeleteFolder( folder, true );
			if ( !this.app.fso.FileExists( this.app.app_folder + "/../Binary/" + nand_update_bin ) )
				throw nand_update_bin + " not created";
		} 
		catch( e )
		{
			this.app.die( "Can not call 7z to compress " + nand_update_bin + " file!" );
		}
	}

	this.make_usb_image_for_spi = function( )
	{
		alert( "To create update image for SPI flash,\n"
			+ "be sure to run\n\n    Binary\\FlashFiles\\FirmwareMaker.exe\n\nand click \"Create\" to pack the resource files.\n" );
		var sOldCurrentDir = this.app.wsh.CurrentDirectory;
		this.app.wsh.CurrentDirectory = this.app.app_folder + "\\..\\Binary\\FlashFiles";
		this.app.wsh.Run( "\"" + this.app.app_folder + "\\..\\Binary\\FlashFiles\\FirmwareMaker.exe\"", 1, true );
		this.app.wsh.CurrentDirectory = sOldCurrentDir;
		return;	//Nothing need to be done in new version since 2007.10.26

		try
		{
			var current_dir = this.app.wsh.CurrentDirectory;
			this.app.wsh.CurrentDirectory = this.app.app_folder + "\\..\\Binary";
			this.app.wsh.Run( "\"" + this.app.app_folder + "\\..\\Tools\\IPCam_Update\\W99802_FWLoader_SPI\\IPCam_utility\\SPI Utility\\romh.exe\" "
				+ "\"" + this.app.app_folder + "/../Binary/img_spi.bin\" "
				+ "07072501",
				1, true );
			this.app.wsh.CurrentDirectory = current_dir;

			this.app.wsh.Run( "\"" + this.app.app_folder + "/7za.exe\" a -tzip -mx=9 "
				+ "\"" + this.app.app_folder + "/../Binary/usbimg_spi.bin\" "
				+ "\"" + this.app.app_folder + "/../Binary/pmp.bin\"",
				1, true );
			this.app.fso.DeleteFile( this.app.app_folder + "/../Binary/pmp.bin", true );
			if ( !this.app.fso.FileExists( this.app.app_folder + "/../Binary/usbimg_spi.bin" ) )
				throw "usbimg_spi.bin not created";
		} 
		catch( e )
		{
			this.app.die( "Can not call 7z to compress usbimg_spi.bin file!" );
		}
	}
}

