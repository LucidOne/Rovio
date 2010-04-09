this.bin_reader = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		"Binary file reader"
	}
	
	this.__win32_path = function( path )
	{
		return (path+"").replace( /\//g, "\\" );
	}

	this.dump_as_C_header = function( in_file_path, out_file_path )
	{
		this.app.wsh.Run( '"' + this.app.app_folder + '\\bin2C" '
			+ '"' + this.__win32_path( in_file_path ) + '" '
			+ '"' + this.__win32_path( out_file_path ) + '" ',
			0, true );
	}

	this.dump_as_C_header1 = function( in_file_path, out_file_path )
	{
		var in_file = this.app.fso.GetFile( in_file_path );
	
		var adodb_sm = new ActiveXObject( "ADODB.Stream" ); 
		var adodb_rs = new ActiveXObject( "ADODB.Recordset" );
	
		var start_time = new Date( );

		/* Load the binary files. */
		adodb_sm.Type = 1
		adodb_sm.Open( );
		adodb_sm.LoadFromFile( in_file_path );
		adodb_sm.Position = 0;
		var bin = adodb_sm.Read( );
	
		this.app.msg_out( "Time1: " + ( new Date( ) - start_time ) );

		/* Read the binary as a wide string. */
		var adLongVarWChar = 203;
		adodb_rs.Fields.Append( "bin", adLongVarWChar, 256 );
		adodb_rs.Open( );
		adodb_rs.AddNew( );
		/* Append the binary. */
		adodb_rs( "bin" ).AppendChunk( bin );
		/* If the file size is odd, append a dummy byte. */
		if ( Math.floor( in_file.Size / 2 ) * 2 != in_file.Size )
			adodb_rs( "bin" ).AppendChunk ( 'd' );
		adodb_rs.Update( );
		var str = adodb_rs( "bin" ).Value;

		this.app.msg_out( "Time2: " + ( new Date( ) - start_time ) );
		var hex_table = [
"00","01","02","03","04","05","06","07",
"08","09","0a","0b","0c","0d","0e","0f",
"10","11","12","13","14","15","16","17",
"18","19","1a","1b","1c","1d","1e","1f",
"20","21","22","23","24","25","26","27",
"28","29","2a","2b","2c","2d","2e","2f",
"30","31","32","33","34","35","36","37",
"38","39","3a","3b","3c","3d","3e","3f",
"40","41","42","43","44","45","46","47",
"48","49","4a","4b","4c","4d","4e","4f",
"50","51","52","53","54","55","56","57",
"58","59","5a","5b","5c","5d","5e","5f",
"60","61","62","63","64","65","66","67",
"68","69","6a","6b","6c","6d","6e","6f",
"70","71","72","73","74","75","76","77",
"78","79","7a","7b","7c","7d","7e","7f",
"80","81","82","83","84","85","86","87",
"88","89","8a","8b","8c","8d","8e","8f",
"90","91","92","93","94","95","96","97",
"98","99","9a","9b","9c","9d","9e","9f",
"a0","a1","a2","a3","a4","a5","a6","a7",
"a8","a9","aa","ab","ac","ad","ae","af",
"b0","b1","b2","b3","b4","b5","b6","b7",
"b8","b9","ba","bb","bc","bd","be","bf",
"c0","c1","c2","c3","c4","c5","c6","c7",
"c8","c9","ca","cb","cc","cd","ce","cf",
"d0","d1","d2","d3","d4","d5","d6","d7",
"d8","d9","da","db","dc","dd","de","df",
"e0","e1","e2","e3","e4","e5","e6","e7",
"e8","e9","ea","eb","ec","ed","ee","ef",
"f0","f1","f2","f3","f4","f5","f6","f7",
"f8","f9","fa","fb","fc","fd","fe","ff"
];

		var hex_table_word = [];
		for ( var i = 0; i < 256; i++ )
		{
			for ( var j = 0; j < 256; j++ )
				hex_table_word[hex_table_word.length] = hex_table[j] + hex_table[i];
		}
		this.app.msg_out( "Time3: " + ( new Date( ) - start_time ) );

		function hex( v )
		{
			return hex_table[v];
		}

		var hex_result = [];
		/* Convert the wide string to the string code. */
		for ( var i = 0; i < str.length; i++ )
		{
			var val = str.charCodeAt( i );
			hex_result[hex_result.length] = hex_table_word[val];
		}
		/* Remove last dummy byte is necessory. */
		if ( Math.floor( in_file.Size / 2 ) * 2 != in_file.Size )
			hex_result[hex_result.length - 1] = hex_result[hex_result.length - 1].substr( 0, 2 );

		adodb_rs.Close( );
		adodb_sm.Close( );	

		this.app.msg_out( "Time4: " + ( new Date( ) - start_time ) );

		/* Convert the string code to a C/C++ header. */
		var str = hex_result.join( "" );
		this.app.msg_out( "Time5: " + ( new Date( ) - start_time ) );
		str = str.replace( /.{16}/g, "$&\n" ).replace( /../g, "0x$&, " );
		this.app.msg_out( "Time6: " + (new Date( ) - start_time ) );
		var out_file = this.app.fso.CreateTextFile( out_file_path, true );
		out_file.Write( str );
		out_file.Write( "\n" );
		out_file.Close( );

		this.app.msg_out( "Time7: " + ( new Date( ) - start_time ) );
	}

}


