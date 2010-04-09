this.builder_mcp = function( app )
{
	this.app = app;
	this.toString = function( )
	{
		return "ADS codewarrior MCP builder";
	}

	this.ide = null;
	this.xmldom = null;
	this.create_ide = function( )
	{
		if (this.ide != null)
			return;
		this.ide = new ActiveXObject( "CodeWarrior.CodeWarriorApp" );
		this.xmldom = new ActiveXObject( "Microsoft.XMLDOM" );
	}
	this.delete_ide = function( )
	{
		if (this.ide == null)
			return;
		if ( this.ide.Projects.Count == 0 )
			this.ide.Quit( 2 );
		this.ide = null;		
		this.xmldom = null;
	}

	this.GetXMLValue = function( xmldom, sPath, sName )
	{
		var aoNames = xmldom.selectNodes( sPath + "/NAME" );
		var aReturn = [];
		for ( var i = 0; i < aoNames.length; i++ )
		{
			if ( aoNames[i].text == sName )
			{
				var oNodeXML = aoNames[i].parentNode;
				var oNodeValue = oNodeXML.selectSingleNode( "./VALUE" );
				aReturn[aReturn.length] =
				{
					xml: oNodeXML,
					value: ( oNodeValue ? oNodeValue.text : null )
				};
			}
		}
		return aReturn;
	}

	this.CheckProject = function( sPath, oProject )
	{
		var bReturn = true;

		/* Check access path. */
		var aoTargets = oProject.Targets;
		var asAllPath = [];

		for ( var i = 0; i < aoTargets.Count; i++ )
		{
			var oTarget = aoTargets.Item( i );
	
			var oAccessPaths = oTarget.AccessPaths.UserAccessPaths;

			var asPaths = [];
			for ( var j = 0; j < oAccessPaths.Count; j++ )
			{
				var oAccessPath = oAccessPaths.Item( j );
				if ( oAccessPath.AccessPathLocation != 1 )
				{
					this.app.die( 'AccessPath "' + oAccessPath.Path.FullPath + '" is not projet relative!' );
					bReturn = false;
				}
				asPaths[asPaths.length] = oAccessPath.Path.FullPath;
			}
			asAllPath[i] = asPaths.sort( ).join( "\n" );
			if ( i != 0 )
			{
				if ( asAllPath[i] != asAllPath[0] )
				{
					var name0 = aoTargets.Item( 0 ).Name;
					var namei = aoTargets.Item( i ).Name;
					this.app.die( 'Different AccessPaths in targets "' + name0 + '" and "' + namei + '".\n' );
					bReturn = false;
				}
			}
		
		}

		/* Check by xml. */
		var sXMLFile = sPath + ".xml";
		oProject.Export( sXMLFile );

		var oFile = this.app.fso.OpenTextFile( sXMLFile, 1 );
		var sXML = oFile.ReadAll( );
		oFile.Close( );
		sXML = sXML.replace( /\<!ELEMENT\s[^>]*\>/g, "" );
		sXML = sXML.replace( /\<!DOCTYPE\s[^>]*\>/g, "" );

		this.xmldom.async = false;
		if (! this.xmldom.loadXML( sXML ) )
		{
			this.app.die( this.xmldom.parseError.reason );
			return false;
		}


		/* Check relative path. */
		var aoRelativePaths = this.GetXMLValue( this.xmldom, "//TARGET/SETTINGLIST/SETTING", 'SaveEntriesUsingRelativePaths' );
		for ( var i = 0; i < aoRelativePaths.length; i++ )
		{
			var bRelative = aoRelativePaths[i].value;
			if ( bRelative != 'true' )
			{
				this.app.die( 'Not use SaveEntriesUsingRelativePaths in target "'
					+ aoRelativePaths[i].xml.parentNode.parentNode.selectSingleNode( "./NAME" ).text
					+ '".\n' );
				bReturn = false;
			}
		}

		/* Check output directory. */
		var aoOutDirs = this.GetXMLValue( this.xmldom, "//TARGET/SETTINGLIST/SETTING", 'OutputDirectory' );
		for ( var i = 0; i < aoOutDirs.length; i++ )
		{
			var aoPathRoot = this.GetXMLValue( aoOutDirs[i].xml, "./SETTING", 'PathRoot' );
			var aoPath = this.GetXMLValue( aoOutDirs[i].xml, "./SETTING", 'Path' );

			if ( aoPathRoot.length != 1 || aoPathRoot[0].value != "Project" )
			{
				this.app.die( 'Project output root not "Project" in target "'
					+ aoOutDirs[i].xml.parentNode.parentNode.selectSingleNode( "./NAME" ).text
					+ '".\n' );
				bReturn = false;
			}
			if ( aoPath.length != 1 || aoPath[0].value.length <= 0 )
			{
				this.app.die( 'Project output path is empty in target "'
					+ aoOutDirs[i].xml.parentNode.parentNode.selectSingleNode( "./NAME" ).text
					+ '".\n' );
				bReturn = false;
			}

		}


		/* Check compiler options. */
		var aoARMCCs = this.GetXMLValue( this.xmldom, "//TARGET/SETTINGLIST/PANELDATA", 'Panel_for_armcc' );
		for ( var i = 0; i < aoARMCCs.length; i++ )
		{
			var sTargetName = aoARMCCs[i].xml.parentNode.parentNode.selectSingleNode( "./NAME" ).text;
			var sValue = aoARMCCs[i].value;
			sValue = sValue.split( /\s+/m ).join( '' );

			if ( !sValue.match( /2D4445434F53/ ) )	/* -DECOS */
			{
				die.app.die( 'No -DECOS defined in target "' + sTargetName + '".\n' );
				bReturn = false;
			}

			if ( !sValue.match( /2D7A6F/ ) )	/* -zo */
			{
				this.app.die( 'No -zo defined in target "' + sTargetName + '".\n' );
				bReturn = false;
			}

			if ( !sValue.match( /41524D393436452D53/ ) )	/* ARM946E_S */
			{
				this.app.die( 'Not use ARM946E-S in target "' + sTargetName + '".\n' );
				bReturn = false;
			}
		
			if ( !sValue.match( /2D673D2B/ ) ) /* -g+ */
			{
				this.app.die( 'No -g+ defined in target "' + sTargetName + '".\n' );
				bReturn = false;
			}

			var sOptimizeTarget = 'Release';
			var sOptimize = '-O2';
			if ( sValue.match( /2D4F2E6C6576656C3D30/ ) )	/*-O.level=0 */
			{
				sOptimizeTarget = 'Debug';
				sOptimize = '-O0';
			}
			else if ( sValue.match( /2D4F2E6C6576656C3D31/ ) )	/*-O.level=1 */
			{
				sOptimizeTarget = 'DebugRel';
				sOptimize = '-O1';
			}
		
			if (sTargetName == 'Debug' || sTargetName == 'DebugRel' ||sTargetName == 'Release')
			{
				if ( sTargetName != sOptimizeTarget )
				{
					this.app.die( 'Optimize option is ' + sOptimize + ' in target "' + sTargetName + '".\n' );
					bReturn = false;
				}
			}
			else if (sOptimizeTarget == 'Debug')
			{
				this.app.die( 'Optimize option is -O0 in target "' + sTargetName + '".\n' );
				bReturn = false;
			}


		}

		this.app.fso.DeleteFile( sXMLFile, true );
		return bReturn;
	}

	this.__build = function( sPath, sTarget, bCheckProject )
	{
		sPath = this.app.fso.GetFile( sPath ).Path;

		if ( this.ide == null )
			this.app.ide( "Call create_ide( ) before build MCP project!\n" );
		var sMovePath = sPath + "_move.mcp"
		var bUseBackup = false;
		if ( !this.app.fso.FileExists( sMovePath ) )
		{
			this.app.fso.MoveFile( sPath, sMovePath );
			this.app.fso.CopyFile( sMovePath, sPath, true );
			bUseBackup = true;
		}
	
		/* Remove version.o */
		var sObjPath = sPath.replace( /\.mcp$/i, "_Data" ) + "\\" + sTarget + "\\ObjectCode\\Version.o";
		
		//if ( this.app.fso.FileExists( sObjPath ) )
		//	this.app.fso.DeleteFile( sObjPath, true );

		/* Open project target and build */
		var oProject = null;
		for ( var j = 0; j < 10; ++j )
		{
			for ( var i = 0; i < this.ide.Projects.Count; i++ )
			{
				oProject = this.ide.Projects.Item(i);
				if ( oProject.FileSpec.FullPath == sPath )
					break;
			}
		
			if ( i >= this.ide.Projects.Count )
				oProject = this.ide.OpenProject( sPath, true, 0, 0 );

			if ( oProject != null )
				break;
		}

		if ( oProject == null )
		{
			this.app.die( 'Can not open project "' + sPath + '"\n');
		}

		if ( bCheckProject && !this.CheckProject( sPath, oProject ) )
			this.app.die( 'Check project setting failed\n' );

		var oTarget = oProject.FindTarget( sTarget );
		if ( oTarget == null )
		{
			this.app.die( 'Can not open target "' + sTarget + '" for project "' + sPath + '"\n');
		}
		var oMsg = oTarget.BuildAndWaitToComplete( );
		oTarget = null;

		if ( oMsg.ErrorCount > 0 )
			this.app.die( "Found error while building\n" );
		oMsg = null;
		//for ( var i = 0; i < oMsg.ErrorCount; i++ )
		//	this.app.msg_out( oMsg.Errors.Item (i).MessageText + "\n" );

		oProject.Close( );
		oProject = null;

		var projects_count = this.ide.Projects.Count;	//To add this lines seems avoiding the files be locked by CW
	
		this.app.fso.DeleteFile( sPath, true );
		this.app.fso.MoveFile( sMovePath, sPath );
		//this.app.fso.DeleteFolder (sPath.substr (0, sPath.length - 4) + "_Data", true);
	}

	this.build = function( sPath, sTarget )
	{
		this.__build( sPath, sTarget, true );
	}

	this.build_without_checking_mcp = function( sPath, sTarget )
	{
		this.__build( sPath, sTarget, false );
	}
}



