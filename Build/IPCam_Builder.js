this.hws_app = function( )	/* HTML/WScript compatible object */
{
	/* objects. */
	this.wsh = new ActiveXObject( "WScript.Shell" );
	this.fso = new ActiveXObject( "Scripting.FileSystemObject" );
	this.shl = new ActiveXObject( "Shell.Application" );

	/* Application type */
	this.__get_app_type = function( )
	{
		try
		{
			if ( typeof( WScript ) == "object" )
			{
				var name = WScript.FullName;
				if ( name.match( /wscript/i ) )
					return "wscript";
				else if ( name.match( /cscript/i ) )
					return "cscript";
			}
			else
			{
				var tags = document.getElementsByTagName( "application" );
				if ( tags == 0 )
					return "html";
				else
					return "hta";
			}
		}
		catch ( error )
		{
		}
		return "unknown";
	}
	
	this.app_type = this.__get_app_type( );
	
	
	
	/* Command line arguments */
	this.__get_app_argv = function( )
	{
		this.__wsc_get_argv = function( )
		{
			var argv = [this.fso.GetFile( WScript.ScriptFullName ).Path];
			for ( var i = 0; i < WScript.Arguments.length; i++ )
				argv[argv.length] = WScript.Arguments( i );
			return argv;
		}	
		
	
		this.__hta_cmdline_to_argv = function( cmdline )
		{
			function escape_words( s )
			{
				s = s.replace( /@/g, "@@@" );
				s = s.replace( /-/g, "@@-" );
				s = s.replace( /#/g, "@--" );
				return s;
			}
			function unescape_words( s )
			{
				s = s.replace( /@--/g, "#" );
				s = s.replace( /@@-/g, "-" );
				s = s.replace( /@@@/g, "@" );
				return s;
			}	
			
			var words = "";
			var res;
			while ( cmdline.length > 0 )
			{
				if ( ( res = cmdline.match( /^"([^"\\]|""|\\.)*"/ ) ) != null )
				{
					cmdline = cmdline.substr( res[0].length );
					if ( res[0].length == 2 )
						words += '"';
					else
					{
						var str = res[0].substr( 1, res[0].length - 2 );
						str = str.replace( /(""|\\")/g, '"' );
						words += escape_words( str );
					}
				}
				else if ( cmdline == '"' )
				{
					words += '#"';
					cmdline = "";
				}
				else if ( ( res = cmdline.match( /([^"\\]|""|\\.)*"/ ) ) != null )
				{
					cmdline = cmdline.substr( res[0].length - 1 );
					if ( res[0].length >= 2 )
					{
						var str = res[0].substr( 0, res[0].length - 1 );
						str = str.replace( /(""|\\")/g, '"' );
						str = escape_words( str );
						words += str.replace( /\s+/g, "#" );
					}
				}
				else
				{
					var str = escape_words( cmdline );
					str = str.replace( /(""|\\")/g, '"' );
					words += str.replace( /\s+/g, "#" );
					cmdline = "";
				}
			}
			
			var args = words.split ("#");
			
			var rt = [];
			for ( var i = 0; i < args.length; i++ )
			{
				if ( args[i].length > 0 )
					rt[rt.length] = unescape_words( args[i] );
			}
			
			return rt;
		}
		
	
		if ( typeof( WScript ) == "object" )
		{
			return this.__wsc_get_argv( );
		}
		else
		{
			var hta = document.getElementsByTagName( "application" );
			if ( hta.length > 0 )
				return this.__hta_cmdline_to_argv( hta[0].commandLine );
			else
				return this.__hta_cmdline_to_argv( location + "" );
		}
	}
	

	this.argv = this.__get_app_argv( );
	this.app_path = this.argv[0];
	this.app_name = this.app_path.replace( /^.*[/\\]/, "" );
	this.app_folder = this.fso.GetFile( this.app_path ).ParentFolder;
	

	this.toString = function ( )
	{
		return "HTML/WScript compatible object [" + this.app_name + "]";
	}
	

	/* Information handler */
	this.die = function( s )
	{
		throw( new Error( s ) );
	}
	
	this.__msg_out = function( s, out_handle )
	{
		if ( typeof( ui_msg_out ) == "function" )
			ui_msg_out( s );
		else
		{
			try
			{
				if ( out_handle == "stdout" )
					WScript.StdOut.Write( s );
				else if ( out_handle == "stderr" )
					WScript.StdErr.Write( s );
				else
					throw( "Unknow output handler" );
			}
			catch ( error )
			{
				var file = this.wsh.Environment( "Process" )( "TEMP" ) + "\\" + this.app_name + ".log";
				{
					var logfile = this.fso.OpenTextFile( file, 8, true );
					if ( logfile != null )
					{
						logfile.Write( s );
						logfile.Close( );
					}
				}
				
				if ( this.wsh.Popup( "The message is:\n"
					+ "\n"
					+ s + "\n"
					+ "\n"
					+ "The message has been written to " + file + ",\n"
					+ "Continue with the script?",
					-1, "Output message error!", 4 + 16 ) != 6 )
				{
					if ( typeof( WScript ) == "object" )
						WScript.Quit( -1 );
					else
						window.close( );
				}
			}
		}
	}
	
	
	this.msg_out = function( s )
	{
		this.__msg_out( s, "stdout" );
	}
	
	
	this.err_out = function( s )
	{
		this.__msg_out( s, "stderr" );
	}
	
	
	
	/* Include */
	
	this.__inc_is_absolution_path = function( path )
	{
		if ( path.charAt(0) == '\\' || path.charAt(0) == '/'
			|| path.match( /^[a-zA-Z]:/ ) )
			return true;
		else
			return false;
	}
	
	this.__inc_get_script_dir = function( )
	{
		return this.fso.GetParentFolderName( this.app_path );
	}	
	
	this.include = function( in_path )
	{
		var path;
		if (!this.__inc_is_absolution_path( in_path ) )
			path = this.__inc_get_script_dir( ) + "/" + in_path;
		else
			path = in_path;
	
		if ( !this.fso.FileExists( path ) )
			this.die( "File " + in_path + " not found" );
	
		path = this.fso.GetFile( path ).Path;
	
		var file = this.fso.OpenTextFile (path, 1);
		var script = file.AtEndOfStream ? "" : file.ReadAll ();
		file.Close ();
	
		return script;
	}
}

this.g_app = new hws_app( );

eval( this.g_app.include( "main.js" ) );

/* Do not remove it! */
var main_check_in_html = function( )
{
	if ( document.readyState == "complete" )
		main_run( );
	else
		setTimeout( "main_check_in_html( );", 50 );
}

var main_run = function( )
{
	var run_exit;
	if ( this.g_app.app_type.match( /^ht/ ) )
	{
		this.g_app.onunload = document.body.onunload;
		document.body.onunload = main_over;
		run_exit = false;
	}
	else
	{
		run_exit = true;
	}
	
	if ( typeof( this.g_app.main ) == "function" )
	{
		if ( this.g_app.main( ) )
		{
			if ( run_exit )
				main_over( );
		}
	}
	else
		this.g_app.msg_out( "Please define new this.hws_app.prototype.main." );
}

var main_over = function( )
{
	if ( this.g_app.onunload )
		this.g_app.onunload( );
	
	if ( typeof( this.g_app.onexit ) == "function" )
		this.g_app.onexit( );
}


/* Run application here. */
if ( this.g_app.app_type.match( /script$/ ) )
	main_run( );
else if ( this.g_app.app_type.match( /^ht/ ) )
	main_check_in_html( );
else
	this.g_app.die( "Can not get applicaton type." );


