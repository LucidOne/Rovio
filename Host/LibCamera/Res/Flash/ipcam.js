
function $(id)
{
	return document.getElementById(id);
}

function escapeHTML(s)
{
	var div = document.createElement('div');
	var text = document.createTextNode(s);
	div.appendChild(text);
	return div.innerHTML;
}


function Int2Hex(i)
{
	if (i>=0 && i<=9) return String.fromCharCode(i+48);
	else return String.fromCharCode(i+55);
}


function Trim(s)
{
	return (s + "").replace(/(^\s*|\s*$)/g, "");
}


function GetFormString(s)
{
	var i;
	var o;
	for (o = "", i = 0; i < s.length; i++)
	{
		var c;
		c = s.charCodeAt(i);
		if (c > 0 && c < 256)
		{
			if (c == 32)
				o += '+';
			else if (c < 32 || c >= 127 || c == 0x25 || c == 0x26 || c == 0x3d || c == 0x2b)
			{
				var c1 = c % 16;
				var c2 = (c - c1) / 16;
				o += "%" + Int2Hex(c2) + Int2Hex(c1);
			}
			else
				o += String.fromCharCode(c);
		}
		else
			o += s.charAt(i);
	}
	return o;
}


function FormSubmitUrl(oForm)
{
	var sRt = GetFormString(oForm.action);
	return sRt;
}

function FormSubmitParam(oForm)
{
	var sRt = "";
	var bFirst = true;
	for (var i = 0; i < oForm.elements.length; i++)
	{
		if (oForm.elements[i].name)
		{
			if (oForm.elements[i].type == 'checkbox' || oForm.elements[i].type == 'radio')
			{
				if (!oForm.elements[i].checked) continue;
			}

			if (!bFirst) sRt += '&';
			else bFirst = false;
			sRt += GetFormString(oForm.elements[i].name) + '=' + GetFormString(oForm.elements[i].value);
		}
	}
	return sRt;
}

function FormSubmit(oForm, sFun)
{
	ConfigBegin();
	DoCommand(FormSubmitUrl(oForm), FormSubmitParam(oForm), sFun);
	return false;
}

function DownLoadEx(oXHR, sUrl, sPostData, sFun)
{
	var xhr = oXHR;

	if (xhr == null)
	{
		try {xhr = new XMLHttpRequest();}
		catch (e) {xhr = null;}
	}
	if (xhr == null)
	{
		try {xhr = new ActiveXObject("Msxml2.XMLHTTP");}
		catch (e) {xhr = null;}
	}
	if (xhr == null)
	{
		try {xhr = new ActiveXObject("Microsoft.XMLHTTP");}
		catch (e) {xhr = null;}
	}
	
	xhr.open ("POST", sUrl, true);
	xhr.onreadystatechange = function ()
	{
		if (xhr.readyState == 4 && xhr.status == 200)
		{
			sFun (xhr.responseText);
		}
	}
	xhr.send (sPostData);

	return xhr;
}


function DownLoad(sUrl, sPostData, sFun)
{
	return DownLoadEx(null, sUrl, sPostData, sFun);
}


function NullFun(sText)
{
}

function DoCommand(sCmd, sParam, sFun)
{
	if (typeof(parent.bDebug) == "undefined")
		parent.bDebug = false;
	
	if (parent.bDebug)
	{
		if (!confirm("About to send command:\n\n" + sCmd + "?" + sParam + "\n\nContinue?"))
			return;
	}

	DownLoad(sCmd, sParam, (sFun?sFun:NullFun));
}


function NodeWait(o)
{
	try
	{
		o.onclick = function () {return false;}
		o.onkeypress = function () {return false;}
		o.style.cursor = "wait";
		for (var i = 0; i < o.childNodes.length; i++)
			NodeWait(o.childNodes[i]);
	} catch (e) {}
}

function ConfigBegin()
{
	NodeWait(document.body);
}


function ConfigEnd()
{
	if (confirm ("Send configuration to server successfully!\nBack to camera window?\n"))
		window.location = parent.sMainPath;
	else
		window.location.reload ();
}


function InputEnable(o, bEnable)
{
	if (typeof(o.disabled_bg_color) == "undefined" && typeof(o.enabled_bg_color) == "undefined")
	{
		if (o.disabled)
			o.disabled_bg_color = o.style.backgroundColor;
		else
			o.enabled_bg_color = o.style.backgroundColor;
	}

	o.disabled = ! bEnable;
	if (o.disabled)
	{
		if (typeof (o.disabled_bg_color) != "undefined")
			o.style.backgroundColor = o.disabled_bg_color;
		else
			o.style.backgroundColor = "#ece9d8";
	}
	else
	{
		if (typeof (o.enabled_bg_color) != "undefined")
			o.style.backgroundColor = o.enabled_bg_color;
		else
			o.style.backgroundColor = "";
	}
}


var REBOOT_TIME = 15000;
function ProgressReboot (pos, total)
{
	if (pos == total)
		ConfigEnd ();
	else
	{
		$('Progress' + pos).className = "progress_yes";
		setTimeout ("ProgressReboot (" + (++pos) + ", " + total + ");", 500);
	}
}


function CheckReboot (sTitle)
{
	var len = Math.floor (REBOOT_TIME/500);
	var s ='<h1 class="tab_name">' + sTitle + '</h1><table class="tab_info">';
	for (var i = 0; i < len; i++)
		s += '<td id="Progress' + i + '" class="progress_no">&nbsp;</td>';
	s += "</tr></table>";
	$('main_area').innerHTML = s;
	
	ProgressReboot (0, len);
}
function AntennaDraw(id,size)
{
    var s ='<table id="'+id+'" border="0" cellspacing="1">';
    for (var i = 0; i < size; i++)
    {
        s += '<tr><td id="'+id+'_'+i+'" style="width:2pt;height:9pt;font-size:1pt;">&nbsp;</td></tr>'
    }
    s += '</table>';
    return s;
}

function AntennaSetPos(id,pos)
{
    pos = document.getElementById(id).rows.length - pos;
    for (var i = 0; i < document.getElementById(id).rows.length; i++)
    {
        document.getElementById(id+'_'+i).style.backgroundColor = (i < pos ? "gray" : "green");
    }
}
