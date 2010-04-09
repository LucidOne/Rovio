this.installer = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "installer";
	}

	this.create = function( install_script_path )
	{
		var installer_path = this.app.wsh.RegRead( "HKCR\\InnoSetupScriptFile\\shell\\open\\command\\" );
	
		if ( installer_path.substr( 0, 1 ) == "\"" )
			installer_path = installer_path.substr( 1, installer_path.indexOf( "\"", 1 ) - 1 );
		else	
			installer_path = installer_path.substr( 0, installer_path.indexOf( " " ) - 1 );
		installer_path = this.app.fso.GetFile( installer_path ).ParentFolder + "\\ISCC.exe";

		if ( !this.app.fso.FileExists( installer_path ) )
			this.app.die( "InnoSetup command line tools not found!" );

		this.app.wsh.Run( '"' + installer_path + '" "' + install_script_path + '"', 0, true );
	}
}

