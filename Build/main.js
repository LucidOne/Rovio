
eval( this.g_app.include( "config.js" ) );
eval( this.g_app.include( "cookie.js" ) );
eval( this.g_app.include( "sdk_maker.js" ) );
eval( this.g_app.include( "installer.js" ) );
eval( this.g_app.include( "builder_vc.js" ) );
eval( this.g_app.include( "builder_mcp.js" ) );
eval( this.g_app.include( "cab_maker.js" ) );
eval( this.g_app.include( "build_modules.js" ) ); 
eval( this.g_app.include( "bin_reader.js" ) );
eval( this.g_app.include( "res_convertor.js" ) );
eval( this.g_app.include( "firmware_convertor.js" ) );
this.g_app.build_modules = new build_modules( this.g_app );
this.g_app.config = new config( this.g_app );

this.hws_app.prototype.main = function( )
{
	if ( this.app_type.match( /script$/ ) )
		this.msg_out( "In script" );

	return true;
}

this.hws_app.prototype.onexit = function( )
{
	
}


