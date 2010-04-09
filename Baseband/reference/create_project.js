function Print (oTarget, str)
{
	try
	{
		oTarget.Write (str);
	}
	catch (err)
	{
		WScript.Echo (str);
	}
}


var oFS = WScript.CreateObject ("Scripting.FileSystemObject");
var oSH = WScript.CreateObject ("WScript.Shell");
var ForReading = 1;
var ForWriting = 2;
var ForAppending = 8;


var sFilePathPattern = "^.*\\.(c|cc|cpp|cxx|c\\+\\+|h|hpp|hxx|h\\+\\+|a)$";


function Usage (oTarget)
{
	Print (oTarget, "Usage:\n"
				+ "    " + WScript.ScriptName + " [-gcc|-ads|-both] <prj_file_path(*.cfg)>\n"
				+ "    " + WScript.ScriptName + " [-gcc|-ads|-both] <folder_path)> [file_path_mask]\n"
				+ "\n"
				+ "    -gcc            Create Makefile for gcc.\n"
				+ "    -ads            Create .mcp for CodeWarrior for ADS.\n"
				+ "    -both           Create both Makefile and .mcp.\n"
				+ "    -file_path_mask Regexp for the files to be added, default is:\n"
				+ "                    " + sFilePathPattern + "\n"
				+ "\n"
				+ "    In case of using <folder_path>, if \"project.cfg\" is found in\n"
				+ "<folder_path>, use <folder_path>\\project.cfg as the configuration\n"
				+ "file, Otherwise all files in <folder_path> will be added and use the\n"
				+ "folder name as the project name.\n"
				);
}


function CheckProjectPath (sPrjPath)
{
	if (GetScriptDir () == GetProjectDir (sPrjPath))
	{
		Print (WScript.StdErr, "Can not create project in the same folder as the folder of " + WScript.ScriptName + ".");
		WScript.Quit (4);
	}
}


/* Get lines with prefix sHeader. */
function GetLine (sPrjPath, sHeader, sPlatform)
{
	var asSource = [];
	var oPrj = oFS.OpenTextFile (sPrjPath, ForReading);
	if (oPrj)
	{
		var reg = new RegExp ('^\\s*' + sHeader + '\\s*=\\s*(.*?)\\s*$');
		var reg_platform = new RegExp ('^\\s*' + sHeader + '_' + sPlatform + '\\s*=\\s*(.*?)\\s*$');
		while (! oPrj.AtEndOfStream)
		{
			var sLine = oPrj.ReadLine ();

			var aRes1 = sLine.match (reg);
			var aRes2 = sLine.match (reg_platform);
			if (aRes1)
				asSource[asSource.length] = aRes1[1];
			if (aRes2)
				asSource[asSource.length] = aRes2[1];
		}
		oPrj.Close ();
	}
	return asSource;
}


/* Get project name. */
this.GetProjectName = function (sPrjPath, sPlatform)
{
	if (oFS.FolderExists (sPrjPath))
	{
		return oFS.GetFileName (sPrjPath);
	}
	else if (oFS.FileExists (sPrjPath))
	{
		var sName = GetLine (sPrjPath, "PROJECT_NAME", sPlatform).join("");
		sName = sName.replace (/(^\s*|\s*$)/g, "");
		if (sName.length == 0)
			sName = oFS.GetFileName (sPrjPath).replace (/\.[^.]*$/i, "");
		return sName;
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}


/* Get all files in folder. */
function GetFilesInFolder (sFolder_Path, sDistBaseDir, regExpFile)
{
	var asFiles = [];
	var oFolder = oFS.GetFolder (sFolder_Path);

	var oFC = new Enumerator (oFolder.files);
	for (oFC.moveFirst (); ! oFC.atEnd (); oFC.moveNext ())
	{
		var sFile_Path = oFC.item ().Path;

		if (sFile_Path.indexOf (sDistBaseDir) == 0)
			sFile_Path = sFile_Path.substr (sDistBaseDir.length);

		if (sFile_Path.match (regExpFile))
		{
			asFiles[asFiles.length] = "./" + sFile_Path.replace (/\\/g, '/');
		}
	}

	var oFC = new Enumerator (oFolder.SubFolders);
	for (oFC.moveFirst (); ! oFC.atEnd (); oFC.moveNext ())
	{
		asFiles = asFiles.concat (GetFilesInFolder (oFC.item ().Path, sDistBaseDir, regExpFile));
	}
	return asFiles;
}


/* Get all link source files. */
function GetLinkSource (sPrjPath, sPlatform)
{
	if (oFS.FolderExists (sPrjPath))
	{
		var sDistBaseDir = oFS.GetFolder (sPrjPath).Path;
		if (sDistBaseDir.length > 0)
		{
			if (sDistBaseDir.charAt (sDistBaseDir.length - 1) != '\\')
				sDistBaseDir += '\\';
		}

		var regExpFile = new RegExp (sFilePathPattern);
		var asFiles = GetFilesInFolder (oFS.GetFolder (sPrjPath).Path, sDistBaseDir, regExpFile);
		return asFiles;
	}
	else if (oFS.FileExists (sPrjPath))
	{
		return GetLine (sPrjPath, "LINK_SOURCE", sPlatform);
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}


/* Get header files. */
function GetHeaderSource (sPrjPath, asSourceFiles)
{
	var asFiles = [];
	return asFiles;
	var sHeaderFile;

	var oOldDir = oSH.CurrentDirectory;
	oSH.CurrentDirectory = GetProjectDir (sPrjPath);
	var sDepFile = "__ADS.dep";
	while (oFS.FileExists (sDepFile))
		sDepFile += ".dep";

	try
	{
		var oFile = oFS.CreateTextFile (sDepFile, true);
		oFile.Close ();

		var SOURCE_ARRAY_SLICE = 8;
		for (var i = 0; i < asSourceFiles.length; i += SOURCE_ARRAY_SLICE)
		{
			oSH.Run ("makedepend -f" + sDepFile + " -w1 -Y -DPLATFORM_ADS_W90N740 \""
				+ asSourceFiles.slice (i, i + SOURCE_ARRAY_SLICE).join ("\" \"")
				+ "\"",
				0, true);
			oFile = oFS.OpenTextFile (sDepFile, 1);
			while (!oFile.AtEndOfStream)
			{
				var sLine = oFile.ReadLine ();
				if (sLine.match (/^.+:.+$/))
				{
					asFiles[asFiles.length] = sLine.replace (/(^.+:\s*|\s*$)/, "");
				}
			}
			oFile.Close ();
		}
	}
	catch (err)
	{
	}

	try
	{
		oFS.DeleteFile (sDepFile);
		oFS.DeleteFile (sDepFile + ".bak");
	}
	catch (err)
	{
	}

	oSH.CurrentDirectory = oOldDir;

	return asFiles;
}


/* Get base dir for distribution. */
function GetDistBaseDir (sPrjPath, sPlatform)
{
	if (oFS.FolderExists (sPrjPath))
	{
		return ".";
	}
	else if (oFS.FileExists (sPrjPath))
	{
		var sDir = GetLine (sPrjPath, "DIST_BASE_DIR", sPlatform).join("");
		sDir = sDir.replace (/(^\s*|\s*$)/g, "");
		if (sDir.length == 0)
			sDir = ".";
		return sDir;
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}

/* Get all released files. */
function GetReleaseFile (sPrjPath, sPlatform)
{
	if (oFS.FolderExists (sPrjPath))
	{
		return ["."];
	}
	else if (oFS.FileExists (sPrjPath))
	{
		return GetLine (sPrjPath, "DIST_FILE", sPlatform);
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}

/* Get all no released files. */
function GetNoReleaseFile (sPrjPath, sPlatform)
{
	if (oFS.FolderExists (sPrjPath))
	{
		return [];
	}
	else if (oFS.FileExists (sPrjPath))
	{
		return GetLine (sPrjPath, "NO_DIST_FILE", sPlatform);
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}

/* Get sub projects. */
function GetSubProject (sPrjPath, sPlatform)
{
	if (oFS.FolderExists (sPrjPath))
	{
		return [];
	}
	else if (oFS.FileExists (sPrjPath))
	{
		return GetLine (sPrjPath, "SUB_PROJECT", sPlatform);
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}


/* The the script directory. */
function GetScriptDir ()
{
	if (typeof (__sScriptDir) == "undefined")
		return oFS.GetParentFolderName (oFS.GetFile (WScript.ScriptFullName));
	else
		return __sScriptDir;
}

/* The project directory. */
function GetProjectDir (sPrjPath)
{
	if (oFS.FolderExists (sPrjPath))
	{
		return oFS.GetFolder (sPrjPath).Path;
	}
	else if (oFS.FileExists (sPrjPath))
	{
		var oPrjPath = oFS.GetFile (sPrjPath);
		return oFS.GetParentFolderName (oPrjPath.Path);
	}
	else
	{
		throw "Error: project file or folder not found!";
	}
}

/* Get a escaped string in Makefile. */
function Escape_Makefile (s)
{
	var rt = s;
	rt = rt.replace (/\\/g, "\\\\");
	rt = rt.replace (/ /g, "\\ ");
	rt = rt.replace (/\t/g, "\\\t");
	rt = rt.replace (/#/g, "\\#");
	return rt;
}

/* Generate Makefile. */
this.Generate_Makefile = function (sPrjPath)
{
	CheckProjectPath (sPrjPath);

	var sProjectName = GetProjectName (sPrjPath, "GCC");
	var sProjectDir = GetProjectDir (sPrjPath);

	var sTemplatePath = GetScriptDir () + "\\Makefile";
	var sMakefilePath = sProjectDir + "\\Makefile";

	var asLinkSource = GetLinkSource (sPrjPath, "GCC");
	var sDistBaseDir = GetDistBaseDir (sPrjPath, "GCC");
	var asReleaseFile = GetReleaseFile (sPrjPath, "GCC");
	var asNoReleaseFile = GetNoReleaseFile (sPrjPath, "GCC");
	var asSubProject = GetSubProject (sPrjPath, "GCC");

	for (var i = 0; i < asLinkSource.length; i++)
	{
		asLinkSource[i] = '\\\n\t' + Escape_Makefile (asLinkSource[i]);
	}
	for (var i = 0; i < asReleaseFile.length; i++)
	{
		asReleaseFile[i] = '\\\n\t' + Escape_Makefile (asReleaseFile[i]);
	}
	for (var i = 0; i < asNoReleaseFile.length; i++)
	{
		asNoReleaseFile[i] = '\\\n\t' + Escape_Makefile (asNoReleaseFile[i]);
	}
	for (var i = 0; i < asSubProject.length; i++)
	{
		asSubProject[i] = '\\\n\t' + Escape_Makefile (asSubProject[i]);
	}

	var oTemplateFile = oFS.OpenTextFile (sTemplatePath, ForReading);
	var oMakefileFile = oFS.OpenTextFile (sMakefilePath, ForWriting, true);

	if (oTemplateFile && oMakefileFile)
	{
		var sFileContent = oTemplateFile.ReadAll ();
		if (sFileContent)
		{
			sFileContent = sFileContent.replace (/^PROJECT_NAME(.|\n)*?[^\\]$/m, "PROJECT_NAME :=\\\n\t" + Escape_Makefile(sProjectName));
			sFileContent = sFileContent.replace (/^LINK_SOURCE(.|\n)*?[^\\]$/m, "LINK_SOURCE :=" + asLinkSource.join(""));
			sFileContent = sFileContent.replace (/^DIST_BASE_DIR(.|\n)*?[^\\]$/m, "DIST_BASE_DIR :=\\\n\t" + Escape_Makefile(sDistBaseDir));
			sFileContent = sFileContent.replace (/^DIST_FILE(.|\n)*?[^\\]$/m, "DIST_FILE :=" + asReleaseFile.join(""));
			sFileContent = sFileContent.replace (/^NO_DIST_FILE(.|\n)*?[^\\]$/m, "NO_DIST_FILE :=" + asNoReleaseFile.join(""));
			sFileContent = sFileContent.replace (/^SUB_PROJECT(.|\n)*?[^\\]$/m, "SUB_PROJECT :=" + asSubProject.join(""));
			oMakefileFile.Write (sFileContent);
		}
	}

	if (oTemplateFile)
		oTemplateFile.Close ();
	if (oMakefileFile)
		oMakefileFile.Close ();
}


/* Check if it's absolute path. */
function IsAbsolutionPath (sPath)
{
	if (sPath.charAt(0) == '\\' || sPath.charAt(0) == '/'
		|| sPath.match (/^[a-zA-Z]:/))
		return true;
	else
		return false;
}

/* Get the same header string. */
function GetSameHeaderStr (s1, s2)
{
	var i;
	for (i = 0; i < s1.length && i < s2.length; i++)
	{
		if (s1.charCodeAt (i) != s2.charCodeAt (i))
			break;
	}
	for (; i>= 0; i--)
	{
		if (s1.charAt (i) == '\\' || s1.charAt (i) == '/')
			break;
	}

	var rt = {
		sSame: s1.substr (0, i),
		sStr1: s1.substr (i),
		sStr2: s2.substr (i)
	};
	return rt;

}


/* Check file's path. */
function CheckPath (sSourcePath, sProjectDir)
{
	var sFilePath = IsAbsolutionPath (sSourcePath) ?
		sSourcePath : sProjectDir + "\\" + sSourcePath;
	sFilePath = oFS.GetFile (sFilePath).Path;

	/* Check same string. */
	var sComPaths = GetSameHeaderStr (sFilePath, sProjectDir);

	/* Check access path. */
	if (sComPaths.sSame.length == 0)
		throw "Use absolute path: " + sFilePath;

	var oRes = sComPaths.sStr2.match (/[\/\\]/g);
	var nDirNum = oRes ? oRes.length : 0;
	var oPath =
	{
		nDirNum: nDirNum,
		sFilePath: sComPaths.sStr1
	};

	return oPath;
}


/* Check common root access path. */
function CheckCommonPath (asFiles, sProjectDir)
{
	var nDirNum = 0;
	for (var i in asFiles)
	{
		if (asFiles[i].nDirNum > nDirNum)
			nDirNum = asFiles[i].nDirNum;
	}

	var sAccessPath = "";
	for (var i = 0; i < nDirNum; i++)
	{
		if (i != 0)
			sAccessPath += "\\";
		sAccessPath += "..";
	}

	for (var i in asFiles)
	{
		var nBackDirNum = nDirNum - asFiles[i].nDirNum;
		var sReg = "$";
		for (var j = 0; j < nBackDirNum; j++)
			sReg = "[\\/\\\\][^\\/\\\\]+" + sReg;

		var sBackDir = (sProjectDir.match (sReg)[0]);
		asFiles[i].sFilePath = (sBackDir + asFiles[i].sFilePath).substr(1);
		asFiles[i].sAccessPath = sAccessPath;
	}

	return sAccessPath;
}


/* Generate XML for access path. */
function Generate_MCP_XML_AccessPath (sAccessPath)
{
	var sRt = "";
	while (1)
	{
		sRt =
  "                    <SETTING>\n"
+ "                        <SETTING><NAME>SearchPath</NAME>\n"
+ "                            <SETTING><NAME>Path</NAME><VALUE>" + sAccessPath + "</VALUE></SETTING>\n"
+ "                            <SETTING><NAME>PathFormat</NAME><VALUE>Windows</VALUE></SETTING>\n"
+ "                            <SETTING><NAME>PathRoot</NAME><VALUE>Project</VALUE></SETTING>\n"
+ "                        </SETTING>\n"
+ "                        <SETTING><NAME>Recursive</NAME><VALUE>true</VALUE></SETTING>\n"
+ "                        <SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n"
+ "                        <SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n"
+ "                    </SETTING>\n"
			+ sRt;

		if (sAccessPath.length == 0)
			break;
		if (sAccessPath.substr (0, 2) == '..')
		{
			sAccessPath = sAccessPath.substr (2);
			if (sAccessPath.substr (0, 1) == '\\')
				sAccessPath = sAccessPath.substr (1);
		}
		else
			sAccessPath = "";
	}
	return sRt;
}


/* Generate XML for files list. */
function Generate_MCP_XML_FileList (asFiles)
{
	var sRt = "";
	for (var i in asFiles)
	{
		sRt +=
  "                <FILE>\n"
+ "                    <PATHTYPE>PathRelative</PATHTYPE>\n"
+ "                    <PATHROOT>Project</PATHROOT>\n"
+ "                    <ACCESSPATH>" + asFiles[i].sAccessPath + "</ACCESSPATH>\n"
+ "                    <PATH>" + asFiles[i].sFilePath + "</PATH>\n"
+ "                    <PATHFORMAT>Windows</PATHFORMAT>\n"
+ "                    <FILEKIND>Text</FILEKIND>\n"
+ "                    <FILEFLAGS>Debug</FILEFLAGS>\n"
+ "                </FILE>\n";
	}
	return sRt;
}


/* Generate XML for link order. */
function Generate_MCP_XML_LinkOrder (asFiles)
{
	var sRt = "";
	for (var i in asFiles)
	{
		sRt +=
  "                <FILEREF>\n"
+ "                    <PATHTYPE>PathRelative</PATHTYPE>\n"
+ "                    <PATHROOT>Project</PATHROOT>\n"
+ "                    <ACCESSPATH>" + asFiles[i].sAccessPath + "</ACCESSPATH>\n"
+ "                    <PATH>" + asFiles[i].sFilePath + "</PATH>\n"
+ "                    <PATHFORMAT>Windows</PATHFORMAT>\n"
+ "                </FILEREF>\n";
	}
	return sRt;
}


/* Generate XML for groups recursively. */
function Generate_MCP_XML_Group_RecurHelp (nAdd, oGroup)
{
	var sAdd ="";
	for (var i = 0; i < nAdd; i++)
		sAdd += "    ";

	var sRt = "";


	for (var i in oGroup.sGroups)
	{
		sRt +=
sAdd + "        <GROUP><NAME>" + i + "</NAME>\n";
		sRt += Generate_MCP_XML_Group_RecurHelp (nAdd + 1, oGroup.sGroups[i]);
		sRt +=
sAdd + "        </GROUP>\n";
	}

	for (var i in oGroup.sFiles)
	{
			sRt +=
  sAdd + "        <FILEREF>\n"
+ sAdd + "            <TARGETNAME>DebugRel</TARGETNAME>\n"
+ sAdd + "            <PATHTYPE>PathRelative</PATHTYPE>\n"
+ sAdd + "            <PATHROOT>Project</PATHROOT>\n"
+ sAdd + "            <ACCESSPATH>" + oGroup.sFiles[i].sAccessPath + "</ACCESSPATH>\n"
+ sAdd + "            <PATH>" + oGroup.sFiles[i].sFilePath + "</PATH>\n"
+ sAdd + "            <PATHFORMAT>Windows</PATHFORMAT>\n"
+ sAdd + "        </FILEREF>\n";

	}

	return sRt;
}


/* Generate XML for groups. */
function Generate_MCP_XML_Group (asFiles)
{
	var oGroup =
	{
		sGroups: {},
		sFiles: {}
	};

	for (var i in asFiles)
	{
		var s;
		var grp;
		var index;

		grp = oGroup;
		for (s = asFiles[i].sFilePath;
			(index = s.indexOf ('\\')) != -1;
			s = s.substr (index + 1))
		{
			var sGroup = s.substr (0, index);
			if (typeof (grp.sGroups[sGroup]) == "undefined")
				grp.sGroups[sGroup] =
				{
					sGroups: {},
					sFiles: {}
				};

			grp = grp.sGroups[sGroup];
		}
		grp.sFiles[s] = asFiles[i];
	}

	return Generate_MCP_XML_Group_RecurHelp (0, oGroup);
}


/* Generate a mcp (CodeWarrior project file). */
this.Generate_MCP = function (sPrjPath)
{
	CheckProjectPath (sPrjPath);

	var sProjectName = GetProjectName (sPrjPath, "ADS");
	var sProjectDir = GetProjectDir (sPrjPath);
	var sTemplatePath = GetScriptDir () + "\\project.mcp.xml";
	var sXMLPath = sProjectDir + "\\" + sProjectName + ".mcp.xml";
	var sMCPPath = sProjectDir + "\\" + sProjectName + ".mcp";

	var asLinkSource = GetLinkSource (sPrjPath, "ADS");

	/* The link source files. */
	var asFiles = [];
	for (var i in asLinkSource)
	{
		var sSourcePath = asLinkSource[i].replace(/\//g, "\\");
		var oPath = CheckPath (sSourcePath, sProjectDir);
		var j;
		for (j = 0; j < asFiles.length; j++)
		{
			if (asFiles[j].sFilePath == oPath.sFilePath)
				break;
		}
		if (j >= asFiles.length)
		{
			asFiles[asFiles.length] = oPath;
		}
	}

	if (asFiles.length == 0)
	{
		Print (WScript.StdErr, "No linked files found in this project.");
		return;
	}
	var asLinkFiles = asFiles;

	/* The header files. */
	var asHeaderSource = GetHeaderSource (sPrjPath, asLinkSource);
	for (var i in asHeaderSource)
	{
		var sSourcePath = asHeaderSource[i].replace(/\//g, "\\");
		var oPath = CheckPath (sSourcePath, sProjectDir);
		for (j = 0; j < asFiles.length; j++)
		{
			if (asFiles[j].sFilePath == oPath.sFilePath)
				break;
		}
		if (j >= asFiles.length)
		{
			asFiles[asFiles.length] = oPath;
		}
	}

	var sAccessPath = CheckCommonPath (asFiles, sProjectDir);


	var sMCP_XML_AccessPath = Generate_MCP_XML_AccessPath (sAccessPath);
	var sMCP_XML_FileList = Generate_MCP_XML_FileList (asFiles);
	var sMCP_XML_LinkOrder = Generate_MCP_XML_LinkOrder (asLinkFiles);
	var sMCP_XML_Group = Generate_MCP_XML_Group (asFiles);


	/* Replace template XML */
	var oTemplateFile = oFS.OpenTextFile (sTemplatePath, ForReading);
	var oXMLFile = oFS.OpenTextFile (sXMLPath, ForWriting, true);

	if (oTemplateFile && oXMLFile)
	{
		var sFileContent = oTemplateFile.ReadAll ();
		if (sFileContent)
		{
			sFileContent = sFileContent.replace (/__TEMP_SEARCH_PATH__/g, sMCP_XML_AccessPath);
			sFileContent = sFileContent.replace (/__TEMP_FILE_LIST__/g, sMCP_XML_FileList);
			sFileContent = sFileContent.replace (/__TEMP_LINK_ORDER__/g, sMCP_XML_LinkOrder);
			sFileContent = sFileContent.replace (/__TEMP_GROUP_LIST__/g, sMCP_XML_Group);
			oXMLFile.Write (sFileContent);
		}
	}

	if (oTemplateFile)
		oTemplateFile.Close ();
	if (oXMLFile)
		oXMLFile.Close ();



	var oCW = WScript.CreateObject ("CodeWarrior.CodeWarriorApp");

	var oProj = oCW.ImportProject (sXMLPath, sMCPPath, true);
	oFS.DeleteFile (sXMLPath, true);
	var oTargets = oProj.Targets;
	for (var i = 0; i < oTargets.Count; i++)
	{
		//oTargets.Item (i).BuildAndWaitToComplete ();
	}
	oCW.Quit (2);
}


function RunAsCommand ()
{
	/* Check command line argument. */
	if (WScript.Arguments.Count () != 1
		&& WScript.Arguments.Count () != 2
		&& WScript.Arguments.Count () != 3)
	{
		Usage (WScript.StdErr);
		WScript.Quit (1);
	}

	/* sAction. */
	var sAction = WScript.Arguments(0);
	if (sAction != "-gcc"
		&& sAction != "-ads"
		&& sAction != "-both")
	{
		Usage (WScript.StdErr);
		WScript.Quit (2);
	}


	/* Check if the project file exists. */
	var sPrjPath = WScript.Arguments(1);
	if (oFS.FolderExists (sPrjPath))
	{
		if (oFS.FileExists (sPrjPath + "/project.cfg"))
			sPrjPath += "/project.cfg";
		if (WScript.Arguments.Count () == 3)
			sFilePathPattern = WScript.Arguments(2);
	}
	else if (oFS.FileExists (sPrjPath))
		;
	else
	{
		Print (WScript.StdErr, "Project file not exists: " + sPrjPath);
		WScript.Quit (3);
	}

	if (sAction == "-gcc" || sAction == "-both")
		Generate_Makefile (sPrjPath);
	if (sAction == "-ads" || sAction == "-both")
		Generate_MCP (sPrjPath);

}


if (typeof (__bIsInInclude) == "undefined" && typeof (__sScriptDir) == "undefined")
	RunAsCommand ();

