this.cab_maker = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "cab_maker";
	}

	this.make = function( src_path, des_path, cert_script_path )
	{
		this.app.wsh.Run( 'makecab /F "' + src_path + '" /D compressiontype=lzx /D compressionmemory=21 /D maxdisksize=1024000000 /D diskdirectorytemplate= /D cabinetnametemplate=cabmaker.cab', 1, true );
		if( this.app.fso.FileExists( des_path ) )
			this.app.fso.DeleteFile( des_path );
		this.app.fso.MoveFile( 'cabmaker.cab', des_path );
		this.app.fso.DeleteFile( 'setup.inf' );
		this.app.fso.DeleteFile( 'setup.rpt' );
		alert( 'Please input "winbond" to sign the ActiveX cab in the next step.' );
		this.app.wsh.Run( '"' + cert_script_path + '/signcode.exe" /spc "'+ cert_script_path +'/client.cer" /v "'+ cert_script_path +'/client.pvk" "' + des_path + '"', 0, true );
	}
}

