this.builder_vc = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "builder_vc";
	}

	this.build = function( solution_path, project_name, config_name )
	{
		var file;

		solution_path = this.app.fso.GetFile( solution_path ).Path;

		var backup_ext_names = [ ".ncb", ".opt", ".plg", ".aps" ];
		var is_backup = [];
		for ( var i = 0; i < backup_ext_names.length; i++ )
		{
			var file = solution_path.replace( /\.[^.]*/, backup_ext_names[i] );
			if ( this.app.fso.FileExists( file ) )
			{
				this.app.fso.CopyFile( file, file + "_move" + backup_ext_names[i], true );
				this.app.fso.DeleteFile( file, true );
				is_backup[i] = true;
			}
			else
				is_backup[i] = false;
		}	

		var vc_app = new ActiveXObject( "MSDev.Application" );
		vc_app.Visible = true;
		
		/* Open solutions. */
		var vc_documents = vc_app.Documents;
		vc_documents.Open( solution_path, "Auto", false );

		/* Select project */
		var vc_project = null;
		for ( var i = 1; i <= vc_app.Projects.Count; i++ )
		{
			if ( vc_app.Projects.Item( i ).Name == project_name )
			{
				vc_project = vc_app.Projects.Item( i );
				vc_app.ActiveProject = vc_project;
				break;
			}
		}
	
		if ( vc_project != null )
		{
			/* Select configuration. */
			this.app.msg_out( vc_project.Configurations.Count );
			var vc_config = null;
			for ( var i = 1; i <= vc_project.Configurations.Count; i++ )
			{
				this.app.msg_out( vc_project.Configurations.Item( i ).Name );
				if ( vc_project.Configurations.Item( i ).Name == config_name )
				{
					vc_config = vc_project.Configurations.Item( i );
					vc_app.ActiveConfiguration = vc_config;
					break;
				}
			}
	
			if( vc_config != null )
			{
				this.app.msg_out( "Rebuild all" );
				vc_app.RebuildAll( );

				if ( vc_app.Errors > 0 )
					this.app.die( "Found error while building\n" );
			}		
		}

		

		for ( var i = 0; i < 2; i++ )
		{
			try
			{
				vc_app.Quit( );
			} catch ( e ) { }
		}

		vc_app = null;

		for ( var i = 0; i < backup_ext_names.length; i++ )
		{
			var file = solution_path.replace( /\.[^.]*/, backup_ext_names[i] );
			if ( is_backup[i] )
			{
				if ( this.app.fso.FileExists( file ) )
					this.app.fso.DeleteFile( file, true );
				this.app.fso.MoveFile( file + "_move" + backup_ext_names[i], file );	
			}
		}
	}

}

